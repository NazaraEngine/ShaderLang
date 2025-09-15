// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include "NZSL/Ast/Enums.hpp"
#include "NZSL/Ast/ExpressionType.hpp"
#include <NZSL/WgslWriter.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Lexer.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Ast/Transformations/BindingResolverTransformer.hpp>
#include <NZSL/Ast/Transformations/BranchSplitterTransformer.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <NZSL/Ast/Transformations/ConstantRemovalTransformer.hpp>
#include <NZSL/Ast/Transformations/EliminateUnusedTransformer.hpp>
#include <NZSL/Ast/Transformations/ForToWhileTransformer.hpp>
#include <NZSL/Ast/Transformations/IdentifierTransformer.hpp>
#include <NZSL/Ast/Transformations/LiteralTransformer.hpp>
#include <NZSL/Ast/Transformations/MatrixTransformer.hpp>
#include <NZSL/Ast/Transformations/ResolveTransformer.hpp>
#include <NZSL/Ast/Transformations/StructAssignmentTransformer.hpp>
#include <NZSL/Ast/Transformations/SwizzleTransformer.hpp>
#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <fmt/format.h>
#include <tsl/ordered_set.h>
#include <frozen/unordered_map.h>
#include <frozen/unordered_set.h>
#include <cassert>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <algorithm>

#include <iostream>

namespace nzsl
{
	constexpr std::string_view s_wgslWriterShaderDrawParametersBaseInstanceName = "_nzslBaseInstance";
	constexpr std::string_view s_wgslWriterShaderDrawParametersBaseVertexName = "_nzslBaseVertex";
	constexpr std::string_view s_wgslWriterShaderDrawParametersDrawIndexName = "_nzslDrawID";

	enum class WgslFeature
	{
		None = -1,

		// wgpu native features
		WgpuBufferBindingArray,
		WgpuConservativeDepth, 
		WgpuEarlyFragmentTests,
		WgpuFloat64,
		WgpuPushConstants,
		WgpuStorageBindingArray,
		WgpuTextureBindingArray,
	};

	struct WgslBuiltin
	{
		std::string_view identifier;
		WgslFeature requiredFeature;
	};

	constexpr auto s_wgslBuiltinMapping = frozen::make_unordered_map<Ast::BuiltinEntry, WgslBuiltin>({
		{ Ast::BuiltinEntry::BaseInstance,            {} },
		{ Ast::BuiltinEntry::BaseVertex,              {} },
		{ Ast::BuiltinEntry::DrawIndex,               {} },
		{ Ast::BuiltinEntry::FragCoord,               { "position", WgslFeature::None } },
		{ Ast::BuiltinEntry::FragDepth,               { "frag_depth", WgslFeature::None } },
		{ Ast::BuiltinEntry::GlocalInvocationIndices, { "global_invocation_id", WgslFeature::None } },
		{ Ast::BuiltinEntry::InstanceIndex,           { "instance_index", WgslFeature::None } },
		{ Ast::BuiltinEntry::LocalInvocationIndex,    { "local_invocation_index", WgslFeature::None } },
		{ Ast::BuiltinEntry::LocalInvocationIndices,  { "local_invocation_id", WgslFeature::None } },
		{ Ast::BuiltinEntry::VertexIndex,             { "vertex_index", WgslFeature::None } },
		{ Ast::BuiltinEntry::VertexPosition,          { "position", WgslFeature::None } },
		{ Ast::BuiltinEntry::WorkgroupCount,          { "num_workgroups", WgslFeature::None } },
		{ Ast::BuiltinEntry::WorkgroupIndices,        { "workgroup_id", WgslFeature::None } }
	});

	struct WgslWriter::PreVisitor : Ast::RecursiveVisitor
	{
		PreVisitor(WgslWriter& writer) :
		m_writer(writer)
		{
		}

		void Visit(Ast::DeclareFunctionStatement& node) override
		{
			if (node.funcIndex)
				m_writer.RegisterFunction(*node.funcIndex, node.name);

			if (node.entryStage.HasValue())
			{
				ShaderStageType stage = node.entryStage.GetResultingValue();

				if (stage == ShaderStageType::Fragment)
				{
					if (node.depthWrite.HasValue() && node.depthWrite.GetResultingValue() != Ast::DepthWriteMode::Replace)
						features.insert(WgslFeature::WgpuConservativeDepth);

					if (node.earlyFragmentTests.HasValue() && node.earlyFragmentTests.GetResultingValue())
						features.insert(WgslFeature::WgpuEarlyFragmentTests);
				}
			}

			RecursiveVisitor::Visit(node);
		}

		void Visit(Ast::DeclareExternalStatement& node) override
		{
			for (const auto& extVar : node.externalVars)
			{
				const Ast::ExpressionType& type = extVar.type.GetResultingValue();
				if (IsPushConstantType(type))
					features.insert(WgslFeature::WgpuPushConstants);
				else if (IsArrayType(type))
				{
					const Ast::ArrayType& array = std::get<Ast::ArrayType>(type);
					if (IsStorageType(array.InnerType()))
						features.insert(WgslFeature::WgpuStorageBindingArray);
					else if (IsTextureType(array.InnerType()))
						features.insert(WgslFeature::WgpuTextureBindingArray);
					else if (IsStructType(array.InnerType()))
						features.insert(WgslFeature::WgpuBufferBindingArray);
				}
			}

			RecursiveVisitor::Visit(node);
		}

		void Visit(Ast::IntrinsicExpression& node) override
		{
			RecursiveVisitor::Visit(node);

			const Ast::ExpressionType& paramType = ResolveAlias(EnsureExpressionType(*node.parameters[0]));

			if (node.intrinsic == Ast::IntrinsicType::IsInf)
			{
				assert((IsVectorType(paramType) || IsPrimitiveType(paramType)) && "invalid type found");
				const Ast::PrimitiveType& type = IsVectorType(paramType) ? std::get<Ast::VectorType>(paramType).type : std::get<Ast::PrimitiveType>(paramType);

				if (type == Ast::PrimitiveType::Float32)
					hasf32Infinity = true;
				else if (type == Ast::PrimitiveType::Float64)
					hasf64Infinity = true;
				else
					assert(false && "isinf can only be used on floating types");
			}
		}

		void Visit(Ast::TypeConstantExpression& node) override
		{
			assert(IsPrimitiveType(node.type));
			Ast::PrimitiveType primitiveType = std::get<Ast::PrimitiveType>(node.type);

			if (node.typeConstant == Ast::TypeConstant::Infinity)
			{
				if (primitiveType == Ast::PrimitiveType::Float32)
					hasf32Infinity = true;
				if (primitiveType == Ast::PrimitiveType::Float64)
					hasf64Infinity = true;
			}
			else if (node.typeConstant == Ast::TypeConstant::NaN)
			{
				if (primitiveType == Ast::PrimitiveType::Float32)
					hasf32NaN = true;
				if (primitiveType == Ast::PrimitiveType::Float64)
					hasf64NaN = true;
			}
		}

		tsl::ordered_set<WgslFeature> features;
		WgslWriter& m_writer;
		bool hasf32Infinity = false;
		bool hasf32NaN = false;
		bool hasf64Infinity = false;
		bool hasf64NaN = false;
	};

	struct WgslWriter::AutoBindingAttribute
	{
		const Ast::ExpressionValue<bool>& autoBinding;

		bool HasValue() const { return autoBinding.HasValue(); }
	};

	struct WgslWriter::AuthorAttribute
	{
		const std::string& author;

		bool HasValue() const { return !author.empty(); }
	};

	struct WgslWriter::BindingAttribute
	{
		const Ast::ExpressionValue<std::uint32_t>& bindingIndex;

		bool HasValue() const { return bindingIndex.HasValue(); }
	};

	struct WgslWriter::BuiltinAttribute
	{
		const Ast::ExpressionValue<Ast::BuiltinEntry>& builtin;

		bool HasValue() const { return builtin.HasValue(); }
	};

	struct WgslWriter::CondAttribute
	{
		const Ast::ExpressionValue<bool>& cond;

		bool HasValue() const { return cond.HasValue(); }
	};

	struct WgslWriter::DepthWriteAttribute
	{
		const Ast::ExpressionValue<Ast::DepthWriteMode>& writeMode;

		bool HasValue() const { return writeMode.HasValue(); }
	};

	struct WgslWriter::DescriptionAttribute
	{
		const std::string& description;

		bool HasValue() const { return !description.empty(); }
	};

	struct WgslWriter::EarlyFragmentTestsAttribute
	{
		const Ast::ExpressionValue<bool>& earlyFragmentTests;

		bool HasValue() const { return earlyFragmentTests.HasValue(); }
	};

	struct WgslWriter::EntryAttribute
	{
		const Ast::ExpressionValue<ShaderStageType>& stageType;

		bool HasValue() const { return stageType.HasValue(); }
	};

	struct WgslWriter::FeatureAttribute
	{
		Ast::ModuleFeature featureAttribute;

		bool HasValue() const { return true; }
	};

	struct WgslWriter::InterpAttribute
	{
		const Ast::ExpressionValue<Ast::InterpolationQualifier>& interpQualifier;

		bool HasValue() const { return interpQualifier.HasValue(); }
	};

	struct WgslWriter::LayoutAttribute
	{
		const Ast::ExpressionValue<Ast::MemoryLayout>& layout;

		bool HasValue() const { return layout.HasValue(); }
	};

	struct WgslWriter::LicenseAttribute
	{
		const std::string& license;

		bool HasValue() const { return !license.empty(); }
	};

	struct WgslWriter::LocationAttribute
	{
		const Ast::ExpressionValue<std::uint32_t>& locationIndex;

		bool HasValue() const { return locationIndex.HasValue(); }
	};
	
	struct WgslWriter::SetAttribute
	{
		const Ast::ExpressionValue<std::uint32_t>& setIndex;

		bool HasValue() const { return setIndex.HasValue(); }
	};

	struct WgslWriter::TagAttribute
	{
		const std::string& tag;

		bool HasValue() const { return !tag.empty(); }
	};

	struct WgslWriter::UnrollAttribute
	{
		const Ast::ExpressionValue<Ast::LoopUnroll>& unroll;

		bool HasValue() const { return unroll.HasValue(); }
	};

	struct WgslWriter::WorkgroupAttribute
	{
		const Ast::ExpressionValue<Vector3u32>& workgroup;

		bool HasValue() const { return workgroup.HasValue(); }
	};

	struct WgslWriter::State
	{
		State(const BackendParameters& backendParameters) :
		backendParameters(backendParameters)
		{
		}

		struct Identifier
		{
			std::optional<std::size_t> externalBlockIndex;
			std::size_t moduleIndex;
			std::string name;
		};

		struct StructData : Identifier
		{
			const Ast::StructDescription* desc;
		};

		std::optional<std::size_t> currentExternalBlockIndex;
		std::size_t currentModuleIndex;
		std::stringstream stream;
		std::unordered_map<std::size_t, Identifier> aliases;
		std::unordered_map<std::size_t, Identifier> constants;
		std::unordered_map<std::size_t, Identifier> functions;
		std::unordered_map<std::size_t, Identifier> modules;
		std::unordered_map<std::size_t, StructData> structs;
		std::unordered_map<std::size_t, Identifier> variables;
		std::unordered_map<std::uint64_t, unsigned int> bindingRemap;
		std::vector<std::string> externalBlockNames;
		std::vector<std::string> moduleNames;
		const BackendParameters& backendParameters;
		bool isInEntryPoint = false;
		int streamEmptyLine = 1;
		unsigned int indentLevel = 0;
		bool isTerminatedScope = false;
		bool hasf32RatioFunction = false;
		bool hasf64RatioFunction = false;
	};

	WgslWriter::Output WgslWriter::Generate(Ast::Module& module, const BackendParameters& parameters)
	{
		State state(parameters);

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
			dependencyConfig.usedShaderStages = ShaderStageType_All;

			Ast::EliminateUnusedPass(module, dependencyConfig);
		}

		// First registration pass (required to register function names)
		PreVisitor previsitor(*this);
		{
			m_currentState->currentModuleIndex = 0;
			for (const auto& importedModule : module.importedModules)
			{
				m_currentState->currentModuleIndex++;
				importedModule.module->rootNode->Visit(previsitor);
				m_currentState->moduleNames.push_back(importedModule.identifier);
			}

			m_currentState->currentModuleIndex = 0;

			std::size_t moduleIndex = 0;
			for (const auto& importedModule : module.importedModules)
				RegisterModule(moduleIndex++, importedModule.identifier);

			module.rootNode->Visit(previsitor);
		}

		AppendHeader(*module.metadata);

		// Validate required features
		auto validateFeature = [&](std::string_view featureName, std::string_view featurePrettyName)
		{
			if (!m_environment.featuresCallback || !m_environment.featuresCallback(featureName))
				throw std::runtime_error(fmt::format("WGSL does not support {} feature, wgpu does natively but you need to confirm its usage using feature callback", featurePrettyName));
		};

		for (WgslFeature feature : previsitor.features)
		{
			switch (feature)
			{
				case WgslFeature::None: break;

				case WgslFeature::WgpuBufferBindingArray:  validateFeature("WgpuBufferBindingArray", "buffer binding array"); break;
				case WgslFeature::WgpuConservativeDepth:   validateFeature("WgpuConservativeDepth", "conservative depth"); break;
				case WgslFeature::WgpuEarlyFragmentTests:  validateFeature("WgpuEarlyFragmentTests", "early fragment depth test"); break;
				case WgslFeature::WgpuFloat64:             validateFeature("WgpuFloat64", "float 64"); break;
				case WgslFeature::WgpuPushConstants:       validateFeature("WgpuPushConstants", "push constants"); break;
				case WgslFeature::WgpuStorageBindingArray: validateFeature("WgpuStorageBindingArray", "storage resource binding array"); break;
				case WgslFeature::WgpuTextureBindingArray: validateFeature("WgpuTextureBindingArray", "texture binding array"); break;
			}
		}

		// Register imported modules
		m_currentState->currentModuleIndex = 0;
		for (const auto& importedModule : module.importedModules)
		{
			AppendModuleAttributes(*importedModule.module->metadata);
			AppendComment("Module " + importedModule.module->metadata->moduleName);

			m_currentState->currentModuleIndex++;
			importedModule.module->rootNode->Visit(*this);
			m_currentState->moduleNames.push_back(importedModule.identifier);
		}

		if (previsitor.hasf32Infinity)
			AppendConstantHelpers(Ast::PrimitiveType::Float32, Ast::TypeConstant::Infinity);
		if (previsitor.hasf32NaN)
			AppendConstantHelpers(Ast::PrimitiveType::Float32, Ast::TypeConstant::NaN);
		if (previsitor.hasf64Infinity)
			AppendConstantHelpers(Ast::PrimitiveType::Float64, Ast::TypeConstant::Infinity);
		if (previsitor.hasf64NaN)
			AppendConstantHelpers(Ast::PrimitiveType::Float64, Ast::TypeConstant::NaN);

		m_currentState->currentModuleIndex = 0;
		module.rootNode->Visit(*this);

		Output output;
		output.code = std::move(state.stream).str();
		output.bindingRemap = std::move(state.bindingRemap);

		return output;
	}

	void WgslWriter::SetEnv(Environment environment)
	{
		m_environment = std::move(environment);
	}

	void WgslWriter::RegisterPasses(Ast::TransformerExecutor& executor)
	{
		// Wtf WGSL ?
		static constexpr auto s_reservedKeywords = frozen::make_unordered_set<frozen::string>({
			"NULL", "Self", "abstract", "active", "alignas", "alignof", "as", "asm", "asm_fragment", "async",
			"attribute", "auto", "await", "become", "cast", "catch", "class", "co_await", "co_return", "co_yield",
			"coherent", "column_major", "common", "compile", "compile_fragment", "concept", "const_cast", "consteval",
			"constexpr", "constinit", "crate", "debugger", "decltype", "delete", "demote", "demote_to_helper",
			"do", "dynamic_cast", "enum", "explicit", "export", "extends", "extern", "external", "fallthrough",
			"filter", "final", "finally", "friend", "from", "fxgroup", "get", "goto", "groupshared", "highp", "impl",
			"implements", "import", "inline", "instanceof", "interface", "layout", "lowp", "macro", "macro_rules",
			"match", "mediump", "meta", "mod", "module", "move", "mut", "mutable", "namespace", "new", "nil",
			"noexcept", "noinline", "nointerpolation", "non_coherent", "noncoherent", "noperspective", "null",
			"nullptr", "of", "operator", "package", "packoffset", "partition", "pass", "patch", "pixelfragment",
			"precise", "precision", "premerge", "priv", "protected", "pub", "public", "readonly", "ref", "regardless",
			"register", "reinterpret_cast", "require", "resource", "restrict", "self", "set", "shared", "sizeof",
			"smooth", "snorm", "static", "static_assert", "static_cast", "std", "subroutine", "super", "target",
			"template", "this", "thread_local", "throw", "trait", "try", "type", "typedef", "typeid", "typename",
			"typeof", "union", "unless", "unorm", "unsafe", "unsized", "use", "using", "varying", "virtual",
			"volatile", "wgsl", "where", "with", "writeonly", "yield", "alias", "break", "case", "const", "const_assert",
			"continue", "continuing", "default", "diagnostic", "discard", "else", "enable", "false", "fn", "for",
			"if", "let", "loop", "override", "requires", "return", "struct", "switch", "true", "var", "while"
		});
		
		// We need two identifiers passes, the first one to rename reserved/forbidden variable names and the second one to ensure all variables name are uniques (which isn't guaranteed by the transformation passes)
		// We can't do this at once at the end because transformations passes will introduce variables prefixed by _nzsl which is forbidden in user code
		Ast::IdentifierTransformer::Options firstIdentifierPassOptions;
		firstIdentifierPassOptions.makeVariableNameUnique = false;
		firstIdentifierPassOptions.identifierSanitizer = [](std::string& identifier, Ast::IdentifierCategory /*scope*/)
		{
			using namespace std::string_view_literals;

			bool nameChanged = false;

			// Identifier can't start with _nzsl
			if (identifier.compare(0, 5, "_nzsl") == 0)
			{
				identifier.replace(0, 5, "_"sv);
				nameChanged = true;
			}

			// Identifier can't be only _
			if (identifier == "_")
			{
				identifier = "_2_2";
				nameChanged = true;
			}

			return nameChanged;
		};

		Ast::IdentifierTransformer::Options secondIdentifierPassOptions;
		secondIdentifierPassOptions.makeVariableNameUnique = true;
		secondIdentifierPassOptions.identifierSanitizer = [](std::string& identifier, Ast::IdentifierCategory /*scope*/)
		{
			using namespace std::string_view_literals;

			bool nameChanged = false;
			while (s_reservedKeywords.count(frozen::string(identifier)) != 0)
			{
				identifier += '_';
				nameChanged = true;
			}

			// Replace __ by _X_
			std::size_t startPos = 0;
			while ((startPos = identifier.find("__"sv, startPos)) != std::string::npos)
			{
				std::size_t endPos = identifier.find_first_not_of('_', startPos);
				identifier.replace(startPos, endPos - startPos, fmt::format("{}{}_", (startPos == 0) ? "_" : "", endPos - startPos));

				startPos = endPos;
				nameChanged = true;
			}

			return nameChanged;
		};

		executor.AddPass<Ast::LiteralTransformer>();
		//executor.AddPass<Ast::BranchSplitterTransformer>();
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
			opt.removeSwizzleAssigment = true;
		});
		executor.AddPass<Ast::MatrixTransformer>([](Ast::MatrixTransformer::Options& opt)
		{
			opt.removeMatrixBinaryAddSub = true;
			opt.removeMatrixCast = true;
		});
		executor.AddPass<Ast::BindingResolverTransformer>();
		executor.AddPass<Ast::ConstantRemovalTransformer>([](Ast::ConstantRemovalTransformer::Options& opt)
		{
			opt.removeConstArraySize = false;
			opt.removeTypeConstant = false;
		});
		executor.AddPass<Ast::IdentifierTransformer>(secondIdentifierPassOptions);
	}

	void WgslWriter::Append(const Ast::AliasType& /*type*/)
	{
		throw std::runtime_error("unexpected AliasType");
	}

	void WgslWriter::Append(const Ast::ArrayType& type)
	{
		if (type.length > 0)
			Append("array<", type.containedType->type, ", ", type.length, ">");
		else
			Append("array<", type.containedType->type, ">");
	}

	void WgslWriter::Append(const Ast::DynArrayType& type)
	{
		Append("array<", type.containedType->type, ">");
	}

	void WgslWriter::Append(const Ast::ExpressionType& type)
	{
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, type);
	}

	void WgslWriter::Append(const Ast::ExpressionValue<Ast::ExpressionType>& type)
	{
		assert(type.HasValue());
		if (type.IsResultingValue())
			Append(type.GetResultingValue());
		else
			type.GetExpression()->Visit(*this);
	}

	void WgslWriter::Append(const Ast::FunctionType& /*functionType*/)
	{
		throw std::runtime_error("unexpected function type");
	}

	void WgslWriter::Append(const Ast::IntrinsicFunctionType& /*functionType*/)
	{
		throw std::runtime_error("unexpected intrinsic function type");
	}

	void WgslWriter::Append(const Ast::ImplicitArrayType& /*type*/)
	{
		throw std::runtime_error("unexpected ImplicitArrayType");
	}

	void WgslWriter::Append(const Ast::ImplicitMatrixType& /*type*/)
	{
		throw std::runtime_error("unexpected ImplicitMatrixType");
	}

	void WgslWriter::Append(const Ast::ImplicitVectorType& /*type*/)
	{
		throw std::runtime_error("unexpected ImplicitVectorType");
	}

	void WgslWriter::Append(const Ast::MatrixType& matrixType)
	{
		Append("mat");
		Append(matrixType.columnCount);
		Append("x");
		Append(matrixType.rowCount);
		Append("<", matrixType.type, ">");
	}

	void WgslWriter::Append(const Ast::MethodType& /*functionType*/)
	{
		throw std::runtime_error("unexpected method type");
	}

	void WgslWriter::Append(const Ast::ModuleType& /*moduleType*/)
	{
		throw std::runtime_error("unexpected module type");
	}

	void WgslWriter::Append(const Ast::NamedExternalBlockType& namedExternalBlockType)
	{
		AppendComment(m_currentState->externalBlockNames[namedExternalBlockType.namedExternalBlockIndex]);
	}

	void WgslWriter::Append(Ast::PrimitiveType type)
	{
		switch (type)
		{
			case Ast::PrimitiveType::Boolean: return Append("bool");
			case Ast::PrimitiveType::Float32: return Append("f32");
			case Ast::PrimitiveType::Float64: return Append("f64");
			case Ast::PrimitiveType::Int32:   return Append("i32");
			case Ast::PrimitiveType::UInt32:  return Append("u32");
			case Ast::PrimitiveType::FloatLiteral: throw std::runtime_error("unexpected untyped float");
			case Ast::PrimitiveType::IntLiteral:   throw std::runtime_error("unexpected untyped integer");
			case Ast::PrimitiveType::String:       throw std::runtime_error("unexpected string type");
		}
	}

	void WgslWriter::Append(const Ast::PushConstantType& pushConstantType)
	{
		Append(pushConstantType.containedType);
	}

	void WgslWriter::Append(const Ast::SamplerType& samplerType)
	{
		std::string dimension;
		std::string type;
		switch (samplerType.dim)
		{
			case ImageType::E1D:
			{
				if (samplerType.depth)
					throw std::runtime_error("depth texture sampler 1D are not supported by WGSL");
				dimension = "1d";
				break;
			}
			case ImageType::E1D_Array:
			{
				if (samplerType.depth)
					throw std::runtime_error("depth texture sampler 1D array are not supported by WGSL");
				dimension = "1d_array";
				break;
			}
			case ImageType::E2D:       dimension = "2d";       break;
			case ImageType::E2D_Array: dimension = "2d_array"; break;
			case ImageType::E3D:
			{
				if (samplerType.depth)
					throw std::runtime_error("depth texture sampler 3D are not supported by WGSL");
				dimension = "3d";
				break;
			}
			case ImageType::Cubemap:   dimension = "cube";     break;
		}
		switch (samplerType.sampledType)
		{
			case Ast::PrimitiveType::Boolean:
				throw std::runtime_error("unexpected bool type for tture");
			case Ast::PrimitiveType::Float64:
				throw std::runtime_error("unexpected f64 type for teure");

			case Ast::PrimitiveType::Float32: type = "<f32>"; break;
			case Ast::PrimitiveType::Int32:   type = "<i32>"; break;
			case Ast::PrimitiveType::UInt32:  type = "<u32>"; break;

			case Ast::PrimitiveType::String:
				throw std::runtime_error("unexpected string type forexture");

			case Ast::PrimitiveType::FloatLiteral:
			case Ast::PrimitiveType::IntLiteral:
				throw std::runtime_error("unexpected litteral type for sampler");
		}
		Append("texture_");
		if (samplerType.depth)
			Append("depth_", dimension);
		else
			Append(dimension, type);
	}

	void WgslWriter::Append(const Ast::StorageType& storageType)
	{
		Append(storageType.containedType);
	}

	void WgslWriter::Append(const Ast::StructType& structType)
	{
		AppendIdentifier(m_currentState->structs, structType.structIndex, true);
	}

	void WgslWriter::Append(const Ast::TextureType& textureType)
	{
		Append("texture_storage_");
		switch (textureType.dim)
		{
			case ImageType::E1D:       Append("1d");       break;
			case ImageType::E2D:       Append("2d");       break;
			case ImageType::E2D_Array: Append("2d_array"); break;
			case ImageType::E3D:       Append("3d");       break;

			default:
				throw std::runtime_error("unexpected storage texture type");
		}
		Append("<");
		switch (textureType.format)
		{
			case ImageFormat::RGBA8:       Append("rgba8unorm");  break;
			case ImageFormat::RGBA8i:      Append("rgba8sint");   break;
			case ImageFormat::RGBA8Snorm:  Append("rgba8snorm");  break;
			case ImageFormat::RGBA8ui:     Append("rgba8uint");   break;

			case ImageFormat::RGBA16f:     Append("rgba16float"); break;
			case ImageFormat::RGBA16i:     Append("rgba16sint");  break;
			case ImageFormat::RGBA16ui:    Append("rgba16uint");  break;

			case ImageFormat::R32f:        Append("r32float");    break;
			case ImageFormat::R32i:        Append("r32sint");     break;
			case ImageFormat::R32ui:       Append("r32uint");     break;

			case ImageFormat::RG32f:       Append("rg32float");   break;
			case ImageFormat::RG32i:       Append("rg32sint");    break;
			case ImageFormat::RG32ui:      Append("rg32uint");    break;

			case ImageFormat::RGBA32f:     Append("rgba32float"); break;
			case ImageFormat::RGBA32i:     Append("rgba32sint");  break;
			case ImageFormat::RGBA32ui:    Append("rgba32uint");  break;

			default:
				throw std::runtime_error("unexpected format type for texture");
		}
		Append(", ");
		switch (textureType.accessPolicy)
		{
			case AccessPolicy::ReadOnly:  Append("read"); break;
			case AccessPolicy::ReadWrite: Append("read_write"); break;
			case AccessPolicy::WriteOnly: Append("write"); break;
		}
		Append(">");
	}

	void WgslWriter::Append(const Ast::Type& /*type*/)
	{
		throw std::runtime_error("unexpected type");
	}

	void WgslWriter::Visit(Ast::TypeConstantExpression& node)
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
					return AppendValue(std::numeric_limits<T>::lowest()); //< Nz::MinValue is implemented by std::numeric_limits<T>::min() which doesn't give the value we want
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
					if constexpr (std::is_same_v<T, float>)
						return Append("_nzslInfinityf32()");
					else if constexpr (std::is_same_v<T, double>)
						return Append("_nzslInfinityf64()");
					else
						static_assert(Nz::AlwaysFalse<T>(), "unhandled type");
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
					if constexpr (std::is_same_v<T, float>)
						return Append("_nzslNaNf32()");
					else if constexpr (std::is_same_v<T, double>)
						return Append("_nzslNaNf64()");
					else
						static_assert(Nz::AlwaysFalse<T>(), "unhandled type");
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

	void WgslWriter::Append(const Ast::UniformType& uniformType)
	{
		Append(uniformType.containedType);
	}

	void WgslWriter::Append(const Ast::VectorType& vecType)
	{
		Append("vec", vecType.componentCount, "<", vecType.type, ">");
	}

	void WgslWriter::Append(Ast::NoType)
	{
		return Append("()");
	}

	template<typename T>
	void WgslWriter::Append(const T& param)
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
	void WgslWriter::Append(const T1& firstParam, const T2& secondParam, Args&&... params)
	{
		Append(firstParam);
		Append(secondParam, std::forward<Args>(params)...);
	}

	template<typename... Args>
	void WgslWriter::AppendAttributes(bool appendLine, Args&&... params)
	{
		bool hasAnyAttribute = (params.HasValue() || ...);
		if (!hasAnyAttribute)
			return;

		bool first = true;

		AppendAttributesInternal(first, std::forward<Args>(params)...);

		if (appendLine)
			AppendLine();
		else
			Append(" ");
	}

	template<typename T>
	void WgslWriter::AppendAttributesInternal(bool& first, const T& param)
	{
		if (!param.HasValue())
			return;

		AppendAttribute(first, param);
		first = false;
	}

	template<typename T1, typename T2, typename... Rest>
	void WgslWriter::AppendAttributesInternal(bool& first, const T1& firstParam, const T2& secondParam, Rest&&... params)
	{
		AppendAttributesInternal(first, firstParam);
		AppendAttributesInternal(first, secondParam, std::forward<Rest>(params)...);
	}

	void WgslWriter::AppendAttribute(bool /*first*/, AutoBindingAttribute /*attribute*/)
	{
	}

	void WgslWriter::AppendAttribute(bool /*first*/, AuthorAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		Append("// Author ", EscapeString(attribute.author));
	}

	void WgslWriter::AppendAttribute(bool first, BindingAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		Append("binding(");

		if (attribute.bindingIndex.IsResultingValue())
			Append(attribute.bindingIndex.GetResultingValue());
		else
			attribute.bindingIndex.GetExpression()->Visit(*this);

		Append(")");
	}

	void WgslWriter::AppendAttribute(bool first, BuiltinAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");
		auto it = s_wgslBuiltinMapping.find(attribute.builtin.GetResultingValue());
		assert(it != s_wgslBuiltinMapping.end());
		if (it->second.identifier.empty())
			throw std::runtime_error("unsupported builtin attribute! (for now)");
		Append("builtin(", it->second.identifier, ")");
	}

	void WgslWriter::AppendAttribute(bool first, CondAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		Append("cond(");

		if (attribute.cond.IsResultingValue())
			Append(Ast::ToString(attribute.cond.GetResultingValue()));
		else
			attribute.cond.GetExpression()->Visit(*this);

		Append(")");
	}
	
	void WgslWriter::AppendAttribute(bool first, DepthWriteAttribute attribute)
	{
		if (!attribute.HasValue() || attribute.writeMode.GetResultingValue() == Ast::DepthWriteMode::Replace)
			return;
		if (!first)
			Append(" ");
		switch (attribute.writeMode.GetResultingValue())
		{
			case Ast::DepthWriteMode::Greater:   Append("@early_depth_test(greater_equal)"); break;
			case Ast::DepthWriteMode::Less:      Append("@early_depth_test(less_equal)"); break;
			case Ast::DepthWriteMode::Replace:   break; // Should never be triggered
			case Ast::DepthWriteMode::Unchanged: Append("@early_depth_test(unchanged)"); break;
		}
	}

	void WgslWriter::AppendAttribute(bool /*first*/, DescriptionAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		AppendComment("Description: " + EscapeString(attribute.description));
	}

	void WgslWriter::AppendAttribute(bool first, EarlyFragmentTestsAttribute attribute)
	{
		if (!attribute.HasValue() || !attribute.earlyFragmentTests.GetResultingValue())
			return;
		if (!first)
			Append(" ");
		Append("@early_depth_test(force)");
	}

	void WgslWriter::AppendAttribute(bool first, EntryAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		if (attribute.stageType.IsResultingValue())
		{
			switch (attribute.stageType.GetResultingValue())
			{
				case ShaderStageType::Compute: Append("compute"); break;
				case ShaderStageType::Fragment: Append("fragment"); break;
				case ShaderStageType::Vertex: Append("vertex"); break;
			}
		}
		else
			attribute.stageType.GetExpression()->Visit(*this);
	}

	void WgslWriter::AppendAttribute(bool /*first*/, FeatureAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		switch (attribute.featureAttribute)
		{
			case Ast::ModuleFeature::Float64:
			{
				if (!m_environment.featuresCallback || !m_environment.featuresCallback("WgpuFloat64"))
					throw std::runtime_error("WGSL does not support float64 feature, wgpu does natively but you need to confirm its usage using feature callback");
				break;
			}

			case Ast::ModuleFeature::PrimitiveExternals:
				throw std::runtime_error("primitive externals have no way to be translated in WGSL");
				break;

			case Ast::ModuleFeature::Texture1D:
				// Supported by WGSL
				break;
		}
	}

	void WgslWriter::AppendAttribute(bool first, InterpAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		Append("interp(");

		if (attribute.interpQualifier.IsResultingValue())
			Append(Parser::ToString(attribute.interpQualifier.GetResultingValue()));
		else
			attribute.interpQualifier.GetExpression()->Visit(*this);

		Append(")");
	}

	void WgslWriter::AppendAttribute(bool /*first*/, LayoutAttribute /*attribute*/)
	{
		// WGSL does not have memory layout management syntax
	}

	void WgslWriter::AppendAttribute(bool /*first*/, LicenseAttribute /*attribute*/)
	{
		// TODO
	}

	void WgslWriter::AppendAttribute(bool first, LocationAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		Append("location(");

		if (attribute.locationIndex.IsResultingValue())
			Append(attribute.locationIndex.GetResultingValue());
		else
			attribute.locationIndex.GetExpression()->Visit(*this);

		Append(")");
	}
	
	void WgslWriter::AppendAttribute(bool first, SetAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		Append("group(");

		if (attribute.setIndex.IsResultingValue())
			Append(attribute.setIndex.GetResultingValue());
		else
			attribute.setIndex.GetExpression()->Visit(*this);

		Append(")");
	}

	void WgslWriter::AppendAttribute(bool /*first*/, TagAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		AppendComment("Tag: " + attribute.tag);
	}

	void WgslWriter::AppendAttribute(bool first, UnrollAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		Append("unroll(");

		if (attribute.unroll.IsResultingValue())
			Append(Parser::ToString(attribute.unroll.GetResultingValue()));
		else
			attribute.unroll.GetExpression()->Visit(*this);

		Append(")");
	}

	void WgslWriter::AppendAttribute(bool first, WorkgroupAttribute attribute)
	{
		if (!attribute.HasValue())
			return;
		if (!first)
			Append(" ");
		Append("@");

		Append("workgroup_size(");

		if (attribute.workgroup.IsResultingValue())
		{
			const Vector3u32& workgroupSize = attribute.workgroup.GetResultingValue();
			Append(workgroupSize.x(), ", ", workgroupSize.y(), ", ", workgroupSize.z());
		}
		else
		{
			const Ast::ExpressionPtr& workgroupExpr = attribute.workgroup.GetExpression();
			if (workgroupExpr->GetType() != Ast::NodeType::CastExpression)
				throw std::runtime_error("expected workgroup expression to be a cast expression");

			const Ast::CastExpression& workgroupCast = static_cast<const Ast::CastExpression&>(*workgroupExpr);
			if (!workgroupCast.targetType.IsResultingValue() || workgroupCast.targetType.GetResultingValue() != Ast::ExpressionType{ Ast::VectorType{ 3, Ast::PrimitiveType::UInt32 }})
				throw std::runtime_error("expected workgroup expression to be a cast to vec3[u32]");

			if (workgroupCast.expressions.size() != 3)
				throw std::runtime_error("expected workgroup expression to be a cast of 3 expressions");

			workgroupCast.expressions[0]->Visit(*this);
			Append(", ");
			workgroupCast.expressions[1]->Visit(*this);
			Append(", ");
			workgroupCast.expressions[2]->Visit(*this);
		}

		Append(")");
	}

	void WgslWriter::AppendComment(std::string_view section)
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
			}
			while ((lineFeed = section.find('\n', previousCut)) != section.npos);
			AppendLine(section.substr(previousCut));
			AppendLine("*/");
		}
		else
			AppendLine("// ", section);
	}

	void WgslWriter::AppendCommentSection(std::string_view section)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		std::string stars((section.size() < 33) ? (36 - section.size()) / 2 : 3, '*');
		Append("/*", stars, ' ', section, ' ', stars, "*/");
		AppendLine();
	}

	void WgslWriter::AppendConstantHelpers(Ast::PrimitiveType type, Ast::TypeConstant constant)
	{
		using namespace std::string_view_literals;

		std::string_view stringType;
		switch (type)
		{
			case Ast::PrimitiveType::Float32: stringType = "f32"sv; break;
			case Ast::PrimitiveType::Float64: stringType = "f64"sv; break;

			default: return;
		}

		auto setupRatioFunction = [this, type, stringType]()
		{
			if (type == Ast::PrimitiveType::Float32)
			{
				if (m_currentState->hasf32RatioFunction)
					return;
				m_currentState->hasf32RatioFunction = true;
			}
			else if (type == Ast::PrimitiveType::Float64)
			{
				if (m_currentState->hasf64RatioFunction)
					return;
				m_currentState->hasf64RatioFunction = true;
			}

			Append(fmt::format(R"(fn _nzslRatio{0}(n: {0}, d: {0}) -> {0}
{{
	return n / d;	
}}

)", stringType));
		};

		switch (constant)
		{
			case Ast::TypeConstant::NaN:
			{
				setupRatioFunction();
				Append(fmt::format(R"(fn _nzslNaN{0}() -> {0}
{{
	return _nzslRatio{0}(0.0, 0.0);	
}}

)", stringType));
				break;
			}
			case Ast::TypeConstant::Infinity:
			{
				setupRatioFunction();
				Append(fmt::format(R"(fn _nzslInfinity{0}() -> {0}
{{
	return _nzslRatio{0}(1.0, 0.0);	
}}

)", stringType));
				break;
			}

			default: break;
		}
	}

	void WgslWriter::AppendLine(std::string_view txt)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (txt.empty() && m_currentState->streamEmptyLine > 1)
			return;

		m_currentState->stream << txt << '\n';
		m_currentState->streamEmptyLine++;
	}

	template<typename T>
	void WgslWriter::AppendIdentifier(const T& map, std::size_t id, bool append_module_prefix)
	{
		const auto& identifier = Nz::Retrieve(map, id);
		if (append_module_prefix && identifier.moduleIndex != 0)
			Append(m_currentState->moduleNames[identifier.moduleIndex - 1], '_');

		Append(identifier.name);
	}

	template<typename... Args>
	void WgslWriter::AppendLine(Args&&... params)
	{
		(Append(std::forward<Args>(params)), ...);
		AppendLine();
	}

	template<typename T>
	void WgslWriter::AppendValue(const T& value)
	{
		if constexpr (std::is_same_v<T, std::vector<bool>::reference>)
		{
			// fallback for std::vector<bool>
			bool v = value;
			return AppendValue(v);
		}
		else if constexpr (IsVector_v<T>)
		{
			std::string str = Ast::ConstantToString(value);
			std::replace(str.begin(), str.end(), '[', '<');
			std::replace(str.begin(), str.end(), ']', '>');
			Append(str);
		}
		else if constexpr (std::is_same_v<T, std::uint32_t>)
		{
			Append(Ast::ToString(value));
			Append("u");
		}
		else
			Append(Ast::ConstantToString(value));
	}

	void WgslWriter::AppendModuleAttributes(const Ast::Module::Metadata& metadata)
	{
		for (Ast::ModuleFeature feature : metadata.enabledFeatures)
			AppendAttributes(true, FeatureAttribute{ feature });

		AppendAttributes(true, AuthorAttribute{ metadata.author }, DescriptionAttribute{ metadata.description });
		AppendAttributes(true, LicenseAttribute{ metadata.license });
	}

	void WgslWriter::AppendStatementList(std::vector<Ast::StatementPtr>& statements)
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

	void WgslWriter::EnterScope()
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		AppendLine("{");
		m_currentState->indentLevel++;
	}

	void WgslWriter::LeaveScope(bool skipLine)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		m_currentState->indentLevel--;
		AppendLine();

		if (skipLine)
			AppendLine("}");
		else
			Append("}");
	}

	void WgslWriter::RegisterAlias(std::size_t aliasIndex, std::string aliasName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(aliasName);

		assert(m_currentState->aliases.find(aliasIndex) == m_currentState->aliases.end());
		m_currentState->aliases.emplace(aliasIndex, std::move(identifier));
	}

	void WgslWriter::RegisterConstant(std::size_t constantIndex, std::string constantName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(constantName);

		assert(m_currentState->constants.find(constantIndex) == m_currentState->constants.end());
		m_currentState->constants.emplace(constantIndex, std::move(identifier));
	}

	void WgslWriter::RegisterFunction(std::size_t funcIndex, std::string functionName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(functionName);

		assert(m_currentState->functions.find(funcIndex) == m_currentState->functions.end());
		m_currentState->functions.emplace(funcIndex, std::move(identifier));
	}

	void WgslWriter::RegisterModule(std::size_t moduleIndex, std::string moduleName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(moduleName);

		assert(m_currentState->modules.find(moduleIndex) == m_currentState->modules.end());
		m_currentState->modules.emplace(moduleIndex, std::move(identifier));
	}

	void WgslWriter::RegisterStruct(std::size_t structIndex, const Ast::StructDescription& structDescription)
	{
		State::StructData structData;
		structData.moduleIndex = m_currentState->currentModuleIndex;
		structData.name = structDescription.name;
		structData.desc = &structDescription;

		assert(m_currentState->structs.find(structIndex) == m_currentState->structs.end());
		m_currentState->structs.emplace(structIndex, std::move(structData));
	}

	void WgslWriter::RegisterVariable(std::size_t varIndex, std::string varName)
	{
		State::Identifier identifier;
		identifier.externalBlockIndex = m_currentState->currentExternalBlockIndex;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(varName);

		assert(m_currentState->variables.find(varIndex) == m_currentState->variables.end());
		m_currentState->variables.emplace(varIndex, std::move(identifier));
	}

	void WgslWriter::ScopeVisit(Ast::Statement& node)
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

	void WgslWriter::Visit(Ast::ExpressionPtr& expr, bool encloseIfRequired)
	{
		bool enclose = encloseIfRequired && (GetExpressionCategory(*expr) != Ast::ExpressionCategory::Variable);

		if (enclose)
			Append("(");

		expr->Visit(*this);

		if (enclose)
			Append(")");
	}

	void WgslWriter::Visit(Ast::AccessFieldExpression& node)
	{
		Visit(node.expr, true);

		const Ast::ExpressionType* exprType = GetExpressionType(*node.expr);
		NazaraUnused(exprType);
		assert(exprType);
		assert(IsStructAddressible(*exprType));

		std::size_t structIndex = Ast::ResolveStructIndex(*exprType);
		assert(structIndex != std::numeric_limits<std::size_t>::max());

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

	void WgslWriter::Visit(Ast::AccessIdentifierExpression& node)
	{
		Visit(node.expr, true);

		for (const auto& identifierEntry : node.identifiers)
			Append(".", identifierEntry.identifier);
	}

	void WgslWriter::Visit(Ast::AccessIndexExpression& node)
	{
		Visit(node.expr, true);

		// Array access
		Append("[");

		bool first = true;
		for (Ast::ExpressionPtr& expr : node.indices)
		{
			if (!first)
				Append(", ");

			expr->Visit(*this);
			first = false;
		}

		Append("]");
	}

	void WgslWriter::Visit(Ast::IdentifierValueExpression& node)
	{
		switch (node.identifierType)
		{
			case Ast::IdentifierType::Alias:         throw std::runtime_error("unexpected Alias identifier, shader is not properly resolved");
			case Ast::IdentifierType::Intrinsic:     throw std::runtime_error("unexpected Intrinsic identifier, shader is not properly resolved");
			case Ast::IdentifierType::Type:          throw std::runtime_error("unexpected Type identifier, shader is not properly resolved");
			case Ast::IdentifierType::Unresolved:    throw std::runtime_error("unexpected Unresolved identifier, shader is not properly resolved");

			case Ast::IdentifierType::ExternalBlock:
			{
				Append(m_currentState->externalBlockNames[node.identifierIndex]);
				break;
			}

			case Ast::IdentifierType::Module:
			{
				AppendIdentifier(m_currentState->modules, node.identifierIndex);
				break;
			}

			case Ast::IdentifierType::Struct:
			{
				AppendIdentifier(m_currentState->structs, node.identifierIndex, true);
				break;
			}

			case Ast::IdentifierType::Constant:
			{
				AppendIdentifier(m_currentState->constants, node.identifierIndex);
				break;
			}

			case Ast::IdentifierType::Function:
			{
				AppendIdentifier(m_currentState->functions, node.identifierIndex, true);
				break;
			}

			case Ast::IdentifierType::Variable:
			{
				AppendIdentifier(m_currentState->variables, node.identifierIndex);
				break;
			}
		}
	}

	void WgslWriter::Visit(Ast::AssignExpression& node)
	{
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

	void WgslWriter::Visit(Ast::BinaryExpression& node)
	{
		bool needsClosingCast = false;

		Visit(node.left, true);

		switch (node.op)
		{
			case Ast::BinaryType::Add:        Append(" + "); break;
			case Ast::BinaryType::Subtract:   Append(" - "); break;
			case Ast::BinaryType::Modulo:     Append(" % "); break;
			case Ast::BinaryType::Multiply:   Append(" * "); break;
			case Ast::BinaryType::Divide:     Append(" / "); break;

			case Ast::BinaryType::CompEq:     Append(" == "); break;
			case Ast::BinaryType::CompGe:     Append(" >= "); break;
			case Ast::BinaryType::CompGt:     Append(" > ");  break;
			case Ast::BinaryType::CompLe:     Append(" <= "); break;
			case Ast::BinaryType::CompLt:     Append(" < ");  break;
			case Ast::BinaryType::CompNe:     Append(" != "); break;

			case Ast::BinaryType::LogicalAnd: Append(" && "); break;
			case Ast::BinaryType::LogicalOr:  Append(" || "); break;
			
			case Ast::BinaryType::BitwiseAnd:  Append(" & ");  break;
			case Ast::BinaryType::BitwiseOr:   Append(" | ");  break;
			case Ast::BinaryType::BitwiseXor:  Append(" ^ ");  break;
			case Ast::BinaryType::ShiftLeft:   Append(" << "); break;
			case Ast::BinaryType::ShiftRight:  Append(" >> "); break;
		}

		if (node.op == Ast::BinaryType::ShiftLeft || node.op == Ast::BinaryType::ShiftRight)
		{
			const Ast::ExpressionType& rightType = Ast::ResolveAlias(Ast::EnsureExpressionType(*node.right));

			if (Ast::IsVectorType(rightType))
			{
				Ast::VectorType vectorType = std::get<Ast::VectorType>(rightType);
				if (vectorType.type == Ast::PrimitiveType::Int32)
				{
					Append("vec");
					Append(std::to_string(vectorType.componentCount));
					Append("<u32>(");
					needsClosingCast = true;
				}
			}
			else if (Ast::IsPrimitiveType(rightType) && std::get<Ast::PrimitiveType>(rightType) == Ast::PrimitiveType::Int32)
			{
				Append("u32(");
				needsClosingCast = true;
			}
		}
		Visit(node.right, true);
		if (needsClosingCast)
			Append(")");
	}

	void WgslWriter::Visit(Ast::CallFunctionExpression& node)
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

	void WgslWriter::Visit(Ast::CastExpression& node)
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

	void WgslWriter::Visit(Ast::ConditionalExpression& /*node*/)
	{
		throw std::runtime_error("unexpected conditional expression, is shader sanitized?");
	}

	void WgslWriter::Visit(Ast::ConstantArrayValueExpression& node)
	{
		Append(*node.cachedExpressionType);
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

	void WgslWriter::Visit(Ast::ConstantValueExpression& node)
	{
		std::visit([&](auto&& arg)
		{
			AppendValue(arg);
		}, node.value);
	}

	void WgslWriter::Visit(Ast::IdentifierExpression& node)
	{
		Append(node.identifier);
	}
	
	void WgslWriter::Visit(Ast::IntrinsicExpression& node)
	{
		bool method = false;
		bool firstParam = true;
		switch (node.intrinsic)
		{
			// Function intrinsics
			case Ast::IntrinsicType::Abs:
			case Ast::IntrinsicType::All:
			case Ast::IntrinsicType::Any:
			case Ast::IntrinsicType::ArcCos:
			case Ast::IntrinsicType::ArcCosh:
			case Ast::IntrinsicType::ArcSin:
			case Ast::IntrinsicType::ArcSinh:
			case Ast::IntrinsicType::ArcTan:
			case Ast::IntrinsicType::ArcTan2:
			case Ast::IntrinsicType::ArcTanh:
			case Ast::IntrinsicType::Ceil:
			case Ast::IntrinsicType::Clamp:
			case Ast::IntrinsicType::Cos:
			case Ast::IntrinsicType::Cosh:
			case Ast::IntrinsicType::CrossProduct:
			case Ast::IntrinsicType::Distance:
			case Ast::IntrinsicType::DotProduct:
			case Ast::IntrinsicType::Exp:
			case Ast::IntrinsicType::Exp2:
			case Ast::IntrinsicType::Floor:
			case Ast::IntrinsicType::Fract:
			case Ast::IntrinsicType::Length:
			case Ast::IntrinsicType::Log:
			case Ast::IntrinsicType::Log2:
			case Ast::IntrinsicType::MatrixInverse:
			case Ast::IntrinsicType::MatrixTranspose:
			case Ast::IntrinsicType::Max:
			case Ast::IntrinsicType::Min:
			case Ast::IntrinsicType::Normalize:
			case Ast::IntrinsicType::Pow:
			case Ast::IntrinsicType::Reflect:
			case Ast::IntrinsicType::Round:
			case Ast::IntrinsicType::Sign:
			case Ast::IntrinsicType::Sin:
			case Ast::IntrinsicType::Sinh:
			case Ast::IntrinsicType::SmoothStep:
			case Ast::IntrinsicType::Sqrt:
			case Ast::IntrinsicType::Step:
			case Ast::IntrinsicType::Tan:
			case Ast::IntrinsicType::Tanh:
			case Ast::IntrinsicType::Trunc:
			{
				auto intrinsicIt = LangData::s_intrinsicData.find(node.intrinsic);
				assert(intrinsicIt != LangData::s_intrinsicData.end());
				assert(!intrinsicIt->second.functionName.empty());

				Append(intrinsicIt->second.functionName);
				break;
			}

			case Ast::IntrinsicType::DegToRad: Append("radians"); break;
			case Ast::IntrinsicType::InverseSqrt: Append("inverseSqrt"); break;

			case Ast::IntrinsicType::IsInf:
			case Ast::IntrinsicType::IsNaN:
			{
				const Ast::ExpressionType& paramType = ResolveAlias(EnsureExpressionType(*node.parameters[0]));
				const Ast::PrimitiveType& innerType = IsVectorType(paramType) ? std::get<Ast::VectorType>(paramType).type : std::get<Ast::PrimitiveType>(paramType);
				std::size_t componentCount = 1;
				if (node.intrinsic == Ast::IntrinsicType::IsInf && IsVectorType(paramType))
				{
					componentCount = std::get<Ast::VectorType>(paramType).componentCount;
					Append("vec", componentCount, "<bool>(");
				}
				for(std::size_t i = 0; i < componentCount; i++)
				{
					if (i != 0)
						Append(", ");
					if (node.intrinsic == Ast::IntrinsicType::IsInf)
					{
						if (IsVectorType(paramType))
						{
							const char* componentStr = "xyzw";
							node.parameters[0]->Visit(*this);
							Append('.', componentStr[i]);
						}
						Append(" == _nzslInfinity", (innerType == Ast::PrimitiveType::Float32 ? "f32" : "f64"), "()");
					}
					else
					{
						node.parameters[0]->Visit(*this);
						Append(" != ");
						node.parameters[0]->Visit(*this);
						return;
					}
				}
				if (IsVectorType(paramType))
					Append(")");
				return;
			}

			case Ast::IntrinsicType::Lerp: Append("mix"); break;
			case Ast::IntrinsicType::Not: Append("!"); break;
			case Ast::IntrinsicType::RadToDeg: Append("degrees"); break;
			case Ast::IntrinsicType::RoundEven: Append("round"); break;

			case Ast::IntrinsicType::Select:
			{
				const Ast::ExpressionType& condParamType = ResolveAlias(EnsureExpressionType(*node.parameters[0]));
				const Ast::ExpressionType& firstParamType = ResolveAlias(EnsureExpressionType(*node.parameters[1]));

				Append("select(");
				node.parameters[2]->Visit(*this);
				Append(", ");
				node.parameters[1]->Visit(*this);
				Append(", ");

				// WGSL requires boolean vectors when selecting vectors
				if (IsVectorType(firstParamType) && !IsVectorType(condParamType))
				{
					std::size_t componentCount = std::get<Ast::VectorType>(firstParamType).componentCount;

					Append("vec", componentCount, "<bool>(");
					node.parameters[0]->Visit(*this);
					Append(")");
				}
				else
					node.parameters[0]->Visit(*this);

				Append(")");
				return;
			}

			case Ast::IntrinsicType::TextureRead: Append("textureLoad"); break;
			case Ast::IntrinsicType::TextureWrite: Append("textureStore"); break;

			// Method intrinsics
			case Ast::IntrinsicType::ArraySize:
				assert(!node.parameters.empty());
				firstParam = false;
				if (node.parameters[0]->cachedExpressionType.has_value())
				{
					auto value = node.parameters[0]->cachedExpressionType.value();
					if (IsArrayType(value) && std::get<Ast::ArrayType>(value).length > 0)
					{
						Append(std::get<Ast::ArrayType>(value).length);
						return;
					}
				}
				Append("arrayLength(&");
				node.parameters[0]->Visit(*this);
				method = true;
				break;

			case Ast::IntrinsicType::TextureSampleImplicitLod:
				firstParam = false;
				Append("textureSample(");
				node.parameters[0]->Visit(*this);
				Append(", ");
				node.parameters[0]->Visit(*this);
				Append("Sampler, ");
				method = true;
				break;

			case Ast::IntrinsicType::TextureSampleImplicitLodDepthComp:
				firstParam = false;
				Append("textureSampleCompare(");
				node.parameters[0]->Visit(*this);
				Append(", ");
				node.parameters[0]->Visit(*this);
				Append("Sampler, ");
				method = true;
				break;
		}

		if (firstParam)
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
	}

	void WgslWriter::Visit(Ast::SwizzleExpression& node)
	{
		Visit(node.expression, true);
		Append(".");

		const char* componentStr = "xyzw";
		for (std::size_t i = 0; i < node.componentCount; ++i)
			Append(componentStr[node.components[i]]);
	}

	void WgslWriter::Visit(Ast::UnaryExpression& node)
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
				break;
		}

		node.expression->Visit(*this);
	}

	void WgslWriter::Visit(Ast::BranchStatement& node)
	{
		bool first = true;
		for (const auto& statement : node.condStatements)
		{
			if (first)
			{
				if (node.isConst)
					Append("const ");
			}
			else
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

	void WgslWriter::Visit(Ast::BreakStatement& /*node*/)
	{
		Append("break;");
	}

	void WgslWriter::Visit(Ast::ConditionalStatement& node)
	{
		Append("[cond(");
		node.condition->Visit(*this);
		AppendLine(")]");
		node.statement->Visit(*this);
	}

	void WgslWriter::Visit(Ast::ContinueStatement& /*node*/)
	{
		Append("continue;");
	}

	void WgslWriter::Visit(Ast::DeclareAliasStatement& /*node*/)
	{
		// all aliases should have been handled by sanitizer
		throw std::runtime_error("unexpected alias declaration, is shader sanitized?");
	}

	void WgslWriter::Visit(Ast::DeclareConstStatement& node)
	{
		if (node.constIndex)
			RegisterConstant(*node.constIndex, node.name);

		Append("const ", node.name);
		if (node.type.HasValue())
			Append(": ", node.type);

		if (node.expression)
		{
			Append(" = ");
			node.expression->Visit(*this);
		}

		AppendLine(";");
	}

	void WgslWriter::Visit(Ast::DeclareExternalStatement& node)
	{
		AppendAttributes(true, TagAttribute{ node.tag });

		if (!node.name.empty())
		{
			m_currentState->currentExternalBlockIndex = m_currentState->externalBlockNames.size();
			m_currentState->externalBlockNames.push_back(node.name);
		}

		AppendLine();

		std::unordered_set<std::uint64_t> reservedBindings;

		for (const auto& externalVar : node.externalVars)
		{
			if (!externalVar.tag.empty() && m_currentState->backendParameters.debugLevel >= DebugLevel::Minimal)
				AppendAttribute(false, TagAttribute{ externalVar.tag });

			const Ast::ExpressionType& exprType = externalVar.type.GetResultingValue();

			std::uint32_t binding = 0;
			std::uint64_t bindingSet = (externalVar.bindingSet.HasValue()) ? externalVar.bindingSet.GetResultingValue() : 0;

			// Binding group declaration in WGSL in built like this
			// @group(G) @binding(B) var<ADDRESS_SPACE[, ACCESS?]> name : TYPE;

			// Binding group handling
			if (!IsPushConstantType(exprType)) // Push constants don't have set or binding
			{
				binding = externalVar.bindingIndex.GetResultingValue();
				for (; reservedBindings.count(bindingSet << 32 | binding); binding++);
				reservedBindings.emplace(bindingSet << 32 | binding);
				m_currentState->bindingRemap[bindingSet << 32 | externalVar.bindingIndex.GetResultingValue()] = binding;

				AppendAttributes(false, SetAttribute{ externalVar.bindingSet }, BindingAttribute{ Ast::ExpressionValue{ binding } });
			}

			Append("var");

			// Address space handling
			if (IsUniformType(exprType))
				Append("<uniform>");
			else if (IsPushConstantType(exprType))
				Append("<push_constant>");
			else if (IsStorageType(exprType))
			{
				const Ast::StorageType& storageType = std::get<Ast::StorageType>(exprType);

				Append("<storage, ");
				switch (storageType.accessPolicy)
				{
					case AccessPolicy::ReadOnly:  Append("read"); break;

					case AccessPolicy::WriteOnly: // WGSL does not support write only storage bindings so readwrite will do just fine
					case AccessPolicy::ReadWrite: Append("read_write"); break;
				}
				Append(">");
			}

			Append(' ');

			std::string variableName;

			if (m_currentState->currentModuleIndex != 0)
				variableName += m_currentState->moduleNames[m_currentState->currentModuleIndex - 1] + '_';
			if (!node.name.empty())
				variableName += node.name + '_';
			variableName += externalVar.name;
			Append(variableName, ": ", exprType);

			if (IsSamplerType(exprType))
			{
				// WGSL has not (yet?) combined image samplers so we need to split textures and samplers
				AppendLine(';'); // Closing last line
				AppendAttributes(false, SetAttribute{ externalVar.bindingSet }, BindingAttribute{ Ast::ExpressionValue{ binding + 1 } });
				reservedBindings.emplace(bindingSet << 32 | binding + 1);
				Append("var ", variableName, "Sampler: sampler");
			}

			AppendLine(';');

			if (externalVar.varIndex)
				RegisterVariable(*externalVar.varIndex, variableName);
		}

		m_currentState->currentExternalBlockIndex = {};
	}

	void WgslWriter::Visit(Ast::DeclareFunctionStatement& node)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		AppendAttributes(true, 
			EntryAttribute{ node.entryStage },
			WorkgroupAttribute{ node.workgroupSize },
			EarlyFragmentTestsAttribute{ node.earlyFragmentTests },
			DepthWriteAttribute{ node.depthWrite }
		);

		Append("fn ");

		assert(node.funcIndex);
		const auto& identifier = Nz::Retrieve(m_currentState->functions, *node.funcIndex);
		if (identifier.moduleIndex != 0)
			Append(m_currentState->moduleNames[identifier.moduleIndex - 1], '_', node.name, '(');
		else
			Append(node.name, '(');
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			const auto& parameter = node.parameters[i];

			if (i != 0)
				Append(", ");

			if (parameter.semantic == Ast::FunctionParameterSemantic::InOut)
				Append("inout ");
			else if (parameter.semantic == Ast::FunctionParameterSemantic::Out)
				Append("out ");

			Append(parameter.name, ": ", parameter.type);

			if (parameter.varIndex)
				RegisterVariable(*parameter.varIndex, parameter.name);
		}
		Append(')');
		if (node.returnType.HasValue())
		{
			if (!node.returnType.IsResultingValue() || !IsNoType(node.returnType.GetResultingValue()))
				Append(" -> ", node.returnType);
		}

		AppendLine();
		EnterScope();
		{
			AppendStatementList(node.statements);
		}
		LeaveScope();
	}

	void WgslWriter::Visit(Ast::DeclareOptionStatement& /*node*/)
	{
		// all options should have been handled by sanitizer
		throw std::runtime_error("unexpected option declaration, is shader sanitized?");
	}

	void WgslWriter::Visit(Ast::DeclareStructStatement& node)
	{
		if (node.structIndex)
			RegisterStruct(*node.structIndex, node.description);

		AppendAttributes(true, LayoutAttribute{ node.description.layout }, TagAttribute{ node.description.tag });
		Append("struct ");

		assert(node.structIndex);
		const auto& identifier = Nz::Retrieve(m_currentState->structs, *node.structIndex);
		if (identifier.moduleIndex != 0)
			AppendLine(m_currentState->moduleNames[identifier.moduleIndex - 1], '_', node.description.name);
		else
			AppendLine(node.description.name);
		EnterScope();
		{
			bool first = true;
			for (const auto& member : node.description.members)
			{
				if (!first)
					AppendLine(",");

				first = false;

				AppendAttributes(false, CondAttribute{ member.cond }, LocationAttribute{ member.locationIndex }, InterpAttribute{ member.interp }, BuiltinAttribute{ member.builtin }, TagAttribute{ member.tag });
				Append(member.name, ": ");
				if (member.builtin.HasValue() && member.builtin.GetResultingValue() == Ast::BuiltinEntry::VertexIndex)
					Append("u32");
				else
					Append(member.type);
			}
		}
		LeaveScope();
	}

	void WgslWriter::Visit(Ast::DeclareVariableStatement& node)
	{
		if (node.varIndex)
			RegisterVariable(*node.varIndex, node.varName);

		Append("var ");
		Append(node.varName);
		if (node.varType.HasValue())
			Append(": ", node.varType);

		if (node.initialExpression)
		{
			Append(" = ");
			node.initialExpression->Visit(*this);
		}

		Append(";");
	}

	void WgslWriter::Visit(Ast::DiscardStatement& /*node*/)
	{
		Append("discard;");
	}

	void WgslWriter::Visit(Ast::ExpressionStatement& node)
	{
		node.expression->Visit(*this);
		Append(";");
	}

	void WgslWriter::Visit(Ast::ForStatement& /*node*/)
	{
		// For loops must have been converted to while loop in prepasses
		throw std::runtime_error("unexpected for statement, is the shader sanitized?");
	}

	void WgslWriter::Visit(Ast::ForEachStatement& node)
	{
		if (node.varIndex)
			RegisterVariable(*node.varIndex, node.varName);

		AppendAttributes(true, UnrollAttribute{ node.unroll });
		Append("for ", node.varName, " in ");
		node.expression->Visit(*this);
		AppendLine();

		ScopeVisit(*node.statement);
	}

	void WgslWriter::Visit(Ast::ImportStatement& node)
	{
		Append("import ");
		
		if (node.identifiers.empty())
		{
			// Whole module import
			Append(node.moduleName);

			std::string_view defaultIdentifierName;
			std::size_t lastSep = node.moduleName.find_last_of('.');
			if (lastSep != std::string::npos)
				defaultIdentifierName = std::string_view(node.moduleName).substr(lastSep + 1);
			else
				defaultIdentifierName = node.moduleName;

			if (node.moduleIdentifier != node.moduleName)
				Append(" as ", node.moduleIdentifier);

			AppendLine(";");
		}
		else
		{
			// Module identifier import
			bool first = true;
			for (const auto& entry : node.identifiers)
			{
				if (!first)
					Append(", ");

				first = false;

				if (!entry.identifier.empty())
				{
					Append(entry.identifier);
					if (!entry.renamedIdentifier.empty())
						Append(" as ", entry.renamedIdentifier);
				}
				else
					Append("*");
			}
			AppendLine(" from ", node.moduleName, ";");
		}
	}

	void WgslWriter::Visit(Ast::MultiStatement& node)
	{
		AppendStatementList(node.statements);
	}

	void WgslWriter::Visit(Ast::NoOpStatement& /*node*/)
	{
		/* nothing to do */
	}

	void WgslWriter::Visit(Ast::ReturnStatement& node)
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

	void WgslWriter::Visit(Ast::ScopedStatement& node)
	{
		EnterScope();
		node.statement->Visit(*this);
		LeaveScope();
	}

	void WgslWriter::Visit(Ast::WhileStatement& node)
	{
		Append("while (");
		node.condition->Visit(*this);
		AppendLine(")");

		ScopeVisit(*node.body);
	}

	void WgslWriter::AppendHeader(const Ast::Module::Metadata& metadata)
	{
		AppendComment("This file was generated by NZSL compiler (Nazara Shading Language)");
		AppendModuleAttributes(metadata);
		AppendLine();
	}
}
