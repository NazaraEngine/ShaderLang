// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/LangWriter.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Lexer.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Constants.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Lang/Version.hpp>
#include <cassert>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace nzsl
{
	struct LangWriter::PreVisitor : Ast::RecursiveVisitor
	{
		PreVisitor(LangWriter& writer) :
		m_writer(writer)
		{
		}

		void Visit(Ast::DeclareFunctionStatement& node) override
		{
			if (node.funcIndex)
				m_writer.RegisterFunction(*node.funcIndex, node.name);

			// Speed up by not visiting function statements, we only need to extract function data
		}

		LangWriter& m_writer;
	};

	struct LangWriter::AutoBindingAttribute
	{
		const Ast::ExpressionValue<bool>& autoBinding;

		bool HasValue() const { return autoBinding.HasValue(); }
	};

	struct LangWriter::AuthorAttribute
	{
		const std::string& author;

		bool HasValue() const { return !author.empty(); }
	};

	struct LangWriter::BindingAttribute
	{
		const Ast::ExpressionValue<std::uint32_t>& bindingIndex;

		bool HasValue() const { return bindingIndex.HasValue(); }
	};

	struct LangWriter::BuiltinAttribute
	{
		const Ast::ExpressionValue<Ast::BuiltinEntry>& builtin;

		bool HasValue() const { return builtin.HasValue(); }
	};

	struct LangWriter::CondAttribute
	{
		const Ast::ExpressionValue<bool>& cond;

		bool HasValue() const { return cond.HasValue(); }
	};

	struct LangWriter::DepthWriteAttribute
	{
		const Ast::ExpressionValue<Ast::DepthWriteMode>& writeMode;

		bool HasValue() const { return writeMode.HasValue(); }
	};

	struct LangWriter::DescriptionAttribute
	{
		const std::string& description;

		bool HasValue() const { return !description.empty(); }
	};

	struct LangWriter::EarlyFragmentTestsAttribute
	{
		const Ast::ExpressionValue<bool>& earlyFragmentTests;

		bool HasValue() const { return earlyFragmentTests.HasValue(); }
	};

	struct LangWriter::EntryAttribute
	{
		const Ast::ExpressionValue<ShaderStageType>& stageType;

		bool HasValue() const { return stageType.HasValue(); }
	};

	struct LangWriter::FeatureAttribute
	{
		Ast::ModuleFeature featureAttribute;

		bool HasValue() const { return true; }
	};

	struct LangWriter::InterpAttribute
	{
		const Ast::ExpressionValue<Ast::InterpolationQualifier>& interpQualifier;

		bool HasValue() const { return interpQualifier.HasValue(); }
	};

	struct LangWriter::LangVersionAttribute
	{
		std::uint32_t version;

		bool HasValue() const { return true; }
	};

	struct LangWriter::LayoutAttribute
	{
		const Ast::ExpressionValue<Ast::MemoryLayout>& layout;

		bool HasValue() const { return layout.HasValue(); }
	};

	struct LangWriter::LicenseAttribute
	{
		const std::string& license;

		bool HasValue() const { return !license.empty(); }
	};

	struct LangWriter::LocationAttribute
	{
		const Ast::ExpressionValue<std::uint32_t>& locationIndex;

		bool HasValue() const { return locationIndex.HasValue(); }
	};

	struct LangWriter::SetAttribute
	{
		const Ast::ExpressionValue<std::uint32_t>& setIndex;

		bool HasValue() const { return setIndex.HasValue(); }
	};

	struct LangWriter::TagAttribute
	{
		const std::string& tag;

		bool HasValue() const { return !tag.empty(); }
	};

	struct LangWriter::UnrollAttribute
	{
		const Ast::ExpressionValue<Ast::LoopUnroll>& unroll;

		bool HasValue() const { return unroll.HasValue(); }
	};

	struct LangWriter::WorkgroupAttribute
	{
		const Ast::ExpressionValue<Vector3u32>& workgroup;

		bool HasValue() const { return workgroup.HasValue(); }
	};

	struct LangWriter::State
	{
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
		std::vector<std::string> externalBlockNames;
		std::vector<std::string> moduleNames;
		const Ast::Module* currentModule;
		bool enforceNonDefaultTypes = false;
		bool isInEntryPoint = false;
		int streamEmptyLine = 1;
		unsigned int indentLevel = 0;
	};

	std::string LangWriter::Generate(const Ast::Module& module)
	{
		State state;
		m_currentState = &state;
		NAZARA_DEFER({ m_currentState = nullptr; });

		AppendHeader(module);

		// First registration pass (required to register function names)
		PreVisitor previsitor(*this);
		{
			m_currentState->currentModuleIndex = 0;
			for (const auto& importedModule : module.importedModules)
			{
				importedModule.module->rootNode->Visit(previsitor);
				m_currentState->currentModuleIndex++;
				m_currentState->moduleNames.push_back(importedModule.identifier);
			}

			m_currentState->currentModuleIndex = std::numeric_limits<std::size_t>::max();

			std::size_t moduleIndex = 0;
			for (const auto& importedModule : module.importedModules)
				RegisterModule(moduleIndex++, importedModule.identifier);

			module.rootNode->Visit(previsitor);
		}

		// Register imported modules
		m_currentState->currentModuleIndex = 0;
		for (const auto& importedModule : module.importedModules)
		{
			m_currentState->currentModule = importedModule.module.get();

			AppendModuleAttributes(*importedModule.module->metadata);
			AppendLine("module ", importedModule.identifier);
			EnterScope();
			importedModule.module->rootNode->Visit(*this);
			LeaveScope(true);

			m_currentState->currentModuleIndex++;
			m_currentState->moduleNames.push_back(importedModule.identifier);
		}

		m_currentState->currentModule = &module;
		m_currentState->currentModuleIndex = std::numeric_limits<std::size_t>::max();
		module.rootNode->Visit(*this);

		return state.stream.str();
	}

	void LangWriter::SetEnv(Environment environment)
	{
		m_environment = std::move(environment);
	}

	void LangWriter::Append(const Ast::AliasType& type)
	{
		AppendIdentifier(m_currentState->aliases, type.aliasIndex);
	}

	void LangWriter::Append(const Ast::ArrayType& type)
	{
		if (type.length > 0)
			Append("array[", type.containedType->type, ", ", type.length, "]");
		else
			Append("array[", type.containedType->type, "]");
	}

	void LangWriter::Append(const Ast::DynArrayType& type)
	{
		Append("dyn_array[", type.containedType->type, "]");
	}

	void LangWriter::Append(const Ast::ExpressionType& type)
	{
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, type);
	}

	void LangWriter::Append(const Ast::ExpressionValue<Ast::ExpressionType>& type)
	{
		assert(type.HasValue());
		if (type.IsResultingValue())
			Append(type.GetResultingValue());
		else
			type.GetExpression()->Visit(*this);
	}

	void LangWriter::Append(const Ast::FunctionType& /*functionType*/)
	{
		throw std::runtime_error("unexpected function type");
	}

	void LangWriter::Append(const Ast::IntrinsicFunctionType& /*functionType*/)
	{
		throw std::runtime_error("unexpected intrinsic function type");
	}

	void LangWriter::Append(const Ast::MatrixType& matrixType)
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

		Append("[", matrixType.type, "]");
	}

	void LangWriter::Append(const Ast::MethodType& /*functionType*/)
	{
		throw std::runtime_error("unexpected method type");
	}

	void LangWriter::Append(const Ast::ModuleType& moduleType)
	{
		AppendIdentifier(m_currentState->modules, moduleType.moduleIndex);
	}

	void LangWriter::Append(const Ast::NamedExternalBlockType& namedExternalBlockType)
	{
		Append(m_currentState->externalBlockNames[namedExternalBlockType.namedExternalBlockIndex]);
	}

	void LangWriter::Append(Ast::PrimitiveType type)
	{
		switch (type)
		{
			case Ast::PrimitiveType::Boolean:      return Append("bool");
			case Ast::PrimitiveType::Float32:      return Append("f32");
			case Ast::PrimitiveType::Float64:      return Append("f64");
			case Ast::PrimitiveType::Int32:        return Append("i32");
			case Ast::PrimitiveType::UInt32:       return Append("u32");
			case Ast::PrimitiveType::String:       return Append("string");
			case Ast::PrimitiveType::FloatLiteral: return Append("FloatLiteral");
			case Ast::PrimitiveType::IntLiteral:   return Append("IntLiteral");
		}
	}

	void LangWriter::Append(const Ast::PushConstantType& pushConstantType)
	{
		Append("push_constant[", pushConstantType.containedType, "]");
	}

	void LangWriter::Append(const Ast::SamplerType& samplerType)
	{
		if (samplerType.depth)
			Append("depth_");

		Append("sampler");

		switch (samplerType.dim)
		{
			case ImageType::E1D:       Append("1D");      break;
			case ImageType::E1D_Array: Append("1D_array"); break;
			case ImageType::E2D:       Append("2D");      break;
			case ImageType::E2D_Array: Append("2D_array"); break;
			case ImageType::E3D:       Append("3D");      break;
			case ImageType::Cubemap:   Append("_cube");    break;
		}

		Append("[", samplerType.sampledType, "]");
	}

	void LangWriter::Append(const Ast::StorageType& storageType)
	{
		Append("storage[", storageType.containedType);
		switch (storageType.accessPolicy)
		{
			case AccessPolicy::ReadOnly:  Append(", readonly"); break;
			case AccessPolicy::ReadWrite: break;
			case AccessPolicy::WriteOnly: Append(", writeonly"); break;
		}
		Append("]");
	}

	void LangWriter::Append(const Ast::StructType& structType)
	{
		AppendIdentifier(m_currentState->structs, structType.structIndex);
	}

	void LangWriter::Append(const Ast::TextureType& textureType)
	{
		Append("texture");

		switch (textureType.dim)
		{
			case ImageType::E1D:       Append("1D");       break;
			case ImageType::E1D_Array: Append("1D_array"); break;
			case ImageType::E2D:       Append("2D");       break;
			case ImageType::E2D_Array: Append("2D_array"); break;
			case ImageType::E3D:       Append("3D");       break;
			case ImageType::Cubemap:   Append("_cube");    break;
		}

		Append("[", textureType.baseType, ", ");
		switch (textureType.accessPolicy)
		{
			case AccessPolicy::ReadOnly:  Append("readonly"); break;
			case AccessPolicy::ReadWrite: Append("readwrite"); break;
			case AccessPolicy::WriteOnly: Append("writeonly"); break;
		}

		if (textureType.format != ImageFormat::Unknown)
		{
			assert(textureType.format == ImageFormat::RGBA8); //< TODO
			Append(", rgba8");
		}
		Append("]");
	}

	void LangWriter::Append(const Ast::Type& /*type*/)
	{
		throw std::runtime_error("unexpected type?");
	}

	void LangWriter::Append(const Ast::UniformType& uniformType)
	{
		Append("uniform[", uniformType.containedType, "]");
	}

	void LangWriter::Append(const Ast::VectorType& vecType)
	{
		Append("vec", vecType.componentCount, "[", vecType.type, "]");
	}

	void LangWriter::Append(Ast::NoType)
	{
		return Append("()");
	}

	template<typename T>
	void LangWriter::Append(const T& param)
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
	void LangWriter::Append(const T1& firstParam, const T2& secondParam, Args&&... params)
	{
		Append(firstParam);
		Append(secondParam, std::forward<Args>(params)...);
	}

	template<typename... Args>
	void LangWriter::AppendAttributes(bool appendLine, Args&&... params)
	{
		bool hasAnyAttribute = (params.HasValue() || ...);
		if (!hasAnyAttribute)
			return;

		bool first = true;

		Append("[");
		AppendAttributesInternal(first, std::forward<Args>(params)...);
		Append("]");

		if (appendLine)
			AppendLine();
		else
			Append(" ");
	}

	template<typename T>
	void LangWriter::AppendAttributesInternal(bool& first, const T& param)
	{
		if (!param.HasValue())
			return;

		if (!first)
			Append(", ");

		first = false;

		AppendAttribute(param);
	}

	template<typename T1, typename T2, typename... Rest>
	void LangWriter::AppendAttributesInternal(bool& first, const T1& firstParam, const T2& secondParam, Rest&&... params)
	{
		AppendAttributesInternal(first, firstParam);
		AppendAttributesInternal(first, secondParam, std::forward<Rest>(params)...);
	}

	void LangWriter::AppendAttribute(AutoBindingAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("auto_binding(");

		if (attribute.autoBinding.IsResultingValue())
			Append((attribute.autoBinding.GetResultingValue()) ? "true" : "false");
		else
			attribute.autoBinding.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(AuthorAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("author(", EscapeString(attribute.author), ")");
	}

	void LangWriter::AppendAttribute(BindingAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("binding(");

		if (attribute.bindingIndex.IsResultingValue())
			Append(attribute.bindingIndex.GetResultingValue());
		else
			attribute.bindingIndex.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(BuiltinAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("builtin(");

		if (attribute.builtin.IsResultingValue())
			Append(Parser::ToString(attribute.builtin.GetResultingValue()));
		else
			attribute.builtin.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(CondAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("cond(");

		if (attribute.cond.IsResultingValue())
			Append(Ast::ToString(attribute.cond.GetResultingValue()));
		else
			attribute.cond.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(DepthWriteAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("depth_write(");

		if (attribute.writeMode.IsResultingValue())
			Append(Parser::ToString(attribute.writeMode.GetResultingValue()));
		else
			attribute.writeMode.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(DescriptionAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("desc(", EscapeString(attribute.description), ")");
	}

	void LangWriter::AppendAttribute(EarlyFragmentTestsAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("early_fragment_tests(");

		if (attribute.earlyFragmentTests.IsResultingValue())
			Append((attribute.earlyFragmentTests.GetResultingValue()) ? "true" : "false");
		else
			attribute.earlyFragmentTests.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(EntryAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("entry(");

		if (attribute.stageType.IsResultingValue())
			Append(Parser::ToString(attribute.stageType.GetResultingValue()));
		else
			attribute.stageType.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(FeatureAttribute attribute)
	{
		Append("feature(");

		auto it = LangData::s_moduleFeatures.find(attribute.featureAttribute);
		assert(it != LangData::s_moduleFeatures.end());

		Append(it->second.identifier);

		Append(")");
	}

	void LangWriter::AppendAttribute(InterpAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("interp(");

		if (attribute.interpQualifier.IsResultingValue())
			Append(Parser::ToString(attribute.interpQualifier.GetResultingValue()));
		else
			attribute.interpQualifier.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(LangVersionAttribute attribute)
	{
		// nzsl_version
		Append("nzsl_version(\"", Version::ToString(attribute.version), "\")");
	}

	void LangWriter::AppendAttribute(LayoutAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("layout(");
		if (attribute.layout.IsResultingValue())
			Append(Parser::ToString(attribute.layout.GetResultingValue()));
		else
			attribute.layout.GetExpression()->Visit(*this);
		Append(")");
	}

	void LangWriter::AppendAttribute(LicenseAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("license(", EscapeString(attribute.license), ")");
	}

	void LangWriter::AppendAttribute(LocationAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("location(");

		if (attribute.locationIndex.IsResultingValue())
			Append(attribute.locationIndex.GetResultingValue());
		else
			attribute.locationIndex.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(SetAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("set(");

		if (attribute.setIndex.IsResultingValue())
			Append(attribute.setIndex.GetResultingValue());
		else
			attribute.setIndex.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(TagAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("tag(", EscapeString(attribute.tag), ")");
	}

	void LangWriter::AppendAttribute(UnrollAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("unroll(");

		if (attribute.unroll.IsResultingValue())
			Append(Parser::ToString(attribute.unroll.GetResultingValue()));
		else
			attribute.unroll.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(WorkgroupAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("workgroup(");

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

	void LangWriter::AppendComment(std::string_view section)
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

	void LangWriter::AppendCommentSection(std::string_view section)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		std::string stars((section.size() < 33) ? (36 - section.size()) / 2 : 3, '*');
		Append("/*", stars, ' ', section, ' ', stars, "*/");
		AppendLine();
	}

	void LangWriter::AppendLine(std::string_view txt)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (txt.empty() && m_currentState->streamEmptyLine > 1)
			return;

		m_currentState->stream << txt << '\n';
		m_currentState->streamEmptyLine++;
	}

	template<typename T>
	void LangWriter::AppendIdentifier(const T& map, std::size_t id)
	{
		const auto& identifier = Nz::Retrieve(map, id);
		if (identifier.moduleIndex != m_currentState->currentModuleIndex)
			Append(m_currentState->moduleNames[identifier.moduleIndex], '.');

		if (identifier.externalBlockIndex && identifier.externalBlockIndex != m_currentState->currentExternalBlockIndex)
			Append(m_currentState->externalBlockNames[*identifier.externalBlockIndex], '.');

		Append(identifier.name);
	}

	template<typename... Args>
	void LangWriter::AppendLine(Args&&... params)
	{
		(Append(std::forward<Args>(params)), ...);
		AppendLine();
	}

	template<typename T>
	void LangWriter::AppendValue(const T& value)
	{
		if constexpr (std::is_same_v<T, std::vector<bool>::reference>)
		{
			// fallback for std::vector<bool>
			bool v = value;
			return AppendValue(v);
		}
		else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, std::uint32_t>)
		{
			bool hasUntypedLiterals = m_currentState->currentModule->metadata->shaderLangVersion >= Version::UntypedLiterals;
			Append(Ast::ToString(value, !hasUntypedLiterals || m_currentState->enforceNonDefaultTypes));
		}
		else
			Append(Ast::ConstantToString(value));
	}

	void LangWriter::AppendModuleAttributes(const Ast::Module::Metadata& metadata)
	{
		AppendAttributes(true, LangVersionAttribute{ metadata.shaderLangVersion });
		for (Ast::ModuleFeature feature : metadata.enabledFeatures)
			AppendAttributes(true, FeatureAttribute{ feature });

		AppendAttributes(true, AuthorAttribute{ metadata.author }, DescriptionAttribute{ metadata.description });
		AppendAttributes(true, LicenseAttribute{ metadata.license });
	}

	void LangWriter::AppendStatementList(std::vector<Ast::StatementPtr>& statements)
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

	void LangWriter::EnterScope()
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		AppendLine("{");
		m_currentState->indentLevel++;
	}

	void LangWriter::LeaveScope(bool skipLine)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		m_currentState->indentLevel--;
		AppendLine();

		if (skipLine)
			AppendLine("}");
		else
			Append("}");
	}

	void LangWriter::RegisterAlias(std::size_t aliasIndex, std::string aliasName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(aliasName);

		assert(m_currentState->aliases.find(aliasIndex) == m_currentState->aliases.end());
		m_currentState->aliases.emplace(aliasIndex, std::move(identifier));
	}

	void LangWriter::RegisterConstant(std::size_t constantIndex, std::string constantName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(constantName);

		assert(m_currentState->constants.find(constantIndex) == m_currentState->constants.end());
		m_currentState->constants.emplace(constantIndex, std::move(identifier));
	}

	void LangWriter::RegisterFunction(std::size_t funcIndex, std::string functionName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(functionName);

		assert(m_currentState->functions.find(funcIndex) == m_currentState->functions.end());
		m_currentState->functions.emplace(funcIndex, std::move(identifier));
	}

	void LangWriter::RegisterModule(std::size_t moduleIndex, std::string moduleName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(moduleName);

		assert(m_currentState->modules.find(moduleIndex) == m_currentState->modules.end());
		m_currentState->modules.emplace(moduleIndex, std::move(identifier));
	}

	void LangWriter::RegisterStruct(std::size_t structIndex, const Ast::StructDescription& structDescription)
	{
		State::StructData structData;
		structData.moduleIndex = m_currentState->currentModuleIndex;
		structData.name = structDescription.name;
		structData.desc = &structDescription;

		assert(m_currentState->structs.find(structIndex) == m_currentState->structs.end());
		m_currentState->structs.emplace(structIndex, std::move(structData));
	}

	void LangWriter::RegisterVariable(std::size_t varIndex, std::string varName)
	{
		State::Identifier identifier;
		identifier.externalBlockIndex = m_currentState->currentExternalBlockIndex;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(varName);

		assert(m_currentState->variables.find(varIndex) == m_currentState->variables.end());
		m_currentState->variables.emplace(varIndex, std::move(identifier));
	}

	void LangWriter::ScopeVisit(Ast::Statement& node)
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

	void LangWriter::Visit(Ast::ExpressionPtr& expr, bool encloseIfRequired)
	{
		bool enclose = encloseIfRequired && (GetExpressionCategory(*expr) != Ast::ExpressionCategory::LValue);

		if (enclose)
			Append("(");

		expr->Visit(*this);

		if (enclose)
			Append(")");
	}

	void LangWriter::Visit(Ast::AccessFieldExpression& node)
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

	void LangWriter::Visit(Ast::AccessIdentifierExpression& node)
	{
		Visit(node.expr, true);

		for (const auto& identifierEntry : node.identifiers)
			Append(".", identifierEntry.identifier);
	}

	void LangWriter::Visit(Ast::AccessIndexExpression& node)
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

	void LangWriter::Visit(Ast::AliasValueExpression& node)
	{
		AppendIdentifier(m_currentState->aliases, node.aliasId);
	}

	void LangWriter::Visit(Ast::AssignExpression& node)
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

	void LangWriter::Visit(Ast::BinaryExpression& node)
	{
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

		Visit(node.right, true);
	}

	void LangWriter::Visit(Ast::CallFunctionExpression& node)
	{
		node.targetFunction->Visit(*this);

		Append("(");
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			if (i != 0)
				Append(", ");

			if (node.parameters[i].semantic == Ast::FunctionParameterSemantic::InOut)
			{
				Append("inout ");
			}
			else if (node.parameters[i].semantic == Ast::FunctionParameterSemantic::Out)
			{
				Append("out ");
			}

			node.parameters[i].expr->Visit(*this);
		}
		Append(")");
	}

	void LangWriter::Visit(Ast::CastExpression& node)
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

	void LangWriter::Visit(Ast::ConditionalExpression& node)
	{
		Append("const_select(");
		node.condition->Visit(*this);
		Append(", ");
		node.truePath->Visit(*this);
		Append(", ");
		node.falsePath->Visit(*this);
		Append(")");
	}

	void LangWriter::Visit(Ast::ConstantArrayValueExpression& node)
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

	void LangWriter::Visit(Ast::ConstantValueExpression& node)
	{
		std::visit([&](auto&& arg)
		{
			AppendValue(arg);
		}, node.value);
	}

	void LangWriter::Visit(Ast::ConstantExpression& node)
	{
		AppendIdentifier(m_currentState->constants, node.constantId);
	}

	void LangWriter::Visit(Ast::FunctionExpression& node)
	{
		AppendIdentifier(m_currentState->functions, node.funcId);
	}

	void LangWriter::Visit(Ast::IdentifierExpression& node)
	{
		Append(node.identifier);
	}

	void LangWriter::Visit(Ast::IntrinsicExpression& node)
	{
		bool method = false;
		switch (node.intrinsic)
		{
			// Function intrinsics
			case Ast::IntrinsicType::Abs:
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
			case Ast::IntrinsicType::DegToRad:
			case Ast::IntrinsicType::Distance:
			case Ast::IntrinsicType::DotProduct:
			case Ast::IntrinsicType::Exp:
			case Ast::IntrinsicType::Exp2:
			case Ast::IntrinsicType::Floor:
			case Ast::IntrinsicType::Fract:
			case Ast::IntrinsicType::InverseSqrt:
			case Ast::IntrinsicType::Length:
			case Ast::IntrinsicType::Lerp:
			case Ast::IntrinsicType::Log:
			case Ast::IntrinsicType::Log2:
			case Ast::IntrinsicType::MatrixInverse:
			case Ast::IntrinsicType::MatrixTranspose:
			case Ast::IntrinsicType::Max:
			case Ast::IntrinsicType::Min:
			case Ast::IntrinsicType::Normalize:
			case Ast::IntrinsicType::Pow:
			case Ast::IntrinsicType::RadToDeg:
			case Ast::IntrinsicType::Reflect:
			case Ast::IntrinsicType::Round:
			case Ast::IntrinsicType::RoundEven:
			case Ast::IntrinsicType::Select:
			case Ast::IntrinsicType::Sign:
			case Ast::IntrinsicType::Sin:
			case Ast::IntrinsicType::Sinh:
			case Ast::IntrinsicType::Sqrt:
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

			// Method intrinsics
			case Ast::IntrinsicType::ArraySize:
				assert(!node.parameters.empty());
				Visit(node.parameters.front(), true);
				Append(".Size");
				method = true;
				break;

			case Ast::IntrinsicType::TextureRead:
				assert(!node.parameters.empty());
				Visit(node.parameters.front(), true);
				Append(".Read");
				method = true;
				break;

			case Ast::IntrinsicType::TextureSampleImplicitLod:
				assert(!node.parameters.empty());
				Visit(node.parameters.front(), true);
				Append(".Sample");
				method = true;
				break;

			case Ast::IntrinsicType::TextureSampleImplicitLodDepthComp:
				assert(!node.parameters.empty());
				Visit(node.parameters.front(), true);
				Append(".SampleDepthComp");
				method = true;
				break;

			case Ast::IntrinsicType::TextureWrite:
				assert(!node.parameters.empty());
				Visit(node.parameters.front(), true);
				Append(".Write");
				method = true;
				break;
		}

		// We have to enforce constant types for intrinsics for the right overload to be used
		bool prevShouldEnforceTypes = m_currentState->enforceNonDefaultTypes;
		m_currentState->enforceNonDefaultTypes = true;

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

		m_currentState->enforceNonDefaultTypes = prevShouldEnforceTypes;
	}

	void LangWriter::Visit(Ast::ModuleExpression& node)
	{
		AppendIdentifier(m_currentState->modules, node.moduleId);
	}

	void LangWriter::Visit(Ast::NamedExternalBlockExpression& node)
	{
		Append(m_currentState->externalBlockNames[node.externalBlockId]);
	}

	void LangWriter::Visit(Ast::StructTypeExpression& node)
	{
		AppendIdentifier(m_currentState->structs, node.structTypeId);
	}

	void LangWriter::Visit(Ast::SwizzleExpression& node)
	{
		Visit(node.expression, true);
		Append(".");

		const char* componentStr = "xyzw";
		for (std::size_t i = 0; i < node.componentCount; ++i)
			Append(componentStr[node.components[i]]);
	}

	void LangWriter::Visit(Ast::VariableValueExpression& node)
	{
		AppendIdentifier(m_currentState->variables, node.variableId);
	}

	void LangWriter::Visit(Ast::UnaryExpression& node)
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

		node.expression->Visit(*this);
	}

	void LangWriter::Visit(Ast::BranchStatement& node)
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

	void LangWriter::Visit(Ast::BreakStatement& /*node*/)
	{
		Append("break;");
	}

	void LangWriter::Visit(Ast::ConditionalStatement& node)
	{
		Append("[cond(");
		node.condition->Visit(*this);
		AppendLine(")]");
		node.statement->Visit(*this);
	}

	void LangWriter::Visit(Ast::ContinueStatement& /*node*/)
	{
		Append("continue;");
	}

	void LangWriter::Visit(Ast::DeclareAliasStatement& node)
	{
		if (node.aliasIndex)
			RegisterAlias(*node.aliasIndex, node.name);

		Append("alias ", node.name, " = ");
		assert(node.expression);
		node.expression->Visit(*this);

		// Special case, if that alias points to a module, use it instead to try to keep source code readable
		if (node.expression->GetType() == Ast::NodeType::ModuleExpression)
		{
			auto& moduleExpr = Nz::SafeCast<Ast::ModuleExpression&>(*node.expression);
			m_currentState->moduleNames[moduleExpr.moduleId] = node.name;
		}

		AppendLine(";");
	}

	void LangWriter::Visit(Ast::DeclareConstStatement& node)
	{
		if (node.constIndex)
			RegisterConstant(*node.constIndex, node.name);

		Append("const ", node.name);
		if (node.type.HasValue() && (!node.type.IsResultingValue() || !IsLiteralType(node.type.GetResultingValue())))
			Append(": ", node.type);

		if (node.expression)
		{
			Append(" = ");
			node.expression->Visit(*this);
		}

		AppendLine(";");
	}

	void LangWriter::Visit(Ast::DeclareExternalStatement& node)
	{
		AppendAttributes(true, SetAttribute{ node.bindingSet }, AutoBindingAttribute{ node.autoBinding }, TagAttribute{ node.tag });
		Append("external");

		if (!node.name.empty())
		{
			Append(" ", node.name);

			m_currentState->currentExternalBlockIndex = m_currentState->externalBlockNames.size();
			m_currentState->externalBlockNames.push_back(node.name);
		}

		AppendLine();

		EnterScope();

		bool first = true;
		for (const auto& externalVar : node.externalVars)
		{
			if (!first)
				AppendLine(",");

			first = false;

			if (externalVar.type.IsResultingValue() && IsPushConstantType(externalVar.type.GetResultingValue())) // push constants don't have set or binding'
				AppendAttributes(false, TagAttribute{ externalVar.tag });
			else
				AppendAttributes(false, SetAttribute{ externalVar.bindingSet }, BindingAttribute{ externalVar.bindingIndex }, TagAttribute{ externalVar.tag });
			Append(externalVar.name, ": ", externalVar.type);

			if (externalVar.varIndex)
				RegisterVariable(*externalVar.varIndex, externalVar.name);
		}

		LeaveScope();

		m_currentState->currentExternalBlockIndex = {};
	}

	void LangWriter::Visit(Ast::DeclareFunctionStatement& node)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		AppendAttributes(true,
			EntryAttribute{ node.entryStage },
			WorkgroupAttribute{ node.workgroupSize },
			EarlyFragmentTestsAttribute{ node.earlyFragmentTests },
			DepthWriteAttribute{ node.depthWrite }
		);

		Append("fn ", node.name, "(");
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			const auto& parameter = node.parameters[i];

			if (i != 0)
				Append(", ");

			if (parameter.semantic == Ast::FunctionParameterSemantic::InOut)
			{
				Append("inout ");
			}
			else if (parameter.semantic == Ast::FunctionParameterSemantic::Out)
			{
				Append("out ");
			}

			Append(parameter.name);
			Append(": ");
			Append(parameter.type);

			if (parameter.varIndex)
				RegisterVariable(*parameter.varIndex, parameter.name);
		}
		Append(")");
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

	void LangWriter::Visit(Ast::DeclareOptionStatement& node)
	{
		if (node.optIndex)
			RegisterConstant(*node.optIndex, node.optName);

		Append("option ", node.optName);
		if (node.optType.HasValue())
			Append(": ", node.optType);

		if (node.defaultValue)
		{
			Append(" = ");
			node.defaultValue->Visit(*this);
		}

		Append(";");
	}

	void LangWriter::Visit(Ast::DeclareStructStatement& node)
	{
		if (node.structIndex)
			RegisterStruct(*node.structIndex, node.description);

		AppendAttributes(true, LayoutAttribute{ node.description.layout }, TagAttribute{ node.description.tag });
		Append("struct ");
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
				Append(member.name, ": ", member.type);
			}
		}
		LeaveScope();
	}

	void LangWriter::Visit(Ast::DeclareVariableStatement& node)
	{
		if (node.varIndex)
			RegisterVariable(*node.varIndex, node.varName);

		Append("let ", node.varName);
		if (node.varType.HasValue() && (!node.varType.IsResultingValue() || !IsLiteralType(node.varType.GetResultingValue())))
			Append(": ", node.varType);

		if (node.initialExpression)
		{
			Append(" = ");
			node.initialExpression->Visit(*this);
		}

		Append(";");
	}

	void LangWriter::Visit(Ast::DiscardStatement& /*node*/)
	{
		Append("discard;");
	}

	void LangWriter::Visit(Ast::ExpressionStatement& node)
	{
		node.expression->Visit(*this);
		Append(";");
	}

	void LangWriter::Visit(Ast::ForStatement& node)
	{
		if (node.varIndex)
			RegisterVariable(*node.varIndex, node.varName);

		AppendAttributes(true, UnrollAttribute{ node.unroll });
		Append("for ", node.varName, " in ");
		node.fromExpr->Visit(*this);
		Append(" -> ");
		node.toExpr->Visit(*this);

		if (node.stepExpr)
		{
			Append(" : ");
			node.stepExpr->Visit(*this);
		}

		AppendLine();

		ScopeVisit(*node.statement);
	}

	void LangWriter::Visit(Ast::ForEachStatement& node)
	{
		if (node.varIndex)
			RegisterVariable(*node.varIndex, node.varName);

		AppendAttributes(true, UnrollAttribute{ node.unroll });
		Append("for ", node.varName, " in ");
		node.expression->Visit(*this);
		AppendLine();

		ScopeVisit(*node.statement);
	}

	void LangWriter::Visit(Ast::ImportStatement& node)
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

	void LangWriter::Visit(Ast::MultiStatement& node)
	{
		AppendStatementList(node.statements);
	}

	void LangWriter::Visit(Ast::NoOpStatement& /*node*/)
	{
		/* nothing to do */
	}

	void LangWriter::Visit(Ast::ReturnStatement& node)
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

	void LangWriter::Visit(Ast::ScopedStatement& node)
	{
		EnterScope();
		node.statement->Visit(*this);
		LeaveScope();
	}

	void LangWriter::Visit(Ast::WhileStatement& node)
	{
		Append("while (");
		node.condition->Visit(*this);
		AppendLine(")");

		ScopeVisit(*node.body);
	}

	void LangWriter::AppendHeader(const Ast::Module& module)
	{
		AppendModuleAttributes(*module.metadata);
		if (!module.metadata->moduleName.empty() && module.metadata->moduleName[0] != '_')
			AppendLine("module ", module.metadata->moduleName, ";");
		else
			AppendLine("module;");
		AppendLine();
	}
}
