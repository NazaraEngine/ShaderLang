// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/GlslWriter.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <Nazara/Utils/Bitset.hpp>
#include <Nazara/Utils/CallOnExit.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/EliminateUnusedPassVisitor.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <frozen/unordered_map.h>
#include <tsl/ordered_set.h>
#include <cassert>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace nzsl
{
	namespace
	{
		constexpr std::string_view s_glslWriterFlipYUniformName = "_nzslFlipYValue";
		constexpr std::string_view s_glslWriterShaderDrawParametersBaseInstanceName = "_nzslBaseInstance";
		constexpr std::string_view s_glslWriterShaderDrawParametersBaseVertexName = "_nzslBaseVertex";
		constexpr std::string_view s_glslWriterShaderDrawParametersDrawIndexName = "_nzslDrawID";
		constexpr std::string_view s_glslWriterInputPrefix = "_nzslIn_";
		constexpr std::string_view s_glslWriterOutputPrefix = "_nzslOut_";
		constexpr std::string_view s_glslWriterOutputVarName = "_nzslOutput";

		enum class GlslCapability
		{
			None = -1,

			ShaderDrawParameters_BaseInstance, // GLSL 4.6 or GL_ARB_shader_draw_parameters
			ShaderDrawParameters_BaseVertex, // GLSL 4.6 or GL_ARB_shader_draw_parameters
			ShaderDrawParameters_DrawIndex, // GLSL 4.6 or GL_ARB_shader_draw_parameters
			SSBO, // GLSL 4.3 or GLSL ES 3.1 or GL_ARB_shader_storage_buffer_object
		};
		
		struct GlslBuiltin
		{
			std::string_view identifier;
			GlslCapability requiredCapability;
		};

		constexpr auto s_glslBuiltinMapping = frozen::make_unordered_map<Ast::BuiltinEntry, GlslBuiltin>({
			{ Ast::BuiltinEntry::BaseInstance,   { "gl_BaseInstance", GlslCapability::ShaderDrawParameters_BaseInstance } },
			{ Ast::BuiltinEntry::BaseVertex,     { "gl_BaseVertex",   GlslCapability::ShaderDrawParameters_BaseVertex } },
			{ Ast::BuiltinEntry::DrawIndex,      { "gl_DrawID",       GlslCapability::ShaderDrawParameters_DrawIndex } },
			{ Ast::BuiltinEntry::FragCoord,      { "gl_FragCoord",    GlslCapability::None } },
			{ Ast::BuiltinEntry::FragDepth,      { "gl_FragDepth",    GlslCapability::None } },
			{ Ast::BuiltinEntry::InstanceIndex,  { "gl_InstanceID",   GlslCapability::ShaderDrawParameters_BaseInstance } },
			{ Ast::BuiltinEntry::VertexIndex,    { "gl_VertexID",     GlslCapability::None } },
			{ Ast::BuiltinEntry::VertexPosition, { "gl_Position",     GlslCapability::None } }
		});

		struct GlslWriterPreVisitor : Ast::RecursiveVisitor
		{
			void Resolve()
			{
				usedStructs.Resize(bufferStructs.GetSize());
				usedStructs.PerformsNOT(usedStructs); //< ~
				bufferStructs &= usedStructs;
			}

			void SanitizeIdentifier(std::string& name)
			{
				while (reservedIdentifiers.find(name) != reservedIdentifiers.end())
					name += '_';

				reservedIdentifiers.insert(name);
			}

			using RecursiveVisitor::Visit;

			void Visit(Ast::CallFunctionExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				assert(currentFunction);
				currentFunction->calledFunctions.UnboundedSet(std::get<Ast::FunctionType>(*GetExpressionType(*node.targetFunction)).funcIndex);
			}

			void Visit(Ast::ConditionalExpression& /*node*/) override
			{
				throw std::runtime_error("unexpected conditional expression, is shader sanitized?");
			}

			void Visit(Ast::ConditionalStatement& /*node*/) override
			{
				throw std::runtime_error("unexpected conditional statement, is shader sanitized?");
			}

			void Visit(Ast::DeclareExternalStatement& node) override
			{
				for (const auto& extVar : node.externalVars)
				{
					const Ast::ExpressionType& type = extVar.type.GetResultingValue();
					if (IsStorageType(type))
					{
						capabilities.insert(GlslCapability::SSBO);
						bufferStructs.UnboundedSet(std::get<Ast::StorageType>(type).containedType.structIndex);
					}
					else if (IsUniformType(type))
						bufferStructs.UnboundedSet(std::get<Ast::UniformType>(type).containedType.structIndex);
				}

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::DeclareFunctionStatement& node) override
			{
				// Dismiss function if it's an entry point of another type than the one selected
				if (node.entryStage.HasValue())
				{
					if (selectedStage)
					{
						if (!node.entryStage.IsResultingValue())
							throw std::runtime_error("unexpected unresolved value for entry attribute, is shader sanitized?");

						ShaderStageType stage = node.entryStage.GetResultingValue();
						if (stage != *selectedStage)
							return;

						assert(!entryPoint);
						entryPoint = &node;
					}
					else
					{
						if (entryPoint)
							throw std::runtime_error("multiple entry point functions found, this is not allowed in GLSL, please select one");

						entryPoint = &node;
					}

					if (!node.parameters.empty())
					{
						assert(node.parameters.size() == 1);
						auto& parameter = node.parameters.front();
						const auto& parameterType = parameter.type.GetResultingValue();

						assert(std::holds_alternative<Ast::StructType>(parameterType));

						std::size_t structIndex = std::get<Ast::StructType>(parameterType).structIndex;
						const Ast::StructDescription* structDesc = Nz::Retrieve(structs, structIndex);

						for (const auto& member : structDesc->members)
						{
							if (member.cond.HasValue() && !member.cond.GetResultingValue())
								continue;

							if (member.builtin.HasValue())
							{
								auto it = s_glslBuiltinMapping.find(member.builtin.GetResultingValue());
								assert(it != s_glslBuiltinMapping.end());

								const GlslBuiltin& builtin = it->second;
								capabilities.insert(builtin.requiredCapability);
							}
						}
					}
				}

				assert(node.funcIndex);
				assert(functions.find(node.funcIndex.value()) == functions.end());
				FunctionData& funcData = functions[node.funcIndex.value()];
				funcData.name = node.name + moduleSuffix;
				funcData.node = &node;

				currentFunction = &funcData;

				RecursiveVisitor::Visit(node);

				currentFunction = nullptr;
			}

			void Visit(Ast::DeclareStructStatement& node) override
			{
				structs[node.structIndex.value()] = &node.description;

				for (const auto& member : node.description.members)
				{
					const Ast::ExpressionType& type = member.type.GetResultingValue();
					if (IsStorageType(type))
						usedStructs.UnboundedSet(std::get<Ast::StorageType>(type).containedType.structIndex);
					else if (IsUniformType(type))
						usedStructs.UnboundedSet(std::get<Ast::UniformType>(type).containedType.structIndex);
				}

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::DeclareVariableStatement& node) override
			{
				const Ast::ExpressionType& type = node.varType.GetResultingValue();
				if (IsStorageType(type))
					usedStructs.UnboundedSet(std::get<Ast::StorageType>(type).containedType.structIndex);
				else if (IsUniformType(type))
					usedStructs.UnboundedSet(std::get<Ast::UniformType>(type).containedType.structIndex);

				RecursiveVisitor::Visit(node);
			}

			struct FunctionData
			{
				std::string name;
				Nz::Bitset<> calledFunctions;
				Ast::DeclareFunctionStatement* node;
			};

			FunctionData* currentFunction = nullptr;

			tsl::ordered_set<GlslCapability> capabilities;
			std::optional<ShaderStageType> selectedStage;
			std::string moduleSuffix;
			std::unordered_map<std::size_t, FunctionData> functions;
			std::unordered_map<std::size_t, Ast::StructDescription*> structs;
			std::unordered_set<std::string> reservedIdentifiers;
			Nz::Bitset<> bufferStructs; //< structs used only in UBO/SSBO that shouldn't be declared as such in GLSL
			Nz::Bitset<> usedStructs; //< & with bufferStructs, to handle case where a UBO/SSBO struct is declared as a variable (which is allowed) or member of a struct
			Ast::DeclareFunctionStatement* entryPoint = nullptr;
		};
	}


	struct GlslWriter::State
	{
		State(const GlslWriter::BindingMapping& bindings) :
		bindingMapping(bindings)
		{
			reservedKeywords = {
				// All reserved GLSL keywords as of GLSL ES 3.2
				"active", "asm", "atomic_uint", "attribute", "bool", "break", "buffer", "bvec2", "bvec3", "bvec4", "case", "cast", "centroid", "class", "coherent", "common", "const", "continue", "default", "discard", "dmat2", "dmat2x2", "dmat2x3", "dmat2x4", "dmat3", "dmat3x2", "dmat3x3", "dmat3x4", "dmat4", "dmat4x2", "dmat4x3", "dmat4x4", "do", "double", "dvec2", "dvec3", "dvec4", "else", "enum", "extern", "external", "false", "filter", "fixed", "flat", "float", "for", "fvec2", "fvec3", "fvec4", "goto", "half", "highp", "hvec2", "hvec3", "hvec4", "if", "iimage1D", "iimage1DArray", "iimage2D", "iimage2DArray", "iimage2DMS", "iimage2DMSArray", "iimage2DRect", "iimage3D", "iimageBuffer", "iimageCube", "iimageCubeArray", "image1D", "image1DArray", "image2D", "image2DArray", "image2DMS", "image2DMSArray", "image2DRect", "image3D", "imageBuffer", "imageCube", "imageCubeArray", "in", "inline", "inout", "input", "int", "interface", "invariant", "isampler1D", "isampler1DArray", "isampler2D", "isampler2DArray", "isampler2DMS", "isampler2DMSArray", "isampler2DRect", "isampler3D", "isamplerBuffer", "isamplerCube", "isamplerCubeArray", "isubpassInput", "isubpassInputMS", "itexture2D", "itexture2DArray", "itexture2DMS", "itexture2DMSArray", "itexture3D", "itextureBuffer", "itextureCube", "itextureCubeArray", "ivec2", "ivec3", "ivec4", "layout", "long", "lowp", "mat2", "mat2x2", "mat2x3", "mat2x4", "mat3", "mat3x2", "mat3x3", "mat3x4", "mat4", "mat4x2", "mat4x3", "mat4x4", "mediump", "namespace", "noinline", "noperspective", "out", "output", "partition", "patch", "precise", "precision", "public", "readonly", "resource", "restrict", "return", "sample", "sampler", "sampler1D", "sampler1DArray", "sampler1DArrayShadow", "sampler1DShadow", "sampler2D", "sampler2DArray", "sampler2DArrayShadow", "sampler2DMS", "sampler2DMSArray", "sampler2DRect", "sampler2DRectShadow", "sampler2DShadow", "sampler3D", "sampler3DRect", "samplerBuffer", "samplerCube", "samplerCubeArray", "samplerCubeArrayShadow", "samplerCubeShadow", "samplerShadow", "shared", "short", "sizeof", "smooth", "static", "struct", "subpassInput", "subpassInputMS", "subroutine", "superp", "switch", "template", "texture2D", "texture2DArray", "texture2DMS", "texture2DMSArray", "texture3D", "textureBuffer", "textureCube", "textureCubeArray", "this", "true", "typedef", "uimage1D", "uimage1DArray", "uimage2D", "uimage2DArray", "uimage2DMS", "uimage2DMSArray", "uimage2DRect", "uimage3D", "uimageBuffer", "uimageCube", "uimageCubeArray", "uint", "uniform", "union", "unsigned", "usampler1D", "usampler1DArray", "usampler2D", "usampler2DArray", "usampler2DMS", "usampler2DMSArray", "usampler2DRect", "usampler3D", "usamplerBuffer", "usamplerCube", "usamplerCubeArray", "using", "usubpassInput", "usubpassInputMS", "utexture2D", "utexture2DArray", "utexture2DMS", "utexture2DMSArray", "utexture3D", "utextureBuffer", "utextureCube", "utextureCubeArray", "uvec2", "uvec3", "uvec4", "varying", "vec2", "vec3", "vec4", "void", "volatile", "while", "writeonly",
				// GLSL intrinsic functions (WIP)
				"cross", "dot", "exp", "inverse", "length", "max", "min", "mod", "normalize", "pow", "texture", "transpose"
			};
		}

		struct InOutField
		{
			std::string memberName;
			std::variant<Ast::BuiltinEntry, std::string> targetName;
		};

		struct StructData
		{
			std::string nameOverride;
			const Ast::StructDescription* desc;
		};

		std::string moduleSuffix;
		std::stringstream stream;
		std::vector<InOutField> inputFields;
		std::vector<InOutField> outputFields;
		std::unordered_map<std::size_t, std::string> constantNames;
		std::unordered_map<std::size_t, StructData> structs;
		std::unordered_map<std::size_t, std::string> variableNames;
		std::unordered_map<std::string, unsigned int> explicitUniformBlockBinding;
		std::unordered_set<std::string> reservedKeywords;
		Nz::Bitset<> declaredFunctions;
		const GlslWriter::BindingMapping& bindingMapping;
		GlslWriterPreVisitor previsitor;
		ShaderStageType stage;
		const States* states = nullptr;
		bool requiresExplicitUniformBinding = false;
		bool supportsVaryingLocations = true;
		bool isInEntryPoint = false;
		bool hasDrawParametersBaseInstanceUniform = false;
		bool hasDrawParametersBaseVertexUniform = false;
		bool hasDrawParametersDrawIndexUniform = false;
		int streamEmptyLine = 1;
		unsigned int indentLevel = 0;
	};

	auto GlslWriter::Generate(std::optional<ShaderStageType> shaderStage, const Ast::Module& module, const BindingMapping& bindingMapping, const States& states) -> GlslWriter::Output
	{
		State state(bindingMapping);

		m_currentState = &state;
		Nz::CallOnExit onExit([this]()
		{
			m_currentState = nullptr;
		});

		Ast::ModulePtr sanitizedModule;
		const Ast::Module* targetModule;
		if (!states.sanitized)
		{
			Ast::SanitizeVisitor::Options options = GetSanitizeOptions();
			options.optionValues = states.optionValues;
			options.moduleResolver = states.shaderModuleResolver;

			sanitizedModule = Ast::Sanitize(module, options);
			targetModule = sanitizedModule.get();
		}
		else
			targetModule = &module;

		if (states.optimize)
		{
			sanitizedModule = Ast::PropagateConstants(*targetModule);

			Ast::DependencyCheckerVisitor::Config dependencyConfig;
			dependencyConfig.usedShaderStages = (shaderStage) ? *shaderStage : ShaderStageType_All; //< only one should exist anyway

			sanitizedModule = Ast::EliminateUnusedPass(*sanitizedModule, dependencyConfig);

			targetModule = sanitizedModule.get();
		}

		// Previsitor
		state.previsitor.selectedStage = shaderStage;

		for (const auto& importedModule : targetModule->importedModules)
		{
			state.previsitor.moduleSuffix = importedModule.identifier;
			importedModule.module->rootNode->Visit(state.previsitor);
		}

		state.previsitor.moduleSuffix = {};
		targetModule->rootNode->Visit(state.previsitor);

		state.previsitor.Resolve();

		if (!state.previsitor.entryPoint)
			throw std::runtime_error("no entry point found");

		assert(state.previsitor.entryPoint->entryStage.HasValue());
		m_currentState->stage = state.previsitor.entryPoint->entryStage.GetResultingValue();

		// Code generation
		AppendHeader();

		for (const auto& importedModule : targetModule->importedModules)
		{
			AppendComment("Module " + importedModule.module->metadata->moduleName);
			AppendModuleComments(*importedModule.module->metadata);
			AppendLine();

			m_currentState->moduleSuffix = importedModule.identifier;
			importedModule.module->rootNode->Visit(*this);

			AppendLine();
		}

		if (!targetModule->importedModules.empty())
		{
			AppendComment("Main module");
			AppendModuleComments(*module.metadata);
			AppendLine();
		}
		else
			AppendModuleComments(*module.metadata);

		m_currentState->moduleSuffix = {};
		targetModule->rootNode->Visit(*this);

		Output output;
		output.code = std::move(state.stream).str();
		output.explicitUniformBlockBinding = std::move(state.explicitUniformBlockBinding);
		output.usesDrawParameterBaseInstanceUniform = m_currentState->hasDrawParametersBaseInstanceUniform;
		output.usesDrawParameterBaseVertexUniform = m_currentState->hasDrawParametersBaseVertexUniform;
		output.usesDrawParameterDrawIndexUniform = m_currentState->hasDrawParametersDrawIndexUniform;

		return output;
	}

	void GlslWriter::SetEnv(Environment environment)
	{
		m_environment = std::move(environment);
	}

	std::string_view GlslWriter::GetDrawParameterBaseInstanceUniformName()
	{
		return s_glslWriterShaderDrawParametersBaseInstanceName;
	}

	std::string_view GlslWriter::GetDrawParameterBaseVertexUniformName()
	{
		return s_glslWriterShaderDrawParametersBaseVertexName;
	}

	std::string_view GlslWriter::GetDrawParameterDrawIndexUniformName()
	{
		return s_glslWriterShaderDrawParametersDrawIndexName;
	}

	std::string_view GlslWriter::GetFlipYUniformName()
	{
		return s_glslWriterFlipYUniformName;
	}

	Ast::SanitizeVisitor::Options GlslWriter::GetSanitizeOptions()
	{
		// Always sanitize for reserved identifiers
		Ast::SanitizeVisitor::Options options;
		options.makeVariableNameUnique = true;
		options.reduceLoopsToWhile = true;
		options.removeAliases = true;
		options.removeCompoundAssignments = false;
		options.removeOptionDeclaration = true;
		options.removeScalarSwizzling = true;
		options.removeSingleConstDeclaration = true;

		return options;
	}

	void GlslWriter::Append(const Ast::AliasType& /*aliasType*/)
	{
		throw std::runtime_error("unexpected AliasType");
	}

	void GlslWriter::Append(const Ast::ArrayType& type)
	{
		AppendArray(type);
	}

	void GlslWriter::Append(Ast::BuiltinEntry builtin)
	{
		unsigned int glVersion = m_environment.glMajorVersion * 100 + m_environment.glMinorVersion * 10;

		switch (builtin)
		{
			case Ast::BuiltinEntry::BaseInstance:
			{
				if (m_currentState->hasDrawParametersBaseInstanceUniform)
					Append(s_glslWriterShaderDrawParametersBaseInstanceName);
				else if (!m_environment.glES && glVersion >= 460)
					Append("gl_BaseInstance");
				else
					Append("gl_BaseInstanceARB");
				break;
			}

			case Ast::BuiltinEntry::BaseVertex:
			{
				if (m_currentState->hasDrawParametersBaseVertexUniform)
					Append(s_glslWriterShaderDrawParametersBaseVertexName);
				else if (!m_environment.glES && glVersion >= 460)
					Append("gl_BaseVertex");
				else
					Append("gl_BaseVertexARB");
				break;
			}

			case Ast::BuiltinEntry::DrawIndex:
			{
				if (m_currentState->hasDrawParametersDrawIndexUniform)
					Append(s_glslWriterShaderDrawParametersDrawIndexName);
				else if (!m_environment.glES && glVersion >= 460)
					Append("gl_DrawID");
				else
					Append("gl_DrawIDARB");
				break;
			}

			case Ast::BuiltinEntry::InstanceIndex:
			{
				Append("(", Ast::BuiltinEntry::BaseInstance, " + gl_InstanceID)");
				break;
			}

			default:
			{
				auto it = s_glslBuiltinMapping.find(builtin);
				assert(it != s_glslBuiltinMapping.end());
				Append(it->second.identifier);
			}
		}
	}

	void GlslWriter::Append(const Ast::DynArrayType& type)
	{
		AppendArray(type);
	}

	void GlslWriter::Append(const Ast::ExpressionType& type)
	{
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, type);
	}

	void GlslWriter::Append(const Ast::ExpressionValue<Ast::ExpressionType>& type)
	{
		Append(type.GetResultingValue());
	}

	void GlslWriter::Append(const Ast::FunctionType& /*functionType*/)
	{
		throw std::runtime_error("unexpected FunctionType");
	}

	void GlslWriter::Append(const Ast::IntrinsicFunctionType& /*intrinsicFunctionType*/)
	{
		throw std::runtime_error("unexpected intrinsic function type");
	}

	void GlslWriter::Append(const Ast::MatrixType& matrixType)
	{
		if (matrixType.columnCount == matrixType.rowCount)
		{
			Append("mat");
			Append(matrixType.columnCount);
		}
		else
		{
			Append("mat");
			Append(matrixType.columnCount);
			Append("x");
			Append(matrixType.rowCount);
		}
	}

	void GlslWriter::Append(const Ast::MethodType& /*methodType*/)
	{
		throw std::runtime_error("unexpected method type");
	}

	void GlslWriter::Append(Ast::PrimitiveType type)
	{
		switch (type)
		{
			case Ast::PrimitiveType::Boolean: return Append("bool");
			case Ast::PrimitiveType::Float32: return Append("float");
			case Ast::PrimitiveType::Int32:   return Append("int");
			case Ast::PrimitiveType::UInt32:  return Append("uint");
			case Ast::PrimitiveType::String:  throw std::runtime_error("unexpected string constant");
		}
	}

	void GlslWriter::Append(const Ast::SamplerType& samplerType)
	{
		switch (samplerType.sampledType)
		{
			case Ast::PrimitiveType::Boolean:
			case Ast::PrimitiveType::Float32:
				break;

			case Ast::PrimitiveType::Int32:   Append("i"); break;
			case Ast::PrimitiveType::UInt32:  Append("u"); break;

			case Ast::PrimitiveType::String:  throw std::runtime_error("unexpected string type");
		}

		Append("sampler");

		switch (samplerType.dim)
		{
			case ImageType::E1D:       Append("1D");      break;
			case ImageType::E1D_Array: Append("1DArray"); break;
			case ImageType::E2D:       Append("2D");      break;
			case ImageType::E2D_Array: Append("2DArray"); break;
			case ImageType::E3D:       Append("3D");      break;
			case ImageType::Cubemap:   Append("Cube");    break;
		}
	}

	void GlslWriter::Append(const Ast::StorageType& /*storageType*/)
	{
		throw std::runtime_error("unexpected StorageType");
	}

	void GlslWriter::Append(const Ast::StructType& structType)
	{
		const auto& structData = Nz::Retrieve(m_currentState->structs, structType.structIndex);
		Append(structData.nameOverride);
	}

	void GlslWriter::Append(const Ast::Type& /*type*/)
	{
		throw std::runtime_error("unexpected Type");
	}

	void GlslWriter::Append(const Ast::UniformType& /*uniformType*/)
	{
		throw std::runtime_error("unexpected UniformType");
	}

	void GlslWriter::Append(const Ast::VectorType& vecType)
	{
		switch (vecType.type)
		{
			case Ast::PrimitiveType::Boolean: Append("b"); break;
			case Ast::PrimitiveType::Float32: break;
			case Ast::PrimitiveType::Int32:   Append("i"); break;
			case Ast::PrimitiveType::UInt32:  Append("u"); break;
			case Ast::PrimitiveType::String:  throw std::runtime_error("unexpected string type");
		}

		Append("vec");
		Append(vecType.componentCount);
	}

	void GlslWriter::Append(Ast::MemoryLayout layout)
	{
		switch (layout)
		{
			case Ast::MemoryLayout::Std140:
				Append("std140");
				break;
		}
	}

	void GlslWriter::Append(Ast::NoType)
	{
		return Append("void");
	}

	template<typename T>
	void GlslWriter::Append(const T& param)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (m_currentState->streamEmptyLine > 0)
		{
			for (std::size_t i = 0; i < m_currentState->indentLevel; ++i)
				m_currentState->stream << '\t';

			m_currentState->streamEmptyLine = 0;
		}

		m_currentState->stream << param;
	}

	template<typename T1, typename T2, typename... Args>
	void GlslWriter::Append(const T1& firstParam, const T2& secondParam, Args&&... params)
	{
		Append(firstParam);
		Append(secondParam, std::forward<Args>(params)...);
	}

	template<typename ...Args>
	void GlslWriter::Append(const std::variant<Args...>& param)
	{
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, param);
	}

	void GlslWriter::AppendArray(const Ast::ExpressionType& type, const std::string& varName)
	{
		std::vector<std::uint32_t> lengths;

		const Ast::ExpressionType* exprType = &type;
		for (;;)
		{
			if (Ast::IsArrayType(*exprType))
			{
				const auto& arrayType = std::get<Ast::ArrayType>(*exprType);
				lengths.push_back(arrayType.length);

				exprType = &arrayType.containedType->type;
			}
			else if (Ast::IsDynArrayType(*exprType))
			{
				const auto& arrayType = std::get<Ast::DynArrayType>(*exprType);
				lengths.push_back(0);

				exprType = &arrayType.containedType->type;
			}
			else
				break;
		}

		assert(!Ast::IsArrayType(*exprType) && !Ast::IsDynArrayType(*exprType));
		Append(*exprType);

		if (!varName.empty())
			Append(" ", varName);

		for (std::uint32_t lengthAttribute : lengths)
		{
			Append("[");
			if (lengthAttribute > 0)
				Append(lengthAttribute);
			Append("]");
		}
	}

	void GlslWriter::AppendComment(std::string_view section)
	{
		std::size_t lineFeed = section.find('\n');
		if (lineFeed != section.npos)
		{
			std::size_t previousCut = 0;

			AppendLine("/*");
			do
			{
				AppendLine(section.substr(previousCut, lineFeed - previousCut));
				previousCut = lineFeed + 1;
			} while ((lineFeed = section.find('\n', previousCut)) != section.npos);
			AppendLine(section.substr(previousCut));
			AppendLine("*/");
		}
		else
			AppendLine("// ", section);
	}

	void GlslWriter::AppendCommentSection(std::string_view section)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		std::string stars((section.size() < 33) ? (36 - section.size()) / 2 : 3, '*');
		Append("/*", stars, ' ', section, ' ', stars, "*/");
		AppendLine();
	}

	void GlslWriter::AppendFunctionDeclaration(const Ast::DeclareFunctionStatement& node, const std::string& nameOverride, bool forward)
	{
		Append(node.returnType, " ", SanitizeIdentifier(nameOverride), "(");

		bool first = true;
		for (const auto& parameter : node.parameters)
		{
			if (!first)
				Append(", ");

			first = false;

			AppendVariableDeclaration(parameter.type.GetResultingValue(), SanitizeIdentifier(parameter.name));
		}
		AppendLine((forward) ? ");" : ")");
	}

	void GlslWriter::AppendHeader()
	{
		unsigned int glslVersion;
		unsigned int glVersion = m_environment.glMajorVersion * 100 + m_environment.glMinorVersion * 10;
		if (m_environment.glES)
		{
			if (glVersion >= 300)
				glslVersion = glVersion;
			else if (m_environment.glMajorVersion >= 2)
				glslVersion = 100;
			else
				throw std::runtime_error("This version of OpenGL ES does not support shaders");
		}
		else
		{
			if (glVersion >= 330)
				glslVersion = glVersion;
			else if (glVersion >= 320)
				glslVersion = 150;
			else if (glVersion >= 310)
				glslVersion = 140;
			else if (glVersion >= 300)
				glslVersion = 130;
			else if (glVersion >= 210)
				glslVersion = 120;
			else if (glVersion >= 200)
				glslVersion = 110;
			else
				throw std::runtime_error("This version of OpenGL does not support shaders");
		}

		// Header
		Append("#version ");
		Append(glslVersion);
		if (m_environment.glES)
			Append(" es");

		AppendLine();
		AppendLine();

		// Comments
		std::string fileTitle;

		switch (m_currentState->stage)
		{
			case ShaderStageType::Fragment: fileTitle += "fragment shader - "; break;
			case ShaderStageType::Vertex: fileTitle += "vertex shader - "; break;
		}

		fileTitle += "this file was generated by NZSL compiler (Nazara Shading Language)";

		AppendComment(fileTitle);
		AppendLine();

		// Extensions

		tsl::ordered_set<std::string> requiredExtensions;
		
		for (GlslCapability capability : m_currentState->previsitor.capabilities)
		{
			switch (capability)
			{
				case GlslCapability::None:
					break;

				case GlslCapability::ShaderDrawParameters_BaseInstance:
				{
					if (m_environment.glES)
						m_currentState->hasDrawParametersBaseInstanceUniform = true;
					else if (glslVersion < 460)
					{
						if (m_environment.extCallback && m_environment.extCallback("GL_ARB_shader_draw_parameters"))
							requiredExtensions.emplace("GL_ARB_shader_draw_parameters");
						else
							m_currentState->hasDrawParametersBaseInstanceUniform = true;
					}

					break;
				}

				case GlslCapability::ShaderDrawParameters_BaseVertex:
				{
					if (m_environment.glES)
						m_currentState->hasDrawParametersBaseVertexUniform = true;
					else if (glslVersion < 460)
					{
						if (m_environment.extCallback && m_environment.extCallback("GL_ARB_shader_draw_parameters"))
							requiredExtensions.emplace("GL_ARB_shader_draw_parameters");
						else
							m_currentState->hasDrawParametersBaseVertexUniform = true;
					}

					break;
				}

				case GlslCapability::ShaderDrawParameters_DrawIndex:
				{
					if (m_environment.glES)
						m_currentState->hasDrawParametersDrawIndexUniform = true;
					else if (glslVersion < 460)
					{
						if (m_environment.extCallback && m_environment.extCallback("GL_ARB_shader_draw_parameters"))
							requiredExtensions.emplace("GL_ARB_shader_draw_parameters");
						else
							m_currentState->hasDrawParametersDrawIndexUniform = true;
					}

					break;
				}

				case GlslCapability::SSBO:
				{
					if (m_environment.glES)
					{
						if (glslVersion < 310)
							throw std::runtime_error("this version of OpenGL ES does not support SSBO");
					}
					else if (glslVersion < 430)
					{
						if (m_environment.extCallback && m_environment.extCallback("GL_ARB_shader_storage_buffer_object"))
							requiredExtensions.emplace("GL_ARB_shader_storage_buffer_object");
						else
							throw std::runtime_error("this version of OpenGL does not support SSBO");
					}
					
					break;
				}
			}
		}

		if ((m_currentState->hasDrawParametersBaseInstanceUniform || m_currentState->hasDrawParametersBaseVertexUniform || m_currentState->hasDrawParametersDrawIndexUniform) && !m_environment.allowDrawParametersUniformsFallback)
			throw std::runtime_error("draw parameters are used but not supported and fallback uniforms are disabled, cannot continue");

		if (m_environment.glES)
		{
			// GL_ARB_separate_shader_objects (required for layout(location = X))
			if (glslVersion < 310)
				m_currentState->supportsVaryingLocations = false;
		}
		else
		{
			// GL_ARB_shading_language_420pack (required for layout(binding = X))
			if (glslVersion < 420)
			{
				if (m_environment.extCallback && m_environment.extCallback("GL_ARB_shading_language_420pack"))
					requiredExtensions.emplace("GL_ARB_shading_language_420pack");
				else
					m_currentState->requiresExplicitUniformBinding = true;
			}

			// GL_ARB_separate_shader_objects (required for layout(location = X))
			if (glslVersion < 410)
			{
				if (m_environment.extCallback && m_environment.extCallback("GL_ARB_separate_shader_objects"))
					requiredExtensions.emplace("GL_ARB_separate_shader_objects");
				else
					m_currentState->supportsVaryingLocations = false;
			}
		}

		if (!requiredExtensions.empty())
		{
			for (std::string_view ext : requiredExtensions)
				AppendLine("#extension ", ext, " : require");

			AppendLine();
		}

		if (m_environment.glES)
		{
			AppendLine("#if GL_FRAGMENT_PRECISION_HIGH");
			AppendLine("precision highp float;");
			AppendLine("#else");
			AppendLine("precision mediump float;");
			AppendLine("#endif");
			AppendLine();
		}

		if (m_currentState->hasDrawParametersBaseInstanceUniform || m_currentState->hasDrawParametersBaseVertexUniform || m_currentState->hasDrawParametersDrawIndexUniform)
		{
			if (m_currentState->hasDrawParametersBaseInstanceUniform)
				AppendLine("uniform int ", s_glslWriterShaderDrawParametersBaseInstanceName, ";");

			if (m_currentState->hasDrawParametersBaseVertexUniform)
				AppendLine("uniform int ", s_glslWriterShaderDrawParametersBaseVertexName, ";");

			if (m_currentState->hasDrawParametersDrawIndexUniform)
				AppendLine("uniform int ", s_glslWriterShaderDrawParametersDrawIndexName, ";");

			AppendLine();
		}
	}

	void GlslWriter::AppendLine(std::string_view txt)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (txt.empty() && m_currentState->streamEmptyLine > 1)
			return;

		m_currentState->stream << txt << '\n';
		m_currentState->streamEmptyLine++;
	}

	template<typename... Args>
	void GlslWriter::AppendLine(Args&&... params)
	{
		(Append(std::forward<Args>(params)), ...);
		AppendLine();
	}

	template<typename T>
	void GlslWriter::AppendValue(const T& value)
	{
		if constexpr (std::is_same_v<T, Vector2i32> || std::is_same_v<T, Vector3i32> || std::is_same_v<T, Vector4i32>)
			Append("i"); //< ivec
		else if constexpr (std::is_same_v<T, Vector2u32> || std::is_same_v<T, Vector3u32> || std::is_same_v<T, Vector4u32>)
			Append("i"); //< uvec

		if constexpr (std::is_same_v<T, Ast::NoValue>)
			throw std::runtime_error("invalid type (value expected)");
		else if constexpr (std::is_same_v<T, std::string>)
			throw std::runtime_error("unexpected string litteral");
		else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, std::vector<bool>::reference>)
			Append((value) ? "true" : "false");
		else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>)
		{
			Append(Ast::ToString(value));
			if constexpr (std::is_same_v<T, std::uint32_t>)
				Append("u");
		}
		else if constexpr (std::is_same_v<T, Vector2f32> || std::is_same_v<T, Vector2i32>)
			Append("vec2(" + Ast::ToString(value.x()) + ", " + Ast::ToString(value.y()) + ")");
		else if constexpr (std::is_same_v<T, Vector3f32> || std::is_same_v<T, Vector3i32>)
			Append("vec3(" + Ast::ToString(value.x()) + ", " + Ast::ToString(value.y()) + ", " + Ast::ToString(value.z()) + ")");
		else if constexpr (std::is_same_v<T, Vector4f32> || std::is_same_v<T, Vector4i32>)
			Append("vec4(" + Ast::ToString(value.x()) + ", " + Ast::ToString(value.y()) + ", " + Ast::ToString(value.z()) + ", " + Ast::ToString(value.w()) + ")");
		else
			static_assert(Nz::AlwaysFalse<T>(), "non-exhaustive visitor");
	}

	void GlslWriter::AppendModuleComments(const Ast::Module::Metadata& metadata)
	{
		if (!metadata.author.empty())
			AppendComment("Author: " + metadata.author);

		if (!metadata.description.empty())
			AppendComment("Description: " + metadata.description);

		if (!metadata.license.empty())
			AppendComment("License: " + metadata.license);
	}

	void GlslWriter::AppendStatementList(std::vector<Ast::StatementPtr>& statements)
	{
		bool first = true;
		for (const Ast::StatementPtr& statement : statements)
		{
			if (statement->GetType() == Ast::NodeType::NoOpStatement)
				continue;

			if (!first)
				AppendLine();

			statement->Visit(*this);

			first = false;
		}
	}

	void GlslWriter::AppendVariableDeclaration(const Ast::ExpressionType& varType, const std::string& varName)
	{
		if (Ast::IsArrayType(varType) || Ast::IsDynArrayType(varType))
			AppendArray(varType, varName);
		else
			Append(varType, " ", varName);
	}

	void GlslWriter::EnterScope()
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		AppendLine("{");
		m_currentState->indentLevel++;
	}

	void GlslWriter::LeaveScope(bool skipLine)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		m_currentState->indentLevel--;
		AppendLine();

		if (skipLine)
			AppendLine("}");
		else
			Append("}");
	}

	void GlslWriter::HandleEntryPoint(Ast::DeclareFunctionStatement& node)
	{
		if (node.entryStage.GetResultingValue() == ShaderStageType::Fragment && node.earlyFragmentTests.HasValue() && node.earlyFragmentTests.GetResultingValue())
		{
			unsigned int glVersion = m_environment.glMajorVersion * 100 + m_environment.glMinorVersion * 10;
			if ((m_environment.glES && glVersion >= 310) || (!m_environment.glES && glVersion >= 420) || (m_environment.extCallback && m_environment.extCallback("GL_ARB_shader_image_load_store")))
			{
				AppendLine("layout(early_fragment_tests) in;");
				AppendLine();
			}
		}

		HandleInOut();
		AppendLine("void main()");
		EnterScope();
		{
			if (!m_currentState->inputFields.empty())
			{
				assert(!node.parameters.empty());

				auto& parameter = node.parameters.front();
				std::string varName = SanitizeIdentifier(parameter.name);
				RegisterVariable(*parameter.varIndex, varName);

				assert(IsStructType(parameter.type.GetResultingValue()));
				std::size_t structIndex = std::get<Ast::StructType>(parameter.type.GetResultingValue()).structIndex;
				const auto& structData = Nz::Retrieve(m_currentState->structs, structIndex);

				AppendLine(structData.nameOverride, " ", varName, ";");
				for (const auto& [memberName, targetName] : m_currentState->inputFields)
					AppendLine(varName, ".", memberName, " = ", targetName, ";");

				AppendLine();
			}

			// Output struct is handled on return node
			m_currentState->isInEntryPoint = true;

			AppendStatementList(node.statements);

			m_currentState->isInEntryPoint = false;
		}
		LeaveScope();
	}

	void GlslWriter::HandleInOut()
	{
		auto AppendInOut = [this](bool in, const State::StructData& structData, std::vector<State::InOutField>& fields, std::string_view targetPrefix)
		{
			bool empty = true;

			for (const auto& member : structData.desc->members)
			{
				if (member.cond.HasValue() && !member.cond.GetResultingValue())
					continue;

				if (member.builtin.HasValue())
				{
					auto it = Ast::s_builtinData.find(member.builtin.GetResultingValue());
					assert(it != Ast::s_builtinData.end());

					const Ast::BuiltinData& builtin = it->second;
					if (!builtin.compatibleStages.Test(m_currentState->stage))
						continue; //< This builtin is not active in this stage, skip it

					fields.push_back({
						member.name,
						it->first
					});
				}
				else
				{
					if (empty)
						AppendCommentSection((in) ? "Inputs" : "Outputs");

					std::string varName = SanitizeIdentifier(std::string(targetPrefix) + member.name);

					auto OutputVariable = [&](auto&&... arg)
					{
						Append((in) ? "in" : "out", " ");
						AppendVariableDeclaration(member.type.GetResultingValue(), varName);
						AppendLine(";", arg...);
					};

					if (member.locationIndex.HasValue())
					{
						bool isSupported = m_currentState->supportsVaryingLocations
						                || (in && m_currentState->stage == nzsl::ShaderStageType::Vertex)
						                || (!in && m_currentState->stage == nzsl::ShaderStageType::Fragment);

						if (isSupported)
						{
							Append("layout(location = ");
							Append(member.locationIndex.GetResultingValue());
							Append(") ");

							OutputVariable();
						}
						else
						{
							std::string originalName = std::move(varName);
							varName = "_nzslVarying_" + std::to_string(member.locationIndex.GetResultingValue());
							OutputVariable(" // ", originalName);
						}
					}
					else
						OutputVariable();
					
					fields.push_back({
						member.name,
						varName
					});

					empty = false;
				}
			}

			if (!empty)
				AppendLine();
		};

		const Ast::DeclareFunctionStatement& node = *m_currentState->previsitor.entryPoint;

		if (!node.parameters.empty())
		{
			assert(node.parameters.size() == 1);
			auto& parameter = node.parameters.front();
			assert(std::holds_alternative<Ast::StructType>(parameter.type.GetResultingValue()));
			std::size_t inputStructIndex = std::get<Ast::StructType>(parameter.type.GetResultingValue()).structIndex;
			
			const auto& inputStruct = Nz::Retrieve(m_currentState->structs, inputStructIndex);
			AppendInOut(true, inputStruct, m_currentState->inputFields, s_glslWriterInputPrefix);
		}

		if (m_currentState->stage == ShaderStageType::Vertex && m_environment.flipYPosition)
		{
			AppendLine("uniform float ", s_glslWriterFlipYUniformName, ";");
			AppendLine();
		}

		if (node.returnType.HasValue() && !IsNoType(node.returnType.GetResultingValue()))
		{
			assert(std::holds_alternative<Ast::StructType>(node.returnType.GetResultingValue()));
			std::size_t outputStructIndex = std::get<Ast::StructType>(node.returnType.GetResultingValue()).structIndex;
			const auto& outputStruct = Nz::Retrieve(m_currentState->structs, outputStructIndex);
			
			AppendInOut(false, outputStruct, m_currentState->outputFields, s_glslWriterOutputPrefix);
		}
	}

	void GlslWriter::RegisterConstant(std::size_t constIndex, std::string constName)
	{
		assert(m_currentState->constantNames.find(constIndex) == m_currentState->constantNames.end());
		m_currentState->constantNames.emplace(constIndex, std::move(constName));
	}

	void GlslWriter::RegisterStruct(std::size_t structIndex, Ast::StructDescription* desc, std::string structName)
	{
		assert(m_currentState->structs.find(structIndex) == m_currentState->structs.end());
		State::StructData structData;
		structData.desc = desc;
		structData.nameOverride = std::move(structName);

		m_currentState->structs.emplace(structIndex, std::move(structData));
	}

	void GlslWriter::RegisterVariable(std::size_t varIndex, std::string varName)
	{
		assert(m_currentState->variableNames.find(varIndex) == m_currentState->variableNames.end());
		m_currentState->variableNames.emplace(varIndex, std::move(varName));
	}

	std::string GlslWriter::SanitizeIdentifier(std::string identifier)
	{
		while (m_currentState->reservedKeywords.find(identifier) != m_currentState->reservedKeywords.end())
			identifier += "_";

		return identifier;
	}

	void GlslWriter::ScopeVisit(Ast::Statement& node)
	{
		if (node.GetType() != Ast::NodeType::ScopedStatement)
		{
			EnterScope();
			node.Visit(*this);
			LeaveScope(true);
		}
		else
			node.Visit(*this);
	}

	void GlslWriter::Visit(Ast::ExpressionPtr& expr, bool encloseIfRequired)
	{
		bool enclose = encloseIfRequired && (GetExpressionCategory(*expr) != Ast::ExpressionCategory::LValue);

		if (enclose)
			Append("(");

		expr->Visit(*this);

		if (enclose)
			Append(")");
	}

	void GlslWriter::Visit(Ast::AccessIdentifierExpression& node)
	{
		Visit(node.expr, true);

		const Ast::ExpressionType* exprType = GetExpressionType(*node.expr);
		NazaraUnused(exprType);
		assert(exprType);
		assert(IsStructType(*exprType));

		for (const auto& identifierEntry : node.identifiers)
			Append(".", identifierEntry.identifier);
	}

	void GlslWriter::Visit(Ast::AccessIndexExpression& node)
	{
		Visit(node.expr, true);

		const Ast::ExpressionType* exprType = GetExpressionType(*node.expr);
		NazaraUnused(exprType);
		assert(exprType);
		assert(!IsStructType(*exprType));

		// Array access
		assert(node.indices.size() == 1);
		Append("[");
		Visit(node.indices.front());
		Append("]");
	}

	void GlslWriter::Visit(Ast::AliasValueExpression& /*node*/)
	{
		// all aliases should have been handled by sanitizer
		throw std::runtime_error("unexpected alias value, is shader sanitized?");
	}

	void GlslWriter::Visit(Ast::AssignExpression& node)
	{
		// Special case, GLSL modulo on floating point requires a call to the mod intrinsic
		if (node.op == Ast::AssignType::CompoundModulo)
		{
			bool isFmod = false;
			if (IsPrimitiveType(*node.cachedExpressionType))
			{
				Ast::PrimitiveType primitiveType = std::get<Ast::PrimitiveType>(*node.cachedExpressionType);
				if (primitiveType == Ast::PrimitiveType::Float32)
					isFmod = true;
			}
			else if (IsVectorType(*node.cachedExpressionType))
			{
				Ast::PrimitiveType primitiveType = std::get<Ast::VectorType>(*node.cachedExpressionType).type;
				if (primitiveType == Ast::PrimitiveType::Float32)
					isFmod = true;
			}
			else
				throw std::runtime_error("unexpected type for modulo");

			if (isFmod)
			{
				node.left->Visit(*this);
				Append(" = mod(");
				node.left->Visit(*this);
				Append(", ");
				Visit(node.right);
				Append(")");

				return; //< prevent normal generation
			}
		}

		node.left->Visit(*this);

		switch (node.op)
		{
			case Ast::AssignType::Simple:             Append(" = "); break;
			case Ast::AssignType::CompoundAdd:        Append(" += "); break;
			case Ast::AssignType::CompoundDivide:     Append(" /= "); break;
			case Ast::AssignType::CompoundModulo:     Append(" %= "); break;
			case Ast::AssignType::CompoundMultiply:   Append(" *= "); break;
			case Ast::AssignType::CompoundLogicalAnd: Append(" &&= "); break;
			case Ast::AssignType::CompoundLogicalOr:  Append(" ||= "); break;
			case Ast::AssignType::CompoundSubtract:   Append(" -= "); break;
		}

		node.right->Visit(*this);
	}

	void GlslWriter::Visit(Ast::BinaryExpression& node)
	{
		// Special case, GLSL modulo on floating point requires a call to the mod intrinsic
		if (node.op == Ast::BinaryType::Modulo)
		{
			bool isFmod = false;
			if (IsPrimitiveType(*node.cachedExpressionType))
			{
				Ast::PrimitiveType primitiveType = std::get<Ast::PrimitiveType>(*node.cachedExpressionType);
				if (primitiveType == Ast::PrimitiveType::Float32)
					isFmod = true;
			}
			else if (IsVectorType(*node.cachedExpressionType))
			{
				Ast::PrimitiveType primitiveType = std::get<Ast::VectorType>(*node.cachedExpressionType).type;
				if (primitiveType == Ast::PrimitiveType::Float32)
					isFmod = true;
			}
			else
				throw std::runtime_error("unexpected type for modulo");

			if (isFmod)
			{
				Append("mod(");
				Visit(node.left);
				Append(", ");
				Visit(node.right);
				Append(")");

				return; //< prevent normal generation
			}
		}

		Visit(node.left, true);

		switch (node.op)
		{
			case Ast::BinaryType::Add:       Append(" + "); break;
			case Ast::BinaryType::Subtract:  Append(" - "); break;
			case Ast::BinaryType::Multiply:  Append(" * "); break;
			case Ast::BinaryType::Divide:    Append(" / "); break;
			case Ast::BinaryType::Modulo:    Append(" % "); break;

			case Ast::BinaryType::CompEq:    Append(" == "); break;
			case Ast::BinaryType::CompGe:    Append(" >= "); break;
			case Ast::BinaryType::CompGt:    Append(" > ");  break;
			case Ast::BinaryType::CompLe:    Append(" <= "); break;
			case Ast::BinaryType::CompLt:    Append(" < ");  break;
			case Ast::BinaryType::CompNe:    Append(" != "); break;

			case Ast::BinaryType::LogicalAnd: Append(" && "); break;
			case Ast::BinaryType::LogicalOr:  Append(" || "); break;
		}

		Visit(node.right, true);
	}

	void GlslWriter::Visit(Ast::CallFunctionExpression& node)
	{
		node.targetFunction->Visit(*this);

		Append("(");
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			if (i != 0)
				Append(", ");

			node.parameters[i]->Visit(*this);
		}
		Append(")");
	}

	void GlslWriter::Visit(Ast::CastExpression& node)
	{
		Append(node.targetType);
		Append("(");

		bool first = true;
		for (const auto& exprPtr : node.expressions)
		{
			if (!first)
				Append(", ");

			first = false;

			exprPtr->Visit(*this);
		}

		Append(")");
	}

	void GlslWriter::Visit(Ast::ConstantExpression& node)
	{
		const std::string& constName = Nz::Retrieve(m_currentState->constantNames, node.constantId);
		Append(constName);
	}

	void GlslWriter::Visit(Ast::ConstantArrayValueExpression& node)
	{
		AppendArray(*node.cachedExpressionType);
		m_currentState->indentLevel++;
		AppendLine("(");
		std::visit([&](auto&& vec)
		{
			using T = std::decay_t<decltype(vec)>;
			
			if constexpr (std::is_same_v<T, Ast::NoValue>)
				throw std::runtime_error("unexpected array of NoValue");
			else
			{
				for (std::size_t i = 0; i < vec.size(); ++i)
				{
					if (i != 0)
						AppendLine(",");

					AppendValue(vec[i]);
				}
			}
		}, node.values);
		m_currentState->indentLevel--;
		AppendLine();
		Append(")");
	}

	void GlslWriter::Visit(Ast::ConstantValueExpression& node)
	{
		std::visit([&](auto&& arg)
		{
			AppendValue(arg);
		}, node.value);
	}

	void GlslWriter::Visit(Ast::FunctionExpression& node)
	{
		const auto& funcData = Nz::Retrieve(m_currentState->previsitor.functions, node.funcId);
		Append(funcData.name);
	}
	
	void GlslWriter::Visit(Ast::IntrinsicExpression& node)
	{
		bool cast = false;
		bool method = false;
		switch (node.intrinsic)
		{
			case Ast::IntrinsicType::ArraySize:
				assert(!node.parameters.empty());
				// array.length() outputs int in GLSL, but NZSL expects uint
				Append("uint(");
				Visit(node.parameters.front(), true);
				Append(".length");
				cast = true;
				method = true;
				break;

			case Ast::IntrinsicType::CrossProduct:
				Append("cross");
				break;

			case Ast::IntrinsicType::DotProduct:
				Append("dot");
				break;

			case Ast::IntrinsicType::Exp:
				Append("exp");
				break;

			case Ast::IntrinsicType::Inverse:
				Append("inverse");
				break;

			case Ast::IntrinsicType::Length:
				Append("length");
				break;

			case Ast::IntrinsicType::Max:
				Append("max");
				break;

			case Ast::IntrinsicType::Min:
				Append("min");
				break;

			case Ast::IntrinsicType::Normalize:
				Append("normalize");
				break;

			case Ast::IntrinsicType::Pow:
				Append("pow");
				break;

			case Ast::IntrinsicType::Reflect:
				Append("reflect");
				break;

			case Ast::IntrinsicType::SampleTexture:
				Append("texture");
				break;

			case Ast::IntrinsicType::Transpose:
				Append("transpose");
				break;

		}

		Append("(");
		bool first = true;
		for (std::size_t i = (method) ? 1 : 0; i < node.parameters.size(); ++i)
		{
			if (!first)
				Append(", ");

			first = false;

			node.parameters[i]->Visit(*this);
		}
		Append(")");
		if (cast)
			Append(")");
	}

	void GlslWriter::Visit(Ast::SwizzleExpression& node)
	{
		Visit(node.expression, true);
		Append(".");

		const char* componentStr = "xyzw";
		for (std::size_t i = 0; i < node.componentCount; ++i)
			Append(componentStr[node.components[i]]);
	}

	void GlslWriter::Visit(Ast::UnaryExpression& node)
	{
		switch (node.op)
		{
			case Ast::UnaryType::LogicalNot:
				Append("!");
				break;

			case Ast::UnaryType::Minus:
				Append("-");
				break;

			case Ast::UnaryType::Plus:
				Append("+");
				break;
		}

		Visit(node.expression);
	}

	void GlslWriter::Visit(Ast::VariableValueExpression& node)
	{
		const std::string& varName = Nz::Retrieve(m_currentState->variableNames, node.variableId);
		Append(varName);
	}


	void GlslWriter::Visit(Ast::BranchStatement& node)
	{
		assert(!node.isConst);

		bool first = true;
		for (const auto& statement : node.condStatements)
		{
			if (!first)
				Append("else ");

			Append("if (");
			statement.condition->Visit(*this);
			AppendLine(")");

			ScopeVisit(*statement.statement);

			first = false;
		}

		if (node.elseStatement)
		{
			AppendLine("else");

			ScopeVisit(*node.elseStatement);
		}
	}
	
	void GlslWriter::Visit(Ast::BreakStatement& /*node*/)
	{
		Append("break;");
	}

	void GlslWriter::Visit(Ast::ContinueStatement& /*node*/)
	{
		Append("continue;");
	}

	void GlslWriter::Visit(Ast::DeclareAliasStatement& /*node*/)
	{
		// all aliases should have been handled by sanitizer
		throw std::runtime_error("unexpected alias declaration, is shader sanitized?");
	}

	void GlslWriter::Visit(Ast::DeclareConstStatement& node)
	{
		assert(node.constIndex);
		RegisterConstant(*node.constIndex, node.name);

		AppendVariableDeclaration(node.type.GetResultingValue(), node.name);
		
		Append(" = ");
		node.expression->Visit(*this);
		Append(";");
	}

	void GlslWriter::Visit(Ast::DeclareExternalStatement& node)
	{
		for (const auto& externalVar : node.externalVars)
		{
			const Ast::ExpressionType& exprType = externalVar.type.GetResultingValue();
			
			bool isUniformOrStorage = IsStorageType(exprType) || IsUniformType(exprType);

			bool isStd140 = false;
			if (isUniformOrStorage)
			{
				std::size_t structIndex;
				if (IsStorageType(exprType))
					structIndex = std::get<Ast::StorageType>(exprType).containedType.structIndex;
				else if (IsUniformType(exprType))
					structIndex = std::get<Ast::UniformType>(exprType).containedType.structIndex;
				else
					throw std::runtime_error("unexpected type");
				
				const auto& structInfo = Nz::Retrieve(m_currentState->structs, structIndex);
				if (structInfo.desc->layout.HasValue())
					isStd140 = structInfo.desc->layout.GetResultingValue() == StructLayout::Std140;
			}

			std::string varName = SanitizeIdentifier(externalVar.name + m_currentState->moduleSuffix);

			if (!m_currentState->bindingMapping.empty() || isStd140)
				Append("layout(");

			if (!m_currentState->bindingMapping.empty())
			{
				assert(externalVar.bindingIndex.HasValue());

				std::uint64_t bindingIndex = externalVar.bindingIndex.GetResultingValue();
				std::uint64_t bindingSet;
				if (externalVar.bindingSet.HasValue())
					bindingSet = externalVar.bindingSet.GetResultingValue();
				else
					bindingSet = 0;

				auto bindingIt = m_currentState->bindingMapping.find(bindingSet << 32 | bindingIndex);
				if (bindingIt == m_currentState->bindingMapping.end())
					throw std::runtime_error("no binding found for (set=" + std::to_string(bindingSet) + ", binding=" + std::to_string(bindingIndex) + ")");

				if (!m_currentState->requiresExplicitUniformBinding)
				{
					Append("binding = ", bindingIt->second);
					if (isStd140)
						Append(", ");
				}
				else
					m_currentState->explicitUniformBlockBinding.emplace(varName, bindingIt->second);
			}

			if (isStd140)
				Append("std140");

			if (!m_currentState->bindingMapping.empty() || isStd140)
				Append(") ");

			if (IsStorageType(exprType))
				Append("buffer ");
			else
				Append("uniform ");

			if (isUniformOrStorage)
			{
				Append("_nzslBinding_");
				AppendLine(varName);

				EnterScope();
				{
					std::size_t structIndex = (IsStorageType(exprType)) ? std::get<Ast::StorageType>(exprType).containedType.structIndex : std::get<Ast::UniformType>(exprType).containedType.structIndex;
					const auto& structData = Nz::Retrieve(m_currentState->structs, structIndex);

					bool first = true;
					for (const auto& member : structData.desc->members)
					{
						if (member.cond.HasValue() && !member.cond.GetResultingValue())
							continue;

						if (!first)
							AppendLine();

						first = false;

						AppendVariableDeclaration(member.type.GetResultingValue(), member.name);
						Append(";");
					}
				}
				LeaveScope(false);

				Append(" ");
				Append(varName);
			}
			else
				AppendVariableDeclaration(externalVar.type.GetResultingValue(), varName);

			AppendLine(";");

			if (isUniformOrStorage)
				AppendLine();

			assert(externalVar.varIndex);
			RegisterVariable(*externalVar.varIndex, varName);
		}
	}

	void GlslWriter::Visit(Ast::DeclareFunctionStatement& node)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (node.entryStage.HasValue() && m_currentState->previsitor.entryPoint != &node)
			return; //< Ignore other entry points

		assert(node.funcIndex);
		auto& funcData = Nz::Retrieve(m_currentState->previsitor.functions, node.funcIndex.value());

		// Declare functions called by this function which aren't already defined
		bool hasPredeclaration = false;
		for (std::size_t i = funcData.calledFunctions.FindFirst(); i != funcData.calledFunctions.npos; i = funcData.calledFunctions.FindNext(i))
		{
			if (!m_currentState->declaredFunctions.UnboundedTest(i))
			{
				hasPredeclaration = true;

				auto& targetFunc = Nz::Retrieve(m_currentState->previsitor.functions, i);
				AppendFunctionDeclaration(*targetFunc.node, targetFunc.name, true);

				m_currentState->declaredFunctions.UnboundedSet(i);
			}
		}

		if (hasPredeclaration)
			AppendLine();

		if (node.entryStage.HasValue())
			return HandleEntryPoint(node);

		for (const auto& parameter : node.parameters)
		{
			assert(parameter.varIndex);
			RegisterVariable(*parameter.varIndex, SanitizeIdentifier(parameter.name));
		}

		AppendFunctionDeclaration(node, funcData.name);
		EnterScope();
		{
			AppendStatementList(node.statements);
		}
		LeaveScope();

		m_currentState->declaredFunctions.UnboundedSet(node.funcIndex.value());
	}

	void GlslWriter::Visit(Ast::DeclareOptionStatement& /*node*/)
	{
		// all options should have been handled by sanitizer
		throw std::runtime_error("unexpected option declaration, is shader sanitized?");
	}

	void GlslWriter::Visit(Ast::DeclareStructStatement& node)
	{
		std::string structName = SanitizeIdentifier(node.description.name + m_currentState->moduleSuffix);

		assert(node.structIndex);
		RegisterStruct(*node.structIndex, &node.description, structName);

		// Don't output structs used for UBO/SSBO description
		if (m_currentState->previsitor.bufferStructs.UnboundedTest(*node.structIndex))
		{
			AppendComment("struct " + structName + " omitted (used as UBO/SSBO)");
			return;
		}

		Append("struct ");
		AppendLine(structName);
		EnterScope();
		{
			bool first = true;
			for (const auto& member : node.description.members)
			{
				if (member.cond.HasValue() && !member.cond.GetResultingValue())
					continue;

				if (!first)
					AppendLine();

				first = false;

				AppendVariableDeclaration(member.type.GetResultingValue(), member.name);
				Append(";");
			}

			// Empty structs are not allowed in GLSL
			if (first)
				AppendLine("int dummy;");
		}
		LeaveScope(false);
		AppendLine(";");
	}

	void GlslWriter::Visit(Ast::DeclareVariableStatement& node)
	{
		std::string varName = SanitizeIdentifier(node.varName);

		assert(node.varIndex);
		RegisterVariable(*node.varIndex, varName);

		AppendVariableDeclaration(node.varType.GetResultingValue(), varName);
		if (node.initialExpression)
		{
			Append(" = ");
			node.initialExpression->Visit(*this);
		}

		Append(";");
	}

	void GlslWriter::Visit(Ast::DiscardStatement& /*node*/)
	{
		Append("discard;");
	}

	void GlslWriter::Visit(Ast::ExpressionStatement& node)
	{
		node.expression->Visit(*this);
		Append(";");
	}

	void GlslWriter::Visit(Ast::ImportStatement& /*node*/)
	{
		throw std::runtime_error("unexpected import statement, is the shader sanitized properly?");
	}

	void GlslWriter::Visit(Ast::MultiStatement& node)
	{
		AppendStatementList(node.statements);
	}

	void GlslWriter::Visit(Ast::NoOpStatement& /*node*/)
	{
		/* nothing to do */
	}

	void GlslWriter::Visit(Ast::ReturnStatement& node)
	{
		if (m_currentState->isInEntryPoint)
		{
			assert(node.returnExpr);

			const Ast::ExpressionType* returnType = GetExpressionType(*node.returnExpr);
			assert(returnType);
			assert(IsStructType(*returnType));
			std::size_t structIndex = std::get<Ast::StructType>(*returnType).structIndex;
			const auto& structData = Nz::Retrieve(m_currentState->structs, structIndex);

			std::string outputStructVarName;
			if (node.returnExpr->GetType() == Ast::NodeType::VariableValueExpression)
				outputStructVarName = Nz::Retrieve(m_currentState->variableNames, static_cast<Ast::VariableValueExpression&>(*node.returnExpr).variableId);
			else
			{
				AppendLine();
				Append(structData.nameOverride, " ", s_glslWriterOutputVarName, " = ");
				node.returnExpr->Visit(*this);
				AppendLine(";");

				outputStructVarName = s_glslWriterOutputVarName;
			}

			AppendLine();

			for (const auto& [name, target] : m_currentState->outputFields)
			{
				bool isOutputPosition = (m_currentState->stage == ShaderStageType::Vertex && std::holds_alternative<Ast::BuiltinEntry>(target) && std::get<Ast::BuiltinEntry>(target) == Ast::BuiltinEntry::VertexPosition);

				AppendLine(target, " = ", outputStructVarName, ".", name, ";");
				if (isOutputPosition)
				{
					// https://veldrid.dev/articles/backend-differences.html
					if (m_environment.flipYPosition)
						AppendLine(target, ".y *= ", s_glslWriterFlipYUniformName, ";");

					if (m_environment.remapZPosition)
						AppendLine(target, ".z = ", target, ".z * 2.0 - ", target, ".w;");
				}
			}

			Append("return;"); //< TODO: Don't return if it's the last statement of the function
		}
		else
		{
			if (node.returnExpr)
			{
				Append("return ");
				node.returnExpr->Visit(*this);
				Append(";");
			}
			else
				Append("return;");
		}
	}

	void GlslWriter::Visit(Ast::ScopedStatement& node)
	{
		EnterScope();
		node.statement->Visit(*this);
		LeaveScope(true);
	}

	void GlslWriter::Visit(Ast::WhileStatement& node)
	{
		Append("while (");
		node.condition->Visit(*this);
		AppendLine(")");

		ScopeVisit(*node.body);
	}
}
