// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/LangWriter.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <Nazara/Utils/CallOnExit.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/LangData.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <cassert>
#include <optional>
#include <stdexcept>

namespace nzsl
{
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

	struct LangWriter::DepthWriteAttribute
	{
		const Ast::ExpressionValue<Ast::DepthWriteMode>& writeMode;

		bool HasValue() const { return writeMode.HasValue(); }
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

	struct LangWriter::LangVersionAttribute
	{
		std::uint32_t version;

		bool HasValue() const { return true; }
	};

	struct LangWriter::LayoutAttribute
	{
		const Ast::ExpressionValue<StructLayout>& layout;

		bool HasValue() const { return layout.HasValue(); }
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

	struct LangWriter::UnrollAttribute
	{
		const Ast::ExpressionValue<Ast::LoopUnroll>& unroll;

		bool HasValue() const { return unroll.HasValue(); }
	};

	struct LangWriter::State
	{
		struct Identifier
		{
			std::size_t moduleIndex;
			std::string name;
		};

		const States* states = nullptr;
		const Ast::Module* module;
		std::size_t currentModuleIndex;
		std::stringstream stream;
		std::unordered_map<std::size_t, Identifier> aliases;
		std::unordered_map<std::size_t, Identifier> constants;
		std::unordered_map<std::size_t, Identifier> functions;
		std::unordered_map<std::size_t, Identifier> structs;
		std::unordered_map<std::size_t, Identifier> variables;
		std::vector<std::string> moduleNames;
		bool isInEntryPoint = false;
		unsigned int indentLevel = 0;
	};

	std::string LangWriter::Generate(const Ast::Module& module, const States& /*states*/)
	{
		State state;
		m_currentState = &state;
		Nz::CallOnExit onExit([this]()
		{
			m_currentState = nullptr;
		});

		state.module = &module;

		AppendHeader();

		// Register imported modules
		m_currentState->currentModuleIndex = 0;
		for (const auto& importedModule : module.importedModules)
		{
			AppendAttributes(true, LangVersionAttribute{ importedModule.module->metadata->shaderLangVersion });
			AppendLine("module ", importedModule.identifier);
			EnterScope();
			importedModule.module->rootNode->Visit(*this);
			LeaveScope(true);

			m_currentState->currentModuleIndex++;
			m_currentState->moduleNames.push_back(importedModule.identifier);
		}

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

	void LangWriter::Append(Ast::PrimitiveType type)
	{
		switch (type)
		{
			case Ast::PrimitiveType::Boolean: return Append("bool");
			case Ast::PrimitiveType::Float32: return Append("f32");
			case Ast::PrimitiveType::Int32:   return Append("i32");
			case Ast::PrimitiveType::UInt32:  return Append("u32");
			case Ast::PrimitiveType::String:  return Append("string");
		}
	}

	void LangWriter::Append(const Ast::SamplerType& samplerType)
	{
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

		Append("[", samplerType.sampledType, "]");
	}

	void LangWriter::Append(const Ast::StorageType& storageType)
	{
		Append("storage[", storageType.containedType, "]");
	}

	void LangWriter::Append(const Ast::StructType& structType)
	{
		AppendIdentifier(m_currentState->structs, structType.structIndex);
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

		m_currentState->stream << param;
	}

	template<typename T1, typename T2, typename... Args>
	void LangWriter::Append(const T1& firstParam, const T2& secondParam, Args&&... params)
	{
		Append(firstParam);
		Append(secondParam, std::forward<Args>(params)...);
	}

	template<typename ...Args>
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
		{
			auto it = Ast::s_builtinData.find(attribute.builtin.GetResultingValue());
			assert(it != Ast::s_builtinData.end());

			Append(it->second.identifier);
		}
		else
			attribute.builtin.GetExpression()->Visit(*this);

		Append(")");
	}
	
	void LangWriter::AppendAttribute(DepthWriteAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("depth_write(");

		if (attribute.writeMode.IsResultingValue())
		{
			switch (attribute.writeMode.GetResultingValue())
			{
				case Ast::DepthWriteMode::Greater:
					Append("greater");
					break;

				case Ast::DepthWriteMode::Less:
					Append("less");
					break;

				case Ast::DepthWriteMode::Replace:
					Append("replace");
					break;

				case Ast::DepthWriteMode::Unchanged:
					Append("unchanged");
					break;
			}
		}
		else
			attribute.writeMode.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(EarlyFragmentTestsAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("early_fragment_tests(");

		if (attribute.earlyFragmentTests.IsResultingValue())
		{
			if (attribute.earlyFragmentTests.GetResultingValue())
				Append("true");
			else
				Append("false");
		}
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
		{
			switch (attribute.stageType.GetResultingValue())
			{
				case ShaderStageType::Fragment:
					Append("frag");
					break;

				case ShaderStageType::Vertex:
					Append("vert");
					break;
			}
		}
		else
			attribute.stageType.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendAttribute(LangVersionAttribute attribute)
	{
		std::uint32_t shaderLangVersion = attribute.version;
		std::uint32_t majorVersion = shaderLangVersion / 100;
		shaderLangVersion -= majorVersion * 100;

		std::uint32_t minorVersion = shaderLangVersion / 10;
		shaderLangVersion -= minorVersion * 100;

		std::uint32_t patchVersion = shaderLangVersion;

		// nzsl_version
		Append("nzsl_version(\"", majorVersion, ".", minorVersion);
		if (patchVersion != 0)
			Append(".", patchVersion);

		Append("\")");
	}

	void LangWriter::AppendAttribute(LayoutAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("layout(");
		if (attribute.layout.IsResultingValue())
		{
			switch (attribute.layout.GetResultingValue())
			{
				case StructLayout::Packed:
					Append("packed");
					break;

				case StructLayout::Std140:
					Append("std140");
					break;
			}
		}
		else
			attribute.layout.GetExpression()->Visit(*this);
		Append(")");
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

	void LangWriter::AppendAttribute(UnrollAttribute attribute)
	{
		if (!attribute.HasValue())
			return;

		Append("unroll(");

		if (attribute.unroll.IsResultingValue())
		{
			switch (attribute.unroll.GetResultingValue())
			{
				case Ast::LoopUnroll::Always:
					Append("always");
					break;

				case Ast::LoopUnroll::Hint:
					Append("hint");
					break;

				case Ast::LoopUnroll::Never:
					Append("never");
					break;

				default:
					break;
			}
		}
		else
			attribute.unroll.GetExpression()->Visit(*this);

		Append(")");
	}

	void LangWriter::AppendComment(const std::string& section)
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

	void LangWriter::AppendCommentSection(const std::string& section)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		std::string stars((section.size() < 33) ? (36 - section.size()) / 2 : 3, '*');
		m_currentState->stream << "/*" << stars << ' ' << section << ' ' << stars << "*/";
		AppendLine();
	}

	void LangWriter::AppendLine(const std::string& txt)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		m_currentState->stream << txt << '\n' << std::string(m_currentState->indentLevel, '\t');
	}

	template<typename T>
	void LangWriter::AppendIdentifier(const T& map, std::size_t id)
	{
		const auto& structIdentifier = Nz::Retrieve(map, id);
		if (structIdentifier.moduleIndex != m_currentState->currentModuleIndex)
			Append(m_currentState->moduleNames[structIdentifier.moduleIndex], '.');

		Append(structIdentifier.name);
	}

	template<typename... Args>
	void LangWriter::AppendLine(Args&&... params)
	{
		(Append(std::forward<Args>(params)), ...);
		AppendLine();
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

		m_currentState->indentLevel++;
		AppendLine("{");
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

	void LangWriter::RegisterStruct(std::size_t structIndex, std::string structName)
	{
		State::Identifier identifier;
		identifier.moduleIndex = m_currentState->currentModuleIndex;
		identifier.name = std::move(structName);

		assert(m_currentState->structs.find(structIndex) == m_currentState->structs.end());
		m_currentState->structs.emplace(structIndex, std::move(identifier));
	}

	void LangWriter::RegisterVariable(std::size_t varIndex, std::string varName)
	{
		State::Identifier identifier;
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
			case Ast::AssignType::Simple: Append(" = "); break;
			case Ast::AssignType::CompoundAdd: Append(" += "); break;
			case Ast::AssignType::CompoundDivide: Append(" /= "); break;
			case Ast::AssignType::CompoundMultiply: Append(" *= "); break;
			case Ast::AssignType::CompoundLogicalAnd: Append(" &&= "); break;
			case Ast::AssignType::CompoundLogicalOr: Append(" ||= "); break;
			case Ast::AssignType::CompoundSubtract: Append(" -= "); break;
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

			node.parameters[i]->Visit(*this);
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
			if (!exprPtr)
				break;

			if (!first)
				m_currentState->stream << ", ";

			exprPtr->Visit(*this);
			first = false;
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

	void LangWriter::Visit(Ast::ConstantValueExpression& node)
	{
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, Ast::NoValue>)
				throw std::runtime_error("invalid type (value expected)");
			else if constexpr (std::is_same_v<T, bool>)
				Append((arg) ? "true" : "false");
			else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t>)
				Append(std::to_string(arg));
			else if constexpr (std::is_same_v<T, std::string>)
				Append('"', arg, '"'); //< TODO: Escape string
			else if constexpr (std::is_same_v<T, Vector2f>)
				Append("vec2[f32](" + std::to_string(arg.x()) + ", " + std::to_string(arg.y()) + ")");
			else if constexpr (std::is_same_v<T, Vector2i32>)
				Append("vec2[i32](" + std::to_string(arg.x()) + ", " + std::to_string(arg.y()) + ")");
			else if constexpr (std::is_same_v<T, Vector3f>)
				Append("vec3[f32](" + std::to_string(arg.x()) + ", " + std::to_string(arg.y()) + ", " + std::to_string(arg.z()) + ")");
			else if constexpr (std::is_same_v<T, Vector3i32>)
				Append("vec3[i32](" + std::to_string(arg.x()) + ", " + std::to_string(arg.y()) + ", " + std::to_string(arg.z()) + ")");
			else if constexpr (std::is_same_v<T, Vector4f>)
				Append("vec4[f32](" + std::to_string(arg.x()) + ", " + std::to_string(arg.y()) + ", " + std::to_string(arg.z()) + ", " + std::to_string(arg.w()) + ")");
			else if constexpr (std::is_same_v<T, Vector4i32>)
				Append("vec4[i32](" + std::to_string(arg.x()) + ", " + std::to_string(arg.y()) + ", " + std::to_string(arg.z()) + ", " + std::to_string(arg.w()) + ")");
			else
				static_assert(Nz::AlwaysFalse<T>::value, "non-exhaustive visitor");
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
			case Ast::IntrinsicType::CrossProduct:
				Append("cross");
				break;

			case Ast::IntrinsicType::DotProduct:
				Append("dot");
				break;

			case Ast::IntrinsicType::Exp:
				Append("exp");
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
				assert(!node.parameters.empty());
				Visit(node.parameters.front(), true);
				Append(".Sample");
				method = true;
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

	void LangWriter::Visit(Ast::ConditionalStatement& node)
	{
		Append("[cond(");
		node.condition->Visit(*this);
		AppendLine(")]");
		node.statement->Visit(*this);
	}

	void LangWriter::Visit(Ast::DeclareAliasStatement& node)
	{
		if (node.aliasIndex)
			RegisterAlias(*node.aliasIndex, node.name);

		Append("alias ", node.name, " = ");
		assert(node.expression);
		node.expression->Visit(*this);
		AppendLine(";");
	}

	void LangWriter::Visit(Ast::DeclareConstStatement& node)
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

	void LangWriter::Visit(Ast::DeclareExternalStatement& node)
	{
		AppendAttributes(true, SetAttribute{ node.bindingSet });
		AppendLine("external");
		EnterScope();

		bool first = true;
		for (const auto& externalVar : node.externalVars)
		{
			if (!first)
				AppendLine(",");

			first = false;

			AppendAttributes(false, SetAttribute{ externalVar.bindingSet }, BindingAttribute{ externalVar.bindingIndex });
			Append(externalVar.name, ": ", externalVar.type);

			if (externalVar.varIndex)
				RegisterVariable(*externalVar.varIndex, externalVar.name);
		}

		LeaveScope();
	}

	void LangWriter::Visit(Ast::DeclareFunctionStatement& node)
	{
		assert(m_currentState && "This function should only be called while processing an AST");

		if (node.funcIndex)
			RegisterFunction(*node.funcIndex, node.name);

		AppendAttributes(true, EntryAttribute{ node.entryStage }, EarlyFragmentTestsAttribute{ node.earlyFragmentTests }, DepthWriteAttribute{ node.depthWrite });
		Append("fn ", node.name, "(");
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			const auto& parameter = node.parameters[i];

			if (i != 0)
				Append(", ");

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
			RegisterStruct(*node.structIndex, node.description.name);

		AppendAttributes(true, LayoutAttribute{ node.description.layout });
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

				AppendAttributes(false, LocationAttribute{ member.locationIndex }, BuiltinAttribute{ member.builtin });
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
		if (node.varType.HasValue())
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

	void LangWriter::AppendHeader()
	{
		AppendAttributes(true, LangVersionAttribute{ m_currentState->module->metadata->shaderLangVersion });
		if (!m_currentState->module->metadata->moduleName.empty() && m_currentState->module->metadata->moduleName[0] != '_')
			AppendLine("module ", m_currentState->module->metadata->moduleName, ";");
		else
			AppendLine("module;");
		AppendLine();
	}
}
