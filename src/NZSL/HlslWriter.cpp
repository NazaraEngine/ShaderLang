// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/HlslWriter.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Lang/Version.hpp>
#include <NZSL/Ast/Transformations/AliasTransformer.hpp>
#include <NZSL/Ast/Transformations/BindingResolverTransformer.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <NZSL/Ast/Transformations/ConstantRemovalTransformer.hpp>
#include <NZSL/Ast/Transformations/EliminateUnusedTransformer.hpp>
#include <NZSL/Ast/Transformations/ForToWhileTransformer.hpp>
#include <NZSL/Ast/Transformations/IdentifierTransformer.hpp>
#include <NZSL/Ast/Transformations/LiteralTransformer.hpp>
#include <NZSL/Ast/Transformations/LoopUnrollTransformer.hpp>
#include <NZSL/Ast/Transformations/ResolveTransformer.hpp>
#include <NZSL/Ast/Transformations/StructAssignmentTransformer.hpp>
#include <NZSL/Ast/Transformations/SwizzleTransformer.hpp>
#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <fmt/format.h>
#include <frozen/unordered_map.h>
#include <frozen/unordered_set.h>
#include <tsl/ordered_set.h>
#include <cassert>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace nzsl
{
	namespace
	{
		constexpr auto s_hlslBuiltinMapping = frozen::make_unordered_map<Ast::BuiltinEntry, std::string_view>({
			// BaseInstance and BaseVertex have no HLSL equivalent (no SV_ semantic exists)
			// They must be provided via a cbuffer by the application
			{ Ast::BuiltinEntry::DrawIndex,               "SV_DrawID" },
			{ Ast::BuiltinEntry::FragCoord,               "SV_Position" },
			{ Ast::BuiltinEntry::FragDepth,               "SV_Depth" },
			{ Ast::BuiltinEntry::GlocalInvocationIndices, "SV_DispatchThreadID" },
			{ Ast::BuiltinEntry::InstanceIndex,           "SV_InstanceID" },
			{ Ast::BuiltinEntry::LocalInvocationIndex,    "SV_GroupIndex" },
			{ Ast::BuiltinEntry::LocalInvocationIndices,  "SV_GroupThreadID" },
			{ Ast::BuiltinEntry::VertexIndex,             "SV_VertexID" },
			{ Ast::BuiltinEntry::VertexPosition,          "SV_Position" },
			// WorkgroupCount requires SM6.6+, handled separately in AppendSemantic
			{ Ast::BuiltinEntry::WorkgroupIndices,        "SV_GroupID" }
		});

		struct HlslWriterPreVisitor : Ast::RecursiveVisitor
		{
			void RegisterStructType(const Ast::ExpressionType& type)
			{
				if (IsStorageType(type))
					usedStructs.UnboundedSet(std::get<Ast::StorageType>(type).containedType.structIndex);
				else if (IsUniformType(type))
					usedStructs.UnboundedSet(std::get<Ast::UniformType>(type).containedType.structIndex);
				else if (IsStructType(type))
					usedStructs.UnboundedSet(std::get<Ast::StructType>(type).structIndex);
				else if (IsArrayType(type))
					RegisterStructType(std::get<Ast::ArrayType>(type).InnerType());
				else if (IsDynArrayType(type))
					RegisterStructType(std::get<Ast::DynArrayType>(type).InnerType());
			}

			void Resolve()
			{
				usedStructs.Resize(bufferStructs.GetSize());
				usedStructs.PerformsNOT(usedStructs);
				bufferStructs &= usedStructs;
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
						std::size_t structIndex = std::get<Ast::StorageType>(type).containedType.structIndex;
						bufferStructs.UnboundedSet(structIndex);
					}
					else if (IsUniformType(type))
					{
						std::size_t structIndex = std::get<Ast::UniformType>(type).containedType.structIndex;
						bufferStructs.UnboundedSet(structIndex);
					}
				}

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::DeclareFunctionStatement& node) override
			{
				if (node.entryStage.HasValue())
				{
					ShaderStageType stage = node.entryStage.GetResultingValue();

					if (selectedStage)
					{
						if (stage != *selectedStage)
							return;

						assert(!entryPoint);
						entryPoint = &node;
					}
					else
					{
						if (!entryPoint)
							entryPoint = &node;

						entryPoints.push_back(&node);
					}

					// Track output struct of fragment shader for SV_Target semantics
					if (stage == ShaderStageType::Fragment && node.returnType.HasValue() && !IsNoType(node.returnType.GetResultingValue()))
					{
						if (std::holds_alternative<Ast::StructType>(node.returnType.GetResultingValue()))
							fragmentOutputStructIndex = std::get<Ast::StructType>(node.returnType.GetResultingValue()).structIndex;
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
					RegisterStructType(member.type.GetResultingValue());

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::DeclareVariableStatement& node) override
			{
				RegisterStructType(node.varType.GetResultingValue());

				RecursiveVisitor::Visit(node);
			}

			void Visit(Ast::IntrinsicExpression& node) override
			{
				RecursiveVisitor::Visit(node);

				if (node.intrinsic == Ast::IntrinsicType::MatrixInverse)
				{
					assert(!node.parameters.empty());
					const Ast::ExpressionType* argType = GetExpressionType(*node.parameters.front());
					if (argType && std::holds_alternative<Ast::MatrixType>(*argType))
					{
						const Ast::MatrixType& matType = std::get<Ast::MatrixType>(*argType);
						if (std::find(requiredInverseMatrixTypes.begin(), requiredInverseMatrixTypes.end(), matType) == requiredInverseMatrixTypes.end())
							requiredInverseMatrixTypes.push_back(matType);
					}
				}
			}

			struct FunctionData
			{
				std::string name;
				Nz::Bitset<> calledFunctions;
				Ast::DeclareFunctionStatement* node;
			};

			FunctionData* currentFunction = nullptr;

			std::optional<ShaderStageType> selectedStage;
			std::string moduleSuffix;
			std::unordered_map<std::size_t, FunctionData> functions;
			std::unordered_map<std::size_t, Ast::StructDescription*> structs;
			std::optional<std::size_t> fragmentOutputStructIndex;
			Nz::Bitset<> bufferStructs;
			Nz::Bitset<> usedStructs;
			Ast::DeclareFunctionStatement* entryPoint = nullptr;
			std::vector<Ast::DeclareFunctionStatement*> entryPoints;
			std::vector<Ast::MatrixType> requiredInverseMatrixTypes;
		};
	}


	struct HlslWriter::State
	{
		State(const BackendParameters& backendParameters, const HlslWriter::Parameters& hlslParameters) :
		backendParameters(backendParameters),
		hlslParameters(hlslParameters)
		{
		}

		struct StructData
		{
			std::string nameOverride;
			const Ast::StructDescription* desc;
		};

		std::string moduleSuffix;
		std::stringstream stream;
		std::unordered_map<std::size_t, std::string> constantNames;
		std::unordered_map<std::size_t, StructData> structs;
		std::unordered_map<std::size_t, std::string> variableNames;
		std::unordered_set<std::string> reservedNames;
		std::unordered_set<std::size_t> unwrappedStorageBufferStructs; // struct indices whose dyn_array member access should be skipped
		Nz::Bitset<> declaredFunctions;
		const BackendParameters& backendParameters;
		const HlslWriter::Parameters& hlslParameters;
		HlslWriterPreVisitor previsitor;
		ShaderStageType stage;
		bool isInEntryPoint = false;
		int streamEmptyLine = 1;
		unsigned int indentLevel = 0;
	};

	auto HlslWriter::Generate(std::optional<ShaderStageType> shaderStage, Ast::Module& module, const BackendParameters& parameters, const Parameters& hlslParameters) -> HlslWriter::Output
	{
		State state(parameters, hlslParameters);

		m_currentState = &state;
		NAZARA_DEFER({ m_currentState = nullptr; });

		if (parameters.backendPasses)
		{
			Ast::TransformerExecutor executor;
			if (parameters.backendPasses.Test(BackendPass::Resolve))
			{
				executor.AddPass<Ast::ResolveTransformer>([&](Ast::ResolveTransformer::Options& opt)
				{
					opt.moduleResolver = parameters.shaderModuleResolver;
				});
			}

			if (parameters.backendPasses.Test(BackendPass::TargetRequired))
				RegisterPasses(executor);

			if (parameters.backendPasses.Test(BackendPass::Optimize))
				executor.AddPass<Ast::ConstantPropagationTransformer>();

			if (parameters.backendPasses.Test(BackendPass::Validate))
			{
				executor.AddPass<Ast::ValidationTransformer>([](Ast::ValidationTransformer::Options& opt)
				{
					opt.allowUntyped = false;
					opt.checkIndices = true;
				});
			}

			Ast::TransformerContext context;
			context.optionValues = parameters.optionValues;

			executor.Transform(module, context);
		}

		if (parameters.backendPasses.Test(BackendPass::RemoveDeadCode))
		{
			Ast::DependencyCheckerVisitor::Config dependencyConfig;
			dependencyConfig.usedShaderStages = (shaderStage) ? *shaderStage : ShaderStageType_All;

			Ast::EliminateUnusedPass(module, dependencyConfig);
		}

		// Previsitor
		state.previsitor.selectedStage = shaderStage;

		for (const auto& importedModule : module.importedModules)
		{
			state.previsitor.moduleSuffix = importedModule.identifier;
			importedModule.module->rootNode->Visit(state.previsitor);
		}

		state.previsitor.moduleSuffix = {};
		module.rootNode->Visit(state.previsitor);

		state.previsitor.Resolve();

		if (!state.previsitor.entryPoint)
			throw std::runtime_error("no entry point found");

		if (state.previsitor.entryPoints.empty())
		{
			assert(state.previsitor.entryPoint->entryStage.HasValue());
			m_currentState->stage = state.previsitor.entryPoint->entryStage.GetResultingValue();
		}

		// Code generation
		AppendHeader();
		AppendHelperFunctions();

		for (const auto& importedModule : module.importedModules)
		{
			if (m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
			{
				AppendComment("Module " + importedModule.module->metadata->moduleName);
				AppendModuleComments(*importedModule.module);
				AppendLine();
			}

			m_currentState->moduleSuffix = importedModule.identifier;
			importedModule.module->rootNode->Visit(*this);
		}

		if (m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
		{
			if (!module.importedModules.empty())
				AppendComment("Main module");

			AppendModuleComments(module);
			AppendLine();
		}

		m_currentState->moduleSuffix = {};
		module.rootNode->Visit(*this);

		Output output;
		output.code = std::move(state.stream).str();

		return output;
	}

	void HlslWriter::SetEnv(Environment environment)
	{
		m_environment = std::move(environment);
	}

	void HlslWriter::RegisterPasses(Ast::TransformerExecutor& executor)
	{
		static constexpr auto s_reservedKeywords = frozen::make_unordered_set<frozen::string>({
			// HLSL reserved keywords
			"AppendStructuredBuffer", "BlendState", "Buffer", "ByteAddressBuffer", "ConsumeStructuredBuffer",
			"DepthStencilState", "DepthStencilView",
			"RWBuffer", "RWByteAddressBuffer", "RWStructuredBuffer", "RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D",
			"RasterizerState", "RenderTargetView",
			"SamplerComparisonState", "SamplerState", "StructuredBuffer",
			"Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS", "Texture2DMSArray", "Texture3D", "TextureCube", "TextureCubeArray",
			"bool", "bool1", "bool2", "bool3", "bool4",
			"break", "case", "cbuffer", "centroid", "class", "column_major", "compile", "const", "continue",
			"default", "discard", "do", "double", "double1", "double2", "double3", "double4",
			"dword", "dword1", "dword2", "dword3", "dword4",
			"else", "export", "extern",
			"false", "float", "float1", "float2", "float3", "float4",
			"float1x1", "float1x2", "float1x3", "float1x4",
			"float2x1", "float2x2", "float2x3", "float2x4",
			"float3x1", "float3x2", "float3x3", "float3x4",
			"float4x1", "float4x2", "float4x3", "float4x4",
			"for", "groupshared",
			"half", "half1", "half2", "half3", "half4",
			"if", "in", "inline", "inout", "int", "int1", "int2", "int3", "int4", "interface",
			"linear", "matrix", "min10float", "min16float", "min12int", "min16int", "min16uint",
			"namespace", "nointerpolation", "noperspective", "NULL",
			"out",
			"packoffset", "pass", "point", "precise",
			"register", "return", "row_major",
			"sample", "sampler", "shared", "snorm", "stateblock", "stateblock_state",
			"static", "string", "struct", "switch",
			"tbuffer", "technique", "texture", "true", "typedef", "triangle", "triangleadj",
			"uint", "uint1", "uint2", "uint3", "uint4", "uniform", "unorm", "unsigned",
			"vector", "vertexshader", "void", "volatile",
			"while",
			// HLSL intrinsic functions
			"abs", "acos", "all", "any", "asin", "atan", "atan2", "ceil", "clamp", "clip", "cos", "cosh", "cross",
			"ddx", "ddx_coarse", "ddx_fine", "ddy", "ddy_coarse", "ddy_fine", "degrees", "determinant", "distance", "dot",
			"exp", "exp2",
			"floor", "fmod", "frac", "frexp", "fwidth",
			"isfinite", "isinf", "isnan",
			"ldexp", "length", "lerp", "lit", "log", "log10", "log2",
			"mad", "max", "min", "modf", "mul",
			"normalize",
			"pow",
			"radians", "rcp", "reflect", "refract", "round", "rsqrt",
			"saturate", "sign", "sin", "sincos", "sinh", "smoothstep", "sqrt", "step",
			"tan", "tanh", "transpose", "trunc",
		});

		Ast::IdentifierTransformer::Options firstIdentifierPassOptions;
		firstIdentifierPassOptions.makeVariableNameUnique = false;
		firstIdentifierPassOptions.identifierSanitizer = [](std::string& identifier, Ast::IdentifierCategory /*category*/)
		{
			using namespace std::string_view_literals;

			if (identifier.compare(0, 5, "_nzsl") == 0)
			{
				identifier.replace(0, 5, "_"sv);
				return true;
			}

			return false;
		};

		Ast::IdentifierTransformer::Options secondIdentifierPassOptions;
		secondIdentifierPassOptions.makeVariableNameUnique = true;
		secondIdentifierPassOptions.identifierSanitizer = [](std::string& identifier, Ast::IdentifierCategory /*category*/)
		{
			bool nameChanged = false;
			while (s_reservedKeywords.count(frozen::string(identifier)) != 0)
			{
				identifier += '_';
				nameChanged = true;
			}

			return nameChanged;
		};

		executor.AddPass<Ast::LoopUnrollTransformer>();
		executor.AddPass<Ast::LiteralTransformer>();
		executor.AddPass<Ast::IdentifierTransformer>(firstIdentifierPassOptions);
		executor.AddPass<Ast::ForToWhileTransformer>();
		executor.AddPass<Ast::StructAssignmentTransformer>([](Ast::StructAssignmentTransformer::Options& opt)
		{
			opt.splitWrappedArrayAssignation = false;
			opt.splitWrappedStructAssignation = true;
		});
		executor.AddPass<Ast::SwizzleTransformer>([](Ast::SwizzleTransformer::Options& opt)
		{
			opt.removeScalarSwizzling = true;
		});
		executor.AddPass<Ast::BindingResolverTransformer>();
		executor.AddPass<Ast::ConstantRemovalTransformer>([](Ast::ConstantRemovalTransformer::Options& opt)
		{
			opt.removeConstArraySize = false;
			opt.removeTypeConstant = false;
		});
		executor.AddPass<Ast::AliasTransformer>();
		executor.AddPass<Ast::IdentifierTransformer>(secondIdentifierPassOptions);
	}

	void HlslWriter::Append(const Ast::AliasType& /*aliasType*/)
	{
		throw std::runtime_error("unexpected AliasType");
	}

	void HlslWriter::Append(const Ast::ArrayType& type)
	{
		AppendArray(type);
	}

	void HlslWriter::Append(Ast::BuiltinEntry builtin)
	{
		auto it = s_hlslBuiltinMapping.find(builtin);
		if (it == s_hlslBuiltinMapping.end())
			throw std::runtime_error("builtin has no HLSL equivalent (BaseInstance/BaseVertex are not supported in HLSL, they must be provided via a cbuffer)");

		Append(it->second);
	}

	void HlslWriter::Append(const Ast::DynArrayType& type)
	{
		AppendArray(type);
	}

	void HlslWriter::Append(const Ast::ExpressionType& type)
	{
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, type);
	}

	void HlslWriter::Append(const Ast::ExpressionValue<Ast::ExpressionType>& type)
	{
		Append(type.GetResultingValue());
	}

	void HlslWriter::Append(const Ast::FunctionType& /*functionType*/)
	{
		throw std::runtime_error("unexpected FunctionType");
	}

	void HlslWriter::Append(const Ast::ImplicitArrayType& /*type*/)
	{
		throw std::runtime_error("unexpected ImplicitArrayType");
	}

	void HlslWriter::Append(const Ast::ImplicitMatrixType& /*type*/)
	{
		throw std::runtime_error("unexpected ImplicitMatrixType");
	}

	void HlslWriter::Append(const Ast::ImplicitVectorType& /*type*/)
	{
		throw std::runtime_error("unexpected ImplicitVectorType");
	}

	void HlslWriter::Append(Ast::InterpolationQualifier interpolation)
	{
		switch (interpolation)
		{
			case Ast::InterpolationQualifier::Flat:          return Append("nointerpolation");
			case Ast::InterpolationQualifier::NoPerspective: return Append("noperspective");
			case Ast::InterpolationQualifier::Smooth:        return Append("linear");
		}
	}

	void HlslWriter::Append(const Ast::IntrinsicFunctionType& /*intrinsicFunctionType*/)
	{
		throw std::runtime_error("unexpected intrinsic function type");
	}

	void HlslWriter::Append(const Ast::MatrixType& matrixType)
	{
		switch (matrixType.type)
		{
			case Ast::PrimitiveType::Float32: Append("float"); break;
			case Ast::PrimitiveType::Float64: Append("double"); break;
			default: throw std::runtime_error("unexpected matrix base type");
		}

		Append(matrixType.columnCount);
		Append("x");
		Append(matrixType.rowCount);
	}

	void HlslWriter::Append(const Ast::MethodType& /*methodType*/)
	{
		throw std::runtime_error("unexpected method type");
	}

	void HlslWriter::Append(const Ast::ModuleType& /*moduleType*/)
	{
		throw std::runtime_error("unexpected module type");
	}

	void HlslWriter::Append(Ast::MemoryLayout /*layout*/)
	{
		// HLSL doesn't have explicit memory layout qualifiers
	}

	void HlslWriter::Append(const Ast::NamedExternalBlockType& /*namedExternalBlockType*/)
	{
		throw std::runtime_error("unexpected named external block type");
	}

	void HlslWriter::Append(Ast::NoType)
	{
		return Append("void");
	}

	void HlslWriter::Append(Ast::PrimitiveType type)
	{
		switch (type)
		{
			case Ast::PrimitiveType::Boolean: return Append("bool");
			case Ast::PrimitiveType::Float32: return Append("float");
			case Ast::PrimitiveType::Float64: return Append("double");
			case Ast::PrimitiveType::Int32:   return Append("int");
			case Ast::PrimitiveType::UInt32:  return Append("uint");

			case Ast::PrimitiveType::FloatLiteral: throw std::runtime_error("unexpected untyped float");
			case Ast::PrimitiveType::IntLiteral:   throw std::runtime_error("unexpected untyped integer");
			case Ast::PrimitiveType::String:       throw std::runtime_error("unexpected string type");
		}
	}

	void HlslWriter::Append(const Ast::PushConstantType& pushConstantType)
	{
		Append(pushConstantType.containedType);
	}

	void HlslWriter::Append(const Ast::SamplerType& samplerType)
	{
		// In HLSL, combined samplers are emitted as Texture2D etc. (the sampler state is separate)
		// But when used as a type declaration, we emit the texture type
		switch (samplerType.sampledType)
		{
			case Ast::PrimitiveType::Float32: break;
			case Ast::PrimitiveType::Int32:   break;
			case Ast::PrimitiveType::UInt32:  break;
			default: throw std::runtime_error("unexpected type for sampler");
		}

		if (samplerType.depth)
		{
			// Depth samplers use Texture2D etc. + SamplerComparisonState
			Append("Texture");
		}
		else
		{
			Append("Texture");
		}

		switch (samplerType.dim)
		{
			case ImageType::E1D:       Append("1D"); break;
			case ImageType::E1D_Array: Append("1DArray"); break;
			case ImageType::E2D:       Append("2D"); break;
			case ImageType::E2D_Array: Append("2DArray"); break;
			case ImageType::E3D:       Append("3D"); break;
			case ImageType::Cubemap:   Append("Cube"); break;
		}

		// Add template type
		Append("<");
		switch (samplerType.sampledType)
		{
			case Ast::PrimitiveType::Float32: Append("float4"); break;
			case Ast::PrimitiveType::Int32:   Append("int4"); break;
			case Ast::PrimitiveType::UInt32:  Append("uint4"); break;
			default: break;
		}
		Append(">");
	}

	void HlslWriter::Append(const Ast::StorageType& /*storageType*/)
	{
		throw std::runtime_error("unexpected StorageType");
	}

	void HlslWriter::Append(const Ast::StructType& structType)
	{
		const auto& structData = Nz::Retrieve(m_currentState->structs, structType.structIndex);
		Append(structData.nameOverride);
	}

	void HlslWriter::Append(const Ast::TextureType& textureType)
	{
		bool isReadWrite = (textureType.accessPolicy == AccessPolicy::ReadWrite || textureType.accessPolicy == AccessPolicy::WriteOnly);
		if (isReadWrite)
			Append("RW");

		Append("Texture");

		switch (textureType.dim)
		{
			case ImageType::E1D:       Append("1D"); break;
			case ImageType::E1D_Array: Append("1DArray"); break;
			case ImageType::E2D:       Append("2D"); break;
			case ImageType::E2D_Array: Append("2DArray"); break;
			case ImageType::E3D:       Append("3D"); break;
			case ImageType::Cubemap:   Append("Cube"); break;
		}

		Append("<");
		switch (textureType.baseType)
		{
			case Ast::PrimitiveType::Float32: Append("float4"); break;
			case Ast::PrimitiveType::Int32:   Append("int4"); break;
			case Ast::PrimitiveType::UInt32:  Append("uint4"); break;
			default: throw std::runtime_error("unexpected texture base type");
		}
		Append(">");
	}

	void HlslWriter::Append(const Ast::Type& /*type*/)
	{
		throw std::runtime_error("unexpected Type");
	}

	void HlslWriter::Append(const Ast::UniformType& /*uniformType*/)
	{
		throw std::runtime_error("unexpected UniformType");
	}

	void HlslWriter::Append(const Ast::VectorType& vecType)
	{
		switch (vecType.type)
		{
			case Ast::PrimitiveType::Boolean: Append("bool"); break;
			case Ast::PrimitiveType::Float32: Append("float"); break;
			case Ast::PrimitiveType::Float64: Append("double"); break;
			case Ast::PrimitiveType::Int32:   Append("int"); break;
			case Ast::PrimitiveType::UInt32:  Append("uint"); break;

			case Ast::PrimitiveType::FloatLiteral: throw std::runtime_error("unexpected FloatLiteral type");
			case Ast::PrimitiveType::IntLiteral:   throw std::runtime_error("unexpected IntLiteral type");
			case Ast::PrimitiveType::String:       throw std::runtime_error("unexpected string type");
		}

		Append(vecType.componentCount);
	}

	template<typename T>
	void HlslWriter::Append(const T& param)
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
	void HlslWriter::Append(const T1& firstParam, const T2& secondParam, Args&&... params)
	{
		Append(firstParam);
		Append(secondParam, std::forward<Args>(params)...);
	}

	template<typename... Args>
	void HlslWriter::Append(const std::variant<Args...>& param)
	{
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, param);
	}

	void HlslWriter::AppendArray(const Ast::ExpressionType& type, const std::string& varName)
	{
		std::vector<std::uint32_t> lengths;

		const Ast::ExpressionType* exprType = &type;
		for (;;)
		{
			if (Ast::IsArrayType(*exprType))
			{
				const auto& arrayType = std::get<Ast::ArrayType>(*exprType);
				lengths.push_back(arrayType.length);

				exprType = &arrayType.InnerType();
			}
			else if (Ast::IsDynArrayType(*exprType))
			{
				const auto& arrayType = std::get<Ast::DynArrayType>(*exprType);
				lengths.push_back(0);

				exprType = &arrayType.InnerType();
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

	void HlslWriter::AppendComment(std::string_view section)
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

	void HlslWriter::AppendCommentSection(std::string_view section)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		std::string stars((section.size() < 33) ? (36 - section.size()) / 2 : 3, '*');
		Append("/*", stars, ' ', section, ' ', stars, "*/");
		AppendLine();
	}

	void HlslWriter::AppendFunctionDeclaration(const Ast::DeclareFunctionStatement& node, const std::string& nameOverride, bool forward)
	{
		Append(node.returnType, " ", nameOverride, "(");

		bool first = true;
		for (const auto& parameter : node.parameters)
		{
			if (!first)
				Append(", ");

			first = false;

			if (parameter.semantic == Ast::FunctionParameterSemantic::InOut)
				Append("inout ");
			else if (parameter.semantic == Ast::FunctionParameterSemantic::Out)
				Append("out ");

			AppendVariableDeclaration(parameter.type.GetResultingValue(), parameter.name);
		}
		AppendLine((forward) ? ");" : ")");
	}

	void HlslWriter::AppendHeader()
	{
		if (m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
		{
			std::string fileTitle;

			if (m_currentState->previsitor.entryPoints.size() > 1)
			{
				fileTitle += "shader - ";
			}
			else
			{
				switch (m_currentState->stage)
				{
					case ShaderStageType::Compute: fileTitle += "compute shader - "; break;
					case ShaderStageType::Fragment: fileTitle += "pixel shader - "; break;
					case ShaderStageType::Vertex: fileTitle += "vertex shader - "; break;
				}
			}

			fileTitle += "this file was generated by NZSL compiler (Nazara Shading Language)";

			AppendComment(fileTitle);
			AppendLine();
		}

		// HLSL uses column-major by default in NZSL, but HLSL defaults to row-major
		AppendLine("#pragma pack_matrix(column_major)");
		AppendLine();

		// Handle compute shader numthreads
		if (m_currentState->stage == ShaderStageType::Compute)
		{
			// numthreads will be emitted on the entry point function directly
		}

		if (m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
		{
			AppendLine("// header end");
			AppendLine();
		}
	}

	void HlslWriter::AppendHelperFunctions()
	{
		for (const Ast::MatrixType& matType : m_currentState->previsitor.requiredInverseMatrixTypes)
		{
			assert(matType.columnCount == matType.rowCount);
			std::string_view scalarName;
			std::string_view oneLiteral;
			switch (matType.type)
			{
				case Ast::PrimitiveType::Float32: scalarName = "float";  oneLiteral = "1.0f"; break;
				case Ast::PrimitiveType::Float64: scalarName = "double"; oneLiteral = "1.0";  break;
				default: throw std::runtime_error("unsupported scalar type for matrix inverse helper");
			}

			std::string matTypeName = fmt::format("{}{}x{}", scalarName, matType.columnCount, matType.rowCount);

			AppendCommentSection(fmt::format("Matrix inverse helper for {}", matTypeName));
			Append(matTypeName, " _nzsl_inverse(", matTypeName, " m)");
			AppendLine();
			EnterScope();

			std::size_t n = matType.columnCount;

			// M^-1 = adj(M) / det(M)
			// adj(M)[i][j] = (-1)^(i+j) * det(minor removing row j, col i)
			AppendLine(scalarName, " det = determinant(m);");
			AppendLine(scalarName, " invDet = ", oneLiteral, " / det;");
			AppendLine(matTypeName, " r;");

			for (std::size_t i = 0; i < n; i++)
			{
				for (std::size_t j = 0; j < n; j++)
				{
					std::vector<std::size_t> subRows, subCols;
					for (std::size_t k = 0; k < n; k++)
					{
						if (k != j) subRows.push_back(k);
						if (k != i) subCols.push_back(k);
					}

					bool negative = (i + j) % 2 == 1;

					if (n == 2)
					{
						// 1x1 minor is a single element
						std::string elem = fmt::format("m[{}][{}]", subRows[0], subCols[0]);
						if (negative)
							AppendLine("r[", i, "][", j, "] = -(", elem, ") * invDet;");
						else
							AppendLine("r[", i, "][", j, "] = ", elem, " * invDet;");
					}
					else
					{
						// Build (n-1)x(n-1) submatrix and use HLSL determinant()
						std::string subMatType = fmt::format("{}{}x{}", scalarName, n - 1, n - 1);
						std::string subMatExpr = subMatType + "(";
						bool first = true;
						for (std::size_t r : subRows)
						{
							for (std::size_t c : subCols)
							{
								if (!first) subMatExpr += ", ";
								subMatExpr += fmt::format("m[{}][{}]", r, c);
								first = false;
							}
						}
						subMatExpr += ")";

						if (negative)
							AppendLine("r[", i, "][", j, "] = -determinant(", subMatExpr, ") * invDet;");
						else
							AppendLine("r[", i, "][", j, "] = determinant(", subMatExpr, ") * invDet;");
					}
				}
			}

			AppendLine("return r;");

			LeaveScope();
			AppendLine();
		}
	}

	void HlslWriter::AppendLine(std::string_view txt)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (txt.empty() && m_currentState->streamEmptyLine > 1)
			return;

		m_currentState->stream << txt << '\n';
		m_currentState->streamEmptyLine++;
	}

	template<typename... Args>
	void HlslWriter::AppendLine(Args&&... params)
	{
		(Append(std::forward<Args>(params)), ...);
		AppendLine();
	}

	template<typename T>
	void HlslWriter::AppendValue(const T& value)
	{
		if constexpr (IsVector_v<T>)
		{
			if constexpr (std::is_same_v<typename T::Base, bool>)
				Append("bool");
			else if constexpr (std::is_same_v<typename T::Base, double>)
				Append("double");
			else if constexpr (std::is_same_v<typename T::Base, std::int32_t>)
				Append("int");
			else if constexpr (std::is_same_v<typename T::Base, std::uint32_t>)
				Append("uint");
			else
				Append("float");
		}

		if constexpr (std::is_same_v<T, Ast::NoValue>)
			throw std::runtime_error("invalid type (value expected)");
		else if constexpr (Ast::IsLiteral_v<T>)
			throw std::runtime_error("unexpected untyped");
		else if constexpr (std::is_same_v<T, std::string>)
			throw std::runtime_error("unexpected string literal");
		else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, std::vector<bool>::reference>)
			Append((value) ? "true" : "false");
		else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>)
		{
			Append(Ast::ToString(value));
			if constexpr (std::is_same_v<T, std::uint32_t>)
				Append("u");
			else if constexpr (std::is_same_v<T, double>)
				Append("lf");
		}
		else if constexpr (IsVector_v<T>)
		{
			Append(T::Dimensions, "(");
			for (std::size_t i = 0; i < T::Dimensions; ++i)
			{
				if (i != 0)
					Append(", ");

				AppendValue(value[i]);
			}

			Append(")");
		}
		else
			static_assert(Nz::AlwaysFalse<T>(), "non-exhaustive visitor");
	}

	void HlslWriter::AppendModuleComments(const Ast::Module& module)
	{
		const auto& metadata = *module.metadata;

		if (m_currentState->backendParameters.debugLevel >= DebugLevel::Regular)
		{
			const SourceLocation& rootLocation = module.rootNode->sourceLocation;

			AppendComment("NZSL version: " + Version::ToString(metadata.langVersion));
			if (rootLocation.file)
			{
				AppendComment("from " + *rootLocation.file);
				if (m_currentState->backendParameters.debugLevel >= DebugLevel::Full)
				{
					std::ifstream file(Nz::Utf8Path(*rootLocation.file));
					if (file)
					{
						AppendLine("/* Module source code");
						AppendLine();

						std::string line;
						while (std::getline(file, line))
							AppendLine(line);

						AppendLine();
						AppendLine("Module source code */");
						AppendLine();
					}
				}
			}
		}

		if (!metadata.author.empty())
			AppendComment("Author: " + metadata.author);

		if (!metadata.description.empty())
			AppendComment("Description: " + metadata.description);

		if (!metadata.license.empty())
			AppendComment("License: " + metadata.license);
	}

	void HlslWriter::AppendSemantic(const Ast::StructDescription::StructMember& member, bool isFragmentOutput)
	{
		if (member.builtin.HasValue())
		{
			Ast::BuiltinEntry builtin = member.builtin.GetResultingValue();

			// WorkgroupCount requires SM6.6+
			if (builtin == Ast::BuiltinEntry::WorkgroupCount)
			{
				bool sm66 = (m_environment.shaderModelMajorVersion > 6) ||
				            (m_environment.shaderModelMajorVersion == 6 && m_environment.shaderModelMinorVersion >= 6);
				if (sm66)
				{
					Append(" : SV_GroupCount");
				}
				else
					throw std::runtime_error("WorkgroupCount requires shader model 6.6+ (use --hlsl-version=66)");
			}
			// BaseInstance/BaseVertex require SM6.8+
			else if (builtin == Ast::BuiltinEntry::BaseInstance || builtin == Ast::BuiltinEntry::BaseVertex)
			{
				bool sm68 = (m_environment.shaderModelMajorVersion > 6) ||
				            (m_environment.shaderModelMajorVersion == 6 && m_environment.shaderModelMinorVersion >= 8);
				if (sm68)
				{
					Append(" : ");
					Append(builtin == Ast::BuiltinEntry::BaseInstance ? "SV_StartInstanceLocation" : "SV_StartVertexLocation");
				}
				else
					throw std::runtime_error("BaseInstance/BaseVertex require shader model 6.8+ (use --hlsl-version=68)");
			}
			else
			{
				auto it = s_hlslBuiltinMapping.find(builtin);
				if (it != s_hlslBuiltinMapping.end())
				{
					Append(" : ");
					Append(it->second);
				}
			}
		}
		else if (member.locationIndex.HasValue())
		{
			if (isFragmentOutput)
			{
				Append(" : SV_Target");
				Append(member.locationIndex.GetResultingValue());
			}
			else
			{
				Append(" : TEXCOORD");
				Append(member.locationIndex.GetResultingValue());
			}
		}
	}

	void HlslWriter::AppendStatementList(std::vector<Ast::StatementPtr>& statements)
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

	void HlslWriter::AppendVariableDeclaration(const Ast::ExpressionType& varType, const std::string& varName)
	{
		if (Ast::IsArrayType(varType) || Ast::IsDynArrayType(varType))
			AppendArray(varType, varName);
		else
			Append(varType, " ", varName);
	}

	void HlslWriter::EnterScope()
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		AppendLine("{");
		m_currentState->indentLevel++;
	}

	void HlslWriter::LeaveScope(bool skipLine)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		m_currentState->indentLevel--;
		AppendLine();

		if (skipLine)
			AppendLine("}");
		else
			Append("}");
	}

	void HlslWriter::HandleEntryPoint(Ast::DeclareFunctionStatement& node)
	{
		assert(node.entryStage.HasValue());
		m_currentState->stage = node.entryStage.GetResultingValue();

		HandleSourceLocation(node.sourceLocation, DebugLevel::Regular);

		// Get entry point name
		std::string entryPointName;
		if (m_currentState->previsitor.entryPoints.size() > 1)
		{
			assert(node.funcIndex);
			entryPointName = Nz::Retrieve(m_currentState->previsitor.functions, node.funcIndex.value()).name;
		}
		else
		{
			switch (m_currentState->stage)
			{
				case ShaderStageType::Compute:  entryPointName = "main"; break;
				case ShaderStageType::Fragment: entryPointName = "main"; break;
				case ShaderStageType::Vertex:   entryPointName = "main"; break;
			}
		}

		// Compute shader: emit [numthreads(X,Y,Z)]
		if (m_currentState->stage == ShaderStageType::Compute)
		{
			if (node.workgroupSize.HasValue())
			{
				const Vector3u32& workgroupSize = node.workgroupSize.GetResultingValue();
				AppendLine("[numthreads(", workgroupSize.x(), ", ", workgroupSize.y(), ", ", workgroupSize.z(), ")]");
			}
		}

		// Fragment shader: early depth stencil
		if (m_currentState->stage == ShaderStageType::Fragment)
		{
			if (node.earlyFragmentTests.HasValue() && node.earlyFragmentTests.GetResultingValue())
				AppendLine("[earlydepthstencil]");
		}

		// Determine return type
		bool hasReturnStruct = node.returnType.HasValue() && !IsNoType(node.returnType.GetResultingValue());

		if (hasReturnStruct)
		{
			assert(std::holds_alternative<Ast::StructType>(node.returnType.GetResultingValue()));
			std::size_t outputStructIndex = std::get<Ast::StructType>(node.returnType.GetResultingValue()).structIndex;
			const auto& structData = Nz::Retrieve(m_currentState->structs, outputStructIndex);
			Append(structData.nameOverride);
		}
		else
			Append("void");

		Append(" ", entryPointName, "(");

		// Parameters (input struct)
		if (!node.parameters.empty())
		{
			assert(node.parameters.size() == 1);
			auto& parameter = node.parameters.front();
			RegisterVariable(*parameter.varIndex, parameter.name);

			assert(IsStructType(parameter.type.GetResultingValue()));
			std::size_t structIndex = std::get<Ast::StructType>(parameter.type.GetResultingValue()).structIndex;
			const auto& structData = Nz::Retrieve(m_currentState->structs, structIndex);
			Append(structData.nameOverride, " ", parameter.name);
		}

		AppendLine(")");
		EnterScope();
		{
			m_currentState->isInEntryPoint = true;

			AppendStatementList(node.statements);

			m_currentState->isInEntryPoint = false;
		}
		LeaveScope();
	}

	void HlslWriter::HandleSourceLocation(const SourceLocation& sourceLocation, DebugLevel requiredLevel)
	{
		if (m_currentState->backendParameters.debugLevel < requiredLevel)
			return;

		if (!sourceLocation.IsValid())
			return;

		std::string_view file;
		if (sourceLocation.file)
			file = *sourceLocation.file;
		else
			file = "unknown";

		AppendLine("// @", file, ":", sourceLocation.startLine, ":", sourceLocation.startColumn);
	}

	void HlslWriter::RegisterConstant(std::size_t constIndex, std::string constName)
	{
		assert(m_currentState->constantNames.find(constIndex) == m_currentState->constantNames.end());
		m_currentState->constantNames.emplace(constIndex, std::move(constName));
	}

	void HlslWriter::RegisterStruct(std::size_t structIndex, Ast::StructDescription* desc, std::string structName)
	{
		assert(m_currentState->structs.find(structIndex) == m_currentState->structs.end());
		State::StructData structData;
		structData.desc = desc;
		structData.nameOverride = std::move(structName);

		m_currentState->structs.emplace(structIndex, std::move(structData));
	}

	void HlslWriter::RegisterVariable(std::size_t varIndex, std::string varName)
	{
		assert(m_currentState->variableNames.find(varIndex) == m_currentState->variableNames.end());
		m_currentState->variableNames.emplace(varIndex, std::move(varName));
	}

	void HlslWriter::ScopeVisit(Ast::Statement& node)
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

	void HlslWriter::Visit(Ast::ExpressionPtr& expr, bool encloseIfRequired)
	{
		bool enclose = encloseIfRequired && (GetExpressionCategory(*expr) == Ast::ExpressionCategory::Temporary);

		if (enclose)
			Append("(");

		expr->Visit(*this);

		if (enclose)
			Append(")");
	}

	void HlslWriter::Visit(Ast::AccessFieldExpression& node)
	{
		const Ast::ExpressionType* exprType = GetExpressionType(*node.expr);
		NazaraUnused(exprType);
		assert(exprType);
		assert(IsStructAddressible(*exprType));

		std::size_t structIndex = Ast::ResolveStructIndex(*exprType);
		assert(structIndex != std::numeric_limits<std::size_t>::max());

		// For unwrapped storage buffers (dyn_array member), skip the field access
		// e.g., objectBuffer.objects[i] → objectBuffer[i]
		if (m_currentState->unwrappedStorageBufferStructs.count(structIndex) > 0)
		{
			Visit(node.expr, true);
			return;
		}

		Visit(node.expr, true);

		const auto& structData = Nz::Retrieve(m_currentState->structs, structIndex);

		std::uint32_t remainingIndices = node.fieldIndex;
		for (const auto& member : structData.desc->members)
		{
			if (member.cond.HasValue() && !member.cond.GetResultingValue())
				continue;

			if (remainingIndices == 0)
			{
				Append(".", member.name);
				break;
			}

			remainingIndices--;
		}
	}

	void HlslWriter::Visit(Ast::AccessIdentifierExpression& node)
	{
		Visit(node.expr, true);

		const Ast::ExpressionType* exprType = GetExpressionType(*node.expr);
		NazaraUnused(exprType);
		assert(exprType);
		assert(IsStructAddressible(*exprType));

		for (const auto& identifierEntry : node.identifiers)
			Append(".", identifierEntry.identifier);
	}

	void HlslWriter::Visit(Ast::AccessIndexExpression& node)
	{
		Visit(node.expr, true);

		const Ast::ExpressionType* exprType = GetExpressionType(*node.expr);
		NazaraUnused(exprType);
		assert(exprType);
		assert(!IsStructAddressible(*exprType));

		assert(node.indices.size() == 1);

		Append("[");
		Visit(node.indices.front());
		Append("]");
	}

	void HlslWriter::Visit(Ast::AssignExpression& node)
	{
		// HLSL uses fmod for floating-point modulo
		if (node.op == Ast::AssignType::CompoundModulo)
		{
			bool isFmod = false;
			if (IsPrimitiveType(*node.cachedExpressionType))
			{
				Ast::PrimitiveType primitiveType = std::get<Ast::PrimitiveType>(*node.cachedExpressionType);
				if (primitiveType == Ast::PrimitiveType::Float32 || primitiveType == Ast::PrimitiveType::Float64)
					isFmod = true;
			}
			else if (IsVectorType(*node.cachedExpressionType))
			{
				Ast::PrimitiveType primitiveType = std::get<Ast::VectorType>(*node.cachedExpressionType).type;
				if (primitiveType == Ast::PrimitiveType::Float32 || primitiveType == Ast::PrimitiveType::Float64)
					isFmod = true;
			}
			else
				throw std::runtime_error("unexpected type for modulo");

			if (isFmod)
			{
				node.left->Visit(*this);
				Append(" = fmod(");
				node.left->Visit(*this);
				Append(", ");
				Visit(node.right);
				Append(")");

				return;
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

	void HlslWriter::Visit(Ast::BinaryExpression& node)
	{
		switch (node.op)
		{
			case Ast::BinaryType::Multiply:
			{
				// HLSL requires mul() for matrix-matrix, matrix-vector, and vector-matrix multiplication
				bool leftIsMatrix = node.left->cachedExpressionType && IsMatrixType(*node.left->cachedExpressionType);
				bool rightIsMatrix = node.right->cachedExpressionType && IsMatrixType(*node.right->cachedExpressionType);
				bool leftIsVector = node.left->cachedExpressionType && IsVectorType(*node.left->cachedExpressionType);
				bool rightIsVector = node.right->cachedExpressionType && IsVectorType(*node.right->cachedExpressionType);

				if (leftIsMatrix || rightIsMatrix)
				{
					// mat*mat, mat*vec, vec*mat all need mul()
					if (leftIsMatrix && (rightIsMatrix || rightIsVector))
					{
						Append("mul(");
						Visit(node.left);
						Append(", ");
						Visit(node.right);
						Append(")");
						return;
					}
					if (rightIsMatrix && leftIsVector)
					{
						Append("mul(");
						Visit(node.left);
						Append(", ");
						Visit(node.right);
						Append(")");
						return;
					}
				}
				break;
			}

			case Ast::BinaryType::CompEq:
			case Ast::BinaryType::CompNe:
			case Ast::BinaryType::CompGe:
			case Ast::BinaryType::CompGt:
			case Ast::BinaryType::CompLe:
			case Ast::BinaryType::CompLt:
			{
				// HLSL doesn't have component-wise vector comparison functions like GLSL
				// Comparison operators on vectors in HLSL return bool vectors directly
				break;
			}

			case Ast::BinaryType::Modulo:
			{
				// HLSL uses fmod for floating-point modulo
				auto BuildFmod = [&]
				{
					Append("fmod(");
					Visit(node.left);
					Append(", ");
					Visit(node.right);
					Append(")");
				};

				if (IsPrimitiveType(*node.cachedExpressionType))
				{
					Ast::PrimitiveType primitiveType = std::get<Ast::PrimitiveType>(*node.cachedExpressionType);
					if (primitiveType == Ast::PrimitiveType::Float32 || primitiveType == Ast::PrimitiveType::Float64)
						return BuildFmod();
				}
				else if (IsVectorType(*node.cachedExpressionType))
				{
					const Ast::VectorType& vecType = std::get<Ast::VectorType>(*node.cachedExpressionType);
					Ast::PrimitiveType primitiveType = vecType.type;
					if (primitiveType == Ast::PrimitiveType::Float32 || primitiveType == Ast::PrimitiveType::Float64)
					{
						if (IsPrimitiveType(*node.left->cachedExpressionType))
						{
							Append("fmod(");
							Append(vecType, "(");
							Visit(node.left);
							Append("), ");
							Visit(node.right);
							Append(")");
							return;
						}

						return BuildFmod();
					}
				}
				else
					throw std::runtime_error("unexpected type for modulo");

				break;
			}

			default:
				break;
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

			case Ast::BinaryType::BitwiseAnd:  Append(" & ");  break;
			case Ast::BinaryType::BitwiseOr:   Append(" | ");  break;
			case Ast::BinaryType::BitwiseXor:  Append(" ^ ");  break;
			case Ast::BinaryType::ShiftLeft:   Append(" << "); break;
			case Ast::BinaryType::ShiftRight:  Append(" >> "); break;
		}

		Visit(node.right, true);
	}

	void HlslWriter::Visit(Ast::CallFunctionExpression& node)
	{
		node.targetFunction->Visit(*this);

		Append("(");
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			if (i != 0)
				Append(", ");

			node.parameters[i].expr->Visit(*this);
		}
		Append(")");
	}

	void HlslWriter::Visit(Ast::CastExpression& node)
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

	void HlslWriter::Visit(Ast::ConstantArrayValueExpression& node)
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

	void HlslWriter::Visit(Ast::ConstantValueExpression& node)
	{
		std::visit([&](auto&& arg)
		{
			AppendValue(arg);
		}, node.value);
	}

	void HlslWriter::Visit(Ast::IdentifierValueExpression& node)
	{
		switch (node.identifierType)
		{
			case Ast::IdentifierType::Alias:            throw std::runtime_error("unexpected Alias identifier, shader is not properly resolved");
			case Ast::IdentifierType::ExternalBlock:    throw std::runtime_error("unexpected ExternalBlock identifier, shader is not properly resolved");
			case Ast::IdentifierType::Intrinsic:        throw std::runtime_error("unexpected Intrinsic identifier, shader is not properly resolved");
			case Ast::IdentifierType::Module:           throw std::runtime_error("unexpected Module identifier, shader is not properly resolved");
			case Ast::IdentifierType::Struct:           throw std::runtime_error("unexpected Struct identifier, shader is not properly resolved");
			case Ast::IdentifierType::Type:             throw std::runtime_error("unexpected Type identifier, shader is not properly resolved");
			case Ast::IdentifierType::Unresolved:       throw std::runtime_error("unexpected Unresolved identifier, shader is not properly resolved");

			case Ast::IdentifierType::Constant:
			{
				Append(Nz::Retrieve(m_currentState->constantNames, node.identifierIndex));
				break;
			}

			case Ast::IdentifierType::Function:
			{
				const auto& funcData = Nz::Retrieve(m_currentState->previsitor.functions, node.identifierIndex);
				Append(funcData.name);
				break;
			}

			case Ast::IdentifierType::Variable:
			{
				const std::string& varName = Nz::Retrieve(m_currentState->variableNames, node.identifierIndex);
				Append(varName);
				break;
			}
		}
	}

	void HlslWriter::Visit(Ast::IntrinsicExpression& node)
	{
		bool firstParam = true;
		bool cast = false;
		std::size_t firstParamIndex = 0;
		switch (node.intrinsic)
		{
			// Function intrinsics - HLSL names
			case Ast::IntrinsicType::Abs:                      Append("abs");          break;
			case Ast::IntrinsicType::All:                      Append("all");          break;
			case Ast::IntrinsicType::Any:                      Append("any");          break;
			case Ast::IntrinsicType::ArcCos:                   Append("acos");         break;
			case Ast::IntrinsicType::ArcCosh:                  Append("acosh");        break; //< SM 6.0+
			case Ast::IntrinsicType::ArcSin:                   Append("asin");         break;
			case Ast::IntrinsicType::ArcSinh:                  Append("asinh");        break; //< SM 6.0+
			case Ast::IntrinsicType::ArcTan:                   Append("atan");         break;
			case Ast::IntrinsicType::ArcTan2:                  Append("atan2");        break;
			case Ast::IntrinsicType::ArcTanh:                  Append("atanh");        break; //< SM 6.0+
			case Ast::IntrinsicType::Ceil:                     Append("ceil");         break;
			case Ast::IntrinsicType::Clamp:                    Append("clamp");        break;
			case Ast::IntrinsicType::Cos:                      Append("cos");          break;
			case Ast::IntrinsicType::Cosh:                     Append("cosh");         break;
			case Ast::IntrinsicType::CrossProduct:             Append("cross");        break;
			case Ast::IntrinsicType::DegToRad:                 Append("radians");      break;
			case Ast::IntrinsicType::Ddx:                      Append("ddx");          break;
			case Ast::IntrinsicType::DdxCoarse:                Append("ddx_coarse");   break;
			case Ast::IntrinsicType::DdxFine:                  Append("ddx_fine");     break;
			case Ast::IntrinsicType::Ddy:                      Append("ddy");          break;
			case Ast::IntrinsicType::DdyCoarse:                Append("ddy_coarse");   break;
			case Ast::IntrinsicType::DdyFine:                  Append("ddy_fine");     break;
			case Ast::IntrinsicType::Distance:                 Append("distance");     break;
			case Ast::IntrinsicType::DotProduct:               Append("dot");          break;
			case Ast::IntrinsicType::Exp:                      Append("exp");          break;
			case Ast::IntrinsicType::Exp2:                     Append("exp2");         break;
			case Ast::IntrinsicType::Floor:                    Append("floor");        break;
			case Ast::IntrinsicType::Fract:                    Append("frac");         break;
			case Ast::IntrinsicType::Fwidth:                   Append("fwidth");       break;
			case Ast::IntrinsicType::FwidthCoarse:             Append("fwidth");       break; //< HLSL doesn't distinguish coarse/fine for fwidth
			case Ast::IntrinsicType::FwidthFine:               Append("fwidth");       break;
			case Ast::IntrinsicType::IsInf:                    Append("isinf");        break;
			case Ast::IntrinsicType::IsNaN:                    Append("isnan");        break;
			case Ast::IntrinsicType::Length:                   Append("length");       break;
			case Ast::IntrinsicType::Lerp:                     Append("lerp");         break;
			case Ast::IntrinsicType::Log:                      Append("log");          break;
			case Ast::IntrinsicType::Log2:                     Append("log2");         break;
			case Ast::IntrinsicType::InverseSqrt:              Append("rsqrt");        break;
			case Ast::IntrinsicType::MatrixInverse:            Append("_nzsl_inverse"); break;
			case Ast::IntrinsicType::MatrixTranspose:          Append("transpose");    break;
			case Ast::IntrinsicType::Max:                      Append("max");          break;
			case Ast::IntrinsicType::Min:                      Append("min");          break;
			case Ast::IntrinsicType::Normalize:                Append("normalize");    break;
			case Ast::IntrinsicType::Not:                      // HLSL doesn't have a not() function for vectors
			{
				Append("!");
				assert(!node.parameters.empty());
				node.parameters[0]->Visit(*this);
				return;
			}
			case Ast::IntrinsicType::Pow:                      Append("pow");          break;
			case Ast::IntrinsicType::Reflect:                  Append("reflect");      break;
			case Ast::IntrinsicType::Sin:                      Append("sin");          break;
			case Ast::IntrinsicType::Sinh:                     Append("sinh");         break;
			case Ast::IntrinsicType::SmoothStep:               Append("smoothstep");   break;
			case Ast::IntrinsicType::Sqrt:                     Append("sqrt");         break;
			case Ast::IntrinsicType::Step:                     Append("step");         break;
			case Ast::IntrinsicType::Tan:                      Append("tan");          break;
			case Ast::IntrinsicType::Tanh:                     Append("tanh");         break;
			case Ast::IntrinsicType::RadToDeg:                 Append("degrees");      break;
			case Ast::IntrinsicType::Round:                    Append("round");        break;
			case Ast::IntrinsicType::RoundEven:                Append("round");        break; //< HLSL round() does round-half-to-even
			case Ast::IntrinsicType::Sign:                     Append("sign");         break;
			case Ast::IntrinsicType::Trunc:                    Append("trunc");        break;

			// Texture read (imageLoad equivalent) - tex[coords] or tex.Load(coords)
			case Ast::IntrinsicType::TextureRead:
			{
				assert(node.parameters.size() >= 2);
				node.parameters[0]->Visit(*this);
				Append("[");
				node.parameters[1]->Visit(*this);
				Append("]");
				return;
			}

			// Texture sample - tex.Sample(sampler, coords)
			case Ast::IntrinsicType::TextureSampleImplicitLod:
			{
				assert(node.parameters.size() >= 2);

				// In NZSL, combined samplers have texture and sampler in one
				// For HLSL, we need to split: texture.Sample(samplerState, coords)
				// But since we're dealing with combined samplers at the AST level,
				// the first param is the sampler, second is coords
				// We emit: param0.Sample(param0_sampler, param1)
				// For now, since we don't have separate sampler state, we use a naming convention
				node.parameters[0]->Visit(*this);
				Append(".Sample(");
				node.parameters[0]->Visit(*this);
				Append("_sampler, ");

				for (std::size_t i = 1; i < node.parameters.size(); ++i)
				{
					if (i > 1)
						Append(", ");
					node.parameters[i]->Visit(*this);
				}
				Append(")");
				return;
			}

			// Texture sample with depth comparison
			case Ast::IntrinsicType::TextureSampleImplicitLodDepthComp:
			{
				assert(node.parameters.size() >= 3);

				node.parameters[0]->Visit(*this);
				Append(".SampleCmp(");
				node.parameters[0]->Visit(*this);
				Append("_sampler, ");
				node.parameters[1]->Visit(*this);
				Append(", ");
				node.parameters[2]->Visit(*this);

				for (std::size_t i = 3; i < node.parameters.size(); ++i)
				{
					Append(", ");
					node.parameters[i]->Visit(*this);
				}
				Append(")");
				return;
			}

			// Texture write (imageStore equivalent) - tex[coords] = value
			case Ast::IntrinsicType::TextureWrite:
			{
				assert(node.parameters.size() >= 3);
				node.parameters[0]->Visit(*this);
				Append("[");
				node.parameters[1]->Visit(*this);
				Append("] = ");
				node.parameters[2]->Visit(*this);
				return;
			}

			// select -> ternary or lerp depending on type
			case Ast::IntrinsicType::Select:
			{
				const Ast::ExpressionType& firstParamType = ResolveAlias(EnsureExpressionType(*node.parameters[1]));
				const Ast::ExpressionType& condParamType = ResolveAlias(EnsureExpressionType(*node.parameters[0]));

				if (IsVectorType(firstParamType))
				{
					const auto& targetType = ResolveAlias(EnsureExpressionType(node));
					std::size_t componentCount = std::get<Ast::VectorType>(firstParamType).componentCount;

					auto AppendTernary = [&](std::size_t componentIndex)
					{
						const char* componentStr = "xyzw";

						Append("(");
						node.parameters[0]->Visit(*this);
						if (IsVectorType(condParamType))
							Append(".", componentStr[componentIndex]);
						Append(") ? ");
						node.parameters[1]->Visit(*this);
						Append(".", componentStr[componentIndex]);
						Append(" : ");
						node.parameters[2]->Visit(*this);
						Append(".", componentStr[componentIndex]);
					};

					Append(targetType, "(");
					for (std::size_t i = 0; i < componentCount; ++i)
					{
						if (i != 0)
							Append(", ");

						AppendTernary(i);
					}
					Append(")");
				}
				else
				{
					Append("(");
					node.parameters[0]->Visit(*this);
					Append(") ? ");
					node.parameters[1]->Visit(*this);
					Append(" : ");
					node.parameters[2]->Visit(*this);
				}
				return;
			}

			// ArraySize -> length (StructuredBuffer.length() in HLSL returns uint)
			case Ast::IntrinsicType::ArraySize:
				assert(!node.parameters.empty());
				Append("uint(");
				Visit(node.parameters.front(), true);
				Append(".Length");
				cast = true;
				firstParamIndex = 1;
				break;
		}

		if (firstParam)
			Append("(");

		for (std::size_t i = firstParamIndex; i < node.parameters.size(); ++i)
		{
			if (!firstParam)
				Append(", ");

			firstParam = false;

			node.parameters[i]->Visit(*this);
		}
		Append(")");

		if (cast)
			Append(")");
	}

	void HlslWriter::Visit(Ast::SwizzleExpression& node)
	{
		Visit(node.expression, true);
		Append(".");

		const char* componentStr = "xyzw";
		for (std::size_t i = 0; i < node.componentCount; ++i)
			Append(componentStr[node.components[i]]);
	}

	void HlslWriter::Visit(Ast::TypeConstantExpression& node)
	{
		assert(IsPrimitiveType(node.type));
		Ast::PrimitiveType primitiveType = std::get<Ast::PrimitiveType>(node.type);

		auto AppendConstant = [&](auto&& type)
		{
			using T = std::decay_t<decltype(type)>;

			if (node.typeConstant == Ast::TypeConstant::Max)
			{
				if constexpr (std::is_same_v<T, float>)
					return Append("3.402823466e+38");
				else if constexpr (std::is_same_v<T, double>)
					return Append("1.7976931348623158e+308lf");
				else
					return AppendValue(Nz::MaxValue<T>());
			}

			if (node.typeConstant == Ast::TypeConstant::Min)
			{
				if constexpr (std::is_same_v<T, float>)
					return Append("-3.402823466e+38");
				else if constexpr (std::is_same_v<T, double>)
					return Append("-1.7976931348623158e+308lf");
				else
					return AppendValue(std::numeric_limits<T>::lowest());
			}

			if constexpr (std::is_floating_point_v<T>)
			{
				if (node.typeConstant == Ast::TypeConstant::Epsilon)
				{
					if constexpr (std::is_same_v<T, float>)
						return Append("1.192092896e-07");
					else if constexpr (std::is_same_v<T, double>)
						return Append("2.2204460492503131e-016lf");
					else
						static_assert(Nz::AlwaysFalse<T>(), "unhandled type");
				}

				if (node.typeConstant == Ast::TypeConstant::Infinity)
				{
					// HLSL doesn't have a literal for infinity, use 1.#INF
					Append("(");
					AppendValue(T{ 1 });
					Append(" / ");
					AppendValue(T{ 0 });
					Append(")");
					return;
				}

				if (node.typeConstant == Ast::TypeConstant::MinPositive)
				{
					if constexpr (std::is_same_v<T, float>)
						return Append("1.175494351e-38");
					else if constexpr (std::is_same_v<T, double>)
						return Append("2.2250738585072014e-308lf");
					else
						static_assert(Nz::AlwaysFalse<T>(), "unhandled type");
				}

				if (node.typeConstant == Ast::TypeConstant::NaN)
				{
					Append("(");
					AppendValue(T{ 0 });
					Append(" / ");
					AppendValue(T{ 0 });
					Append(")");
					return;
				}
			}

			throw std::runtime_error("unexpected type constant with type");
		};

		switch (primitiveType)
		{
			case Ast::PrimitiveType::Float32: AppendConstant(float{}); break;
			case Ast::PrimitiveType::Float64: AppendConstant(double{}); break;
			case Ast::PrimitiveType::Int32:   AppendConstant(std::int32_t{}); break;
			case Ast::PrimitiveType::UInt32:  AppendConstant(std::uint32_t{}); break;

			case Ast::PrimitiveType::Boolean:
			case Ast::PrimitiveType::FloatLiteral:
			case Ast::PrimitiveType::IntLiteral:
			case Ast::PrimitiveType::String:
				throw std::runtime_error("unexpected primitive type");
		}
	}

	void HlslWriter::Visit(Ast::UnaryExpression& node)
	{
		switch (node.op)
		{
			case Ast::UnaryType::BitwiseNot:
				Append("~");
				break;

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

	void HlslWriter::Visit(Ast::BranchStatement& node)
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

	void HlslWriter::Visit(Ast::BreakStatement& /*node*/)
	{
		Append("break;");
	}

	void HlslWriter::Visit(Ast::ContinueStatement& /*node*/)
	{
		Append("continue;");
	}

	void HlslWriter::Visit(Ast::DeclareAliasStatement& /*node*/)
	{
		throw std::runtime_error("unexpected alias declaration, is shader sanitized?");
	}

	void HlslWriter::Visit(Ast::DeclareConstStatement& node)
	{
		HandleSourceLocation(node.sourceLocation, DebugLevel::Regular);

		assert(node.constIndex);
		RegisterConstant(*node.constIndex, node.name);

		Append("static const ");
		AppendVariableDeclaration(node.type.GetResultingValue(), node.name);

		Append(" = ");
		node.expression->Visit(*this);
		Append(";");
	}

	void HlslWriter::Visit(Ast::DeclareExternalStatement& node)
	{
		HandleSourceLocation(node.sourceLocation, DebugLevel::Regular);

		if (!node.tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
			AppendComment("external block tag: " + node.tag);

		// Collect primitive externals to wrap them in a single cbuffer
		struct PrimitiveExternalInfo
		{
			const Ast::ExpressionType* type;
			std::string varName;
			std::size_t varIndex;
			std::uint32_t bindingIndex;
			std::uint32_t bindingSet;
		};
		std::vector<PrimitiveExternalInfo> primitiveExternals;

		for (const auto& externalVar : node.externalVars)
		{
			if (!externalVar.tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
				AppendComment("external var tag: " + externalVar.tag);

			const Ast::ExpressionType& exprType = externalVar.type.GetResultingValue();

			std::string varName = externalVar.name + m_currentState->moduleSuffix;
			if (!node.name.empty())
				varName = fmt::format("{}_{}", node.name, varName);

			if (m_currentState->reservedNames.count(varName) > 0)
			{
				unsigned int cloneIndex = 2;
				std::string candidateName;
				do
				{
					candidateName = fmt::format("{}_{}", varName, cloneIndex++);
				}
				while (m_currentState->reservedNames.count(candidateName) > 0);

				varName = std::move(candidateName);
			}

			m_currentState->reservedNames.insert(varName);

			// Determine register type and binding
			std::uint32_t bindingIndex = 0;
			std::uint32_t bindingSet = 0;

			if (externalVar.bindingIndex.HasValue())
				bindingIndex = externalVar.bindingIndex.GetResultingValue();
			if (externalVar.bindingSet.HasValue())
				bindingSet = externalVar.bindingSet.GetResultingValue();

			if (IsUniformType(exprType) || IsPushConstantType(exprType))
			{
				// cbuffer — use a struct variable inside so dot-access (e.g. cameraBuffer.viewproj) works
				std::size_t structIndex;
				if (IsUniformType(exprType))
					structIndex = std::get<Ast::UniformType>(exprType).containedType.structIndex;
				else
					structIndex = std::get<Ast::PushConstantType>(exprType).containedType.structIndex;

				const auto& structData = Nz::Retrieve(m_currentState->structs, structIndex);

				if (!structData.desc->tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
					AppendComment("struct tag: " + structData.desc->tag);

				// Emit the struct type definition (used as cbuffer content)
				AppendLine("struct _nzslType_", varName);
				EnterScope();
				{
					bool first = true;
					for (const auto& member : structData.desc->members)
					{
						if (member.cond.HasValue() && !member.cond.GetResultingValue())
							continue;

						if (!first)
							AppendLine();

						first = false;

						if (!member.tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
							AppendComment("member tag: " + member.tag);

						AppendVariableDeclaration(member.type.GetResultingValue(), member.name);
						Append(";");
					}
				}
				LeaveScope(false);
				AppendLine(";");
				AppendLine();

				Append("cbuffer _nzslCbuf_", varName, " : register(b", bindingIndex);
				if (bindingSet > 0)
					Append(", space", bindingSet);
				AppendLine(")");
				EnterScope();
				{
					Append("_nzslType_", varName, " ", varName, ";");
				}
				LeaveScope(false);
				AppendLine(";");
				AppendLine();

				assert(externalVar.varIndex);
				RegisterVariable(*externalVar.varIndex, varName);
			}
			else if (IsStorageType(exprType))
			{
				const Ast::StorageType& storageType = std::get<Ast::StorageType>(exprType);
				std::size_t structIndex = storageType.containedType.structIndex;
				const auto& structData = Nz::Retrieve(m_currentState->structs, structIndex);

				if (!structData.desc->tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
					AppendComment("struct tag: " + structData.desc->tag);

				// Determine if RW or read-only
				bool isReadOnly = (storageType.accessPolicy == AccessPolicy::ReadOnly);

				// Check if the struct has a single dyn_array member — if so, unwrap it
				// e.g., struct ObjectBuffer { objects: dyn_array[ObjectData] } → StructuredBuffer<ObjectData>
				std::string elementTypeName = structData.nameOverride;
				bool unwrapped = false;

				if (structData.desc->members.size() == 1)
				{
					const auto& member = structData.desc->members[0];
					if (member.type.HasValue() && Ast::IsDynArrayType(member.type.GetResultingValue()))
					{
						const auto& dynArrayType = std::get<Ast::DynArrayType>(member.type.GetResultingValue());
						const Ast::ExpressionType& innerType = dynArrayType.InnerType();
						if (Ast::IsStructType(innerType))
						{
							std::size_t innerStructIndex = std::get<Ast::StructType>(innerType).structIndex;
							const auto& innerStructData = Nz::Retrieve(m_currentState->structs, innerStructIndex);
							elementTypeName = innerStructData.nameOverride;
							unwrapped = true;
							m_currentState->unwrappedStorageBufferStructs.insert(structIndex);
						}
					}
				}

				if (isReadOnly)
					Append("StructuredBuffer<", elementTypeName, ">");
				else
					Append("RWStructuredBuffer<", elementTypeName, ">");

				Append(" ", varName, " : register(");
				Append(isReadOnly ? "t" : "u", bindingIndex);
				if (bindingSet > 0)
					Append(", space", bindingSet);
				AppendLine(");");
				AppendLine();

				assert(externalVar.varIndex);
				RegisterVariable(*externalVar.varIndex, varName);
			}
			else if (IsSamplerType(exprType))
			{
				const Ast::SamplerType& samplerType = std::get<Ast::SamplerType>(exprType);

				// In HLSL, we need to emit both a Texture and a SamplerState for combined samplers
				// Texture
				Append(samplerType);
				Append(" ", varName, " : register(t", bindingIndex);
				if (bindingSet > 0)
					Append(", space", bindingSet);
				AppendLine(");");

				// SamplerState
				if (samplerType.depth)
					Append("SamplerComparisonState");
				else
					Append("SamplerState");
				Append(" ", varName, "_sampler : register(s", bindingIndex);
				if (bindingSet > 0)
					Append(", space", bindingSet);
				AppendLine(");");
				AppendLine();

				assert(externalVar.varIndex);
				RegisterVariable(*externalVar.varIndex, varName);
			}
			else if (IsTextureType(exprType))
			{
				AppendVariableDeclaration(externalVar.type.GetResultingValue(), varName);
				Append(" : register(");

				const Ast::TextureType& textureType = std::get<Ast::TextureType>(exprType);
				bool isReadWrite = (textureType.accessPolicy == AccessPolicy::ReadWrite || textureType.accessPolicy == AccessPolicy::WriteOnly);
				Append(isReadWrite ? "u" : "t", bindingIndex);
				if (bindingSet > 0)
					Append(", space", bindingSet);
				AppendLine(");");
				AppendLine();

				assert(externalVar.varIndex);
				RegisterVariable(*externalVar.varIndex, varName);
			}
			else
			{
				// Primitive external (not a buffer/sampler/texture) — collect for cbuffer wrapping
				assert(externalVar.varIndex);
				primitiveExternals.push_back({&exprType, std::move(varName), *externalVar.varIndex, bindingIndex, bindingSet});
			}
		}

		// Wrap collected primitive externals in a single cbuffer
		if (!primitiveExternals.empty())
		{
			const auto& first = primitiveExternals.front();
			std::string cbufferName = !node.name.empty() ? node.name : "globals";
			Append("cbuffer _nzslCbuf_", cbufferName, " : register(b", first.bindingIndex);
			if (first.bindingSet > 0)
				Append(", space", first.bindingSet);
			AppendLine(")");
			EnterScope();
			{
				bool isFirst = true;
				for (const auto& prim : primitiveExternals)
				{
					if (!isFirst)
						AppendLine();

					isFirst = false;

					AppendVariableDeclaration(*prim.type, prim.varName);
					Append(";");
				}
			}
			LeaveScope(false);
			AppendLine(";");
			AppendLine();

			for (const auto& prim : primitiveExternals)
				RegisterVariable(prim.varIndex, prim.varName);
		}
	}

	void HlslWriter::Visit(Ast::DeclareFunctionStatement& node)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (node.entryStage.HasValue())
		{
			const auto& entryPoints = m_currentState->previsitor.entryPoints;
			if (entryPoints.empty())
			{
				if (m_currentState->previsitor.entryPoint != &node)
					return;
			}
			else
			{
				if (std::find(entryPoints.begin(), entryPoints.end(), &node) == entryPoints.end())
					return;
			}
		}

		assert(node.funcIndex);
		auto& funcData = Nz::Retrieve(m_currentState->previsitor.functions, node.funcIndex.value());

		// Declare functions called by this function which aren't already defined
		bool hasPredeclaration = false;
		for (std::size_t i : funcData.calledFunctions.IterBits())
		{
			if (m_currentState->declaredFunctions.UnboundedTest(i))
				continue;

			hasPredeclaration = true;

			auto& targetFunc = Nz::Retrieve(m_currentState->previsitor.functions, i);
			AppendFunctionDeclaration(*targetFunc.node, targetFunc.name, true);

			m_currentState->declaredFunctions.UnboundedSet(i);
		}

		if (hasPredeclaration)
			AppendLine();

		if (node.entryStage.HasValue())
			return HandleEntryPoint(node);

		HandleSourceLocation(node.sourceLocation, DebugLevel::Regular);

		for (const auto& parameter : node.parameters)
		{
			assert(parameter.varIndex);
			RegisterVariable(*parameter.varIndex, parameter.name);
		}

		AppendFunctionDeclaration(node, funcData.name);
		EnterScope();
		{
			AppendStatementList(node.statements);
		}
		LeaveScope();

		m_currentState->declaredFunctions.UnboundedSet(node.funcIndex.value());
	}

	void HlslWriter::Visit(Ast::DeclareOptionStatement& /*node*/)
	{
		throw std::runtime_error("unexpected option declaration, is shader sanitized?");
	}

	void HlslWriter::Visit(Ast::DeclareStructStatement& node)
	{
		std::string structName = node.description.name + m_currentState->moduleSuffix;

		assert(node.structIndex);
		RegisterStruct(*node.structIndex, &node.description, structName);

		// Don't output structs used for UBO/SSBO description
		if (m_currentState->previsitor.bufferStructs.UnboundedTest(*node.structIndex))
		{
			if (m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
				AppendComment("struct " + structName + " omitted (used as cbuffer/StructuredBuffer)");

			return;
		}

		HandleSourceLocation(node.sourceLocation, DebugLevel::Regular);

		if (!node.description.tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
			AppendComment("struct tag: " + node.description.tag);

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

				if (!member.tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
					AppendComment("member tag: " + member.tag);

				if (member.interp.HasValue())
					Append(member.interp.GetResultingValue(), " ");

				// SV_VertexID and SV_InstanceID must be uint in HLSL regardless of NZSL type
				bool forceUint = member.builtin.HasValue() &&
				                 (member.builtin.GetResultingValue() == Ast::BuiltinEntry::VertexIndex ||
				                  member.builtin.GetResultingValue() == Ast::BuiltinEntry::InstanceIndex);
				if (forceUint)
					Append("uint ", member.name);
				else
					AppendVariableDeclaration(member.type.GetResultingValue(), member.name);
				bool isFragOutput = m_currentState->previsitor.fragmentOutputStructIndex.has_value()
				                 && *node.structIndex == *m_currentState->previsitor.fragmentOutputStructIndex;
				AppendSemantic(member, isFragOutput);
				Append(";");
			}

			// Empty structs are not allowed in HLSL
			if (first)
				AppendLine("int _dummy;");
		}
		LeaveScope(false);
		AppendLine(";");
	}

	void HlslWriter::Visit(Ast::DeclareVariableStatement& node)
	{
		assert(node.varIndex);

		std::string varName = node.varName;
		if (m_currentState->reservedNames.count(varName) > 0)
		{
			unsigned int cloneIndex = 2;
			std::string candidateName;
			do
			{
				candidateName = fmt::format("{}_{}", varName, cloneIndex++);
			} while (m_currentState->reservedNames.count(candidateName) > 0);

			varName = std::move(candidateName);
		}

		AppendVariableDeclaration(node.varType.GetResultingValue(), varName);
		RegisterVariable(*node.varIndex, std::move(varName));

		if (node.initialExpression)
		{
			Append(" = ");
			node.initialExpression->Visit(*this);
		}

		Append(";");
	}

	void HlslWriter::Visit(Ast::DiscardStatement& /*node*/)
	{
		Append("discard;");
	}

	void HlslWriter::Visit(Ast::ExpressionStatement& node)
	{
		node.expression->Visit(*this);
		Append(";");
	}

	void HlslWriter::Visit(Ast::ImportStatement& /*node*/)
	{
		throw std::runtime_error("unexpected import statement, is the shader sanitized properly?");
	}

	void HlslWriter::Visit(Ast::MultiStatement& node)
	{
		AppendStatementList(node.statements);
	}

	void HlslWriter::Visit(Ast::NoOpStatement& /*node*/)
	{
		/* nothing to do */
	}

	void HlslWriter::Visit(Ast::ReturnStatement& node)
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

	void HlslWriter::Visit(Ast::ScopedStatement& node)
	{
		EnterScope();
		node.statement->Visit(*this);
		LeaveScope(true);
	}

	void HlslWriter::Visit(Ast::WhileStatement& node)
	{
		Append("while (");
		node.condition->Visit(*this);
		AppendLine(")");

		ScopeVisit(*node.body);
	}
}
