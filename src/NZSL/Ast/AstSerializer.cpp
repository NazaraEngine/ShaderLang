// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/AstSerializer.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>

namespace nzsl::Ast
{
	namespace
	{
		constexpr std::uint32_t s_shaderAstMagicNumber = 0x4E534852;
		constexpr std::uint32_t s_shaderAstCurrentVersion = 7;

		class ShaderSerializerVisitor : public ExpressionVisitor, public StatementVisitor
		{
			public:
				ShaderSerializerVisitor(SerializerBase& serializer) :
				m_serializer(serializer)
				{
				}

#define NZSL_SHADERAST_NODE(Node, Category) void Visit(Node##Category& node) override \
				{ \
					m_serializer.Serialize(node); \
					m_serializer.SerializeNodeCommon(node); \
					m_serializer.Serialize##Category##Common(node); \
				}

#include <NZSL/Ast/NodeList.hpp>

			private:
				SerializerBase& m_serializer;
		};
	}

	void SerializerBase::Serialize(AccessIdentifierExpression& node)
	{
		Node(node.expr);

		Container(node.identifiers);
		for (auto& identifier : node.identifiers)
		{
			Value(identifier.identifier);
			SourceLoc(identifier.sourceLocation);
		}
	}

	void SerializerBase::Serialize(AccessIndexExpression& node)
	{
		Node(node.expr);

		Container(node.indices);
		for (auto& identifier : node.indices)
			Node(identifier);
	}

	void SerializerBase::Serialize(AliasValueExpression& node)
	{
		SizeT(node.aliasId);
	}

	void SerializerBase::Serialize(AssignExpression& node)
	{
		Enum(node.op);
		Node(node.left);
		Node(node.right);
	}

	void SerializerBase::Serialize(BinaryExpression& node)
	{
		Enum(node.op);
		Node(node.left);
		Node(node.right);
	}

	void SerializerBase::Serialize(CallFunctionExpression& node)
	{
		Node(node.targetFunction);

		Container(node.parameters);
		for (auto& param : node.parameters)
			Node(param);
	}

	void SerializerBase::Serialize(CallMethodExpression& node)
	{
		Node(node.object);
		Value(node.methodName);

		Container(node.parameters);
		for (auto& param : node.parameters)
			Node(param);
	}

	void SerializerBase::Serialize(CastExpression& node)
	{
		ExprValue(node.targetType);

		Container(node.expressions);
		for (auto& expr : node.expressions)
			Node(expr);
	}

	void SerializerBase::Serialize(ConditionalExpression& node)
	{
		Node(node.condition);
		Node(node.truePath);
		Node(node.falsePath);
	}

	void SerializerBase::Serialize(ConstantExpression& node)
	{
		SizeT(node.constantId);
	}

	void SerializerBase::Serialize(ConstantArrayValueExpression& node)
	{
		std::uint32_t typeIndex;
		if (IsWriting())
			typeIndex = std::uint32_t(node.values.index());

		Value(typeIndex);

		// Waiting for template lambda in C++20
		auto SerializeValue = [&](auto dummyType)
		{
			using T = std::decay_t<decltype(dummyType)>;

			using VecT = std::vector<T>;

			auto& values = (IsWriting()) ? std::get<VecT>(node.values) : node.values.emplace<VecT>();
			Container(values);

			// Cannot use range-for because of std::vector<bool> (fuck std::vector<bool>)
			if (IsWriting())
			{
				for (std::size_t i = 0; i < values.size(); ++i)
				{
					const T& value = values[i];
					Value(const_cast<T&>(value)); //< not used for writing
				}
			}
			else
			{
				for (std::size_t i = 0; i < values.size(); ++i)
				{
					T value;
					Value(value);
					
					values[i] = std::move(value);
				}
			}
		};

		static_assert(std::variant_size_v<decltype(node.values)> == 22);
		switch (typeIndex)
		{
			case 0:  break;
			case 1:  SerializeValue(bool()); break;
			case 2:  SerializeValue(float()); break;
			case 3:  SerializeValue(std::int32_t()); break;
			case 4:  SerializeValue(std::uint32_t()); break;
			case 5:  SerializeValue(Vector2f32()); break;
			case 6:  SerializeValue(Vector3f32()); break;
			case 7:  SerializeValue(Vector4f32()); break;
			case 8:  SerializeValue(Vector2i32()); break;
			case 9:  SerializeValue(Vector3i32()); break;
			case 10: SerializeValue(Vector4i32()); break;
			case 11: SerializeValue(std::string()); break;
			case 12: SerializeValue(double()); break;
			case 13: SerializeValue(Vector2f64()); break;
			case 14: SerializeValue(Vector3f64()); break;
			case 15: SerializeValue(Vector4f64()); break;
			case 16: SerializeValue(Vector2u32()); break;
			case 17: SerializeValue(Vector3u32()); break;
			case 18: SerializeValue(Vector4u32()); break;
			case 19: SerializeValue(Vector2<bool>()); break;
			case 20: SerializeValue(Vector3<bool>()); break;
			case 21: SerializeValue(Vector4<bool>()); break;
			default: throw std::runtime_error("unexpected data type");
		}
	}

	void SerializerBase::Serialize(ConstantValueExpression& node)
	{
		std::uint32_t typeIndex;
		if (IsWriting())
			typeIndex = std::uint32_t(node.value.index());

		Value(typeIndex);

		// Waiting for template lambda in C++20
		auto SerializeValue = [&](auto dummyType)
		{
			using T = std::decay_t<decltype(dummyType)>;

			auto& value = (IsWriting()) ? std::get<T>(node.value) : node.value.emplace<T>();
			Value(value);
		};

		static_assert(std::variant_size_v<decltype(node.value)> == 22);
		switch (typeIndex)
		{
			case 0:  break;
			case 1:  SerializeValue(bool()); break;
			case 2:  SerializeValue(float()); break;
			case 3:  SerializeValue(std::int32_t()); break;
			case 4:  SerializeValue(std::uint32_t()); break;
			case 5:  SerializeValue(Vector2f32()); break;
			case 6:  SerializeValue(Vector3f32()); break;
			case 7:  SerializeValue(Vector4f32()); break;
			case 8:  SerializeValue(Vector2i32()); break;
			case 9:  SerializeValue(Vector3i32()); break;
			case 10: SerializeValue(Vector4i32()); break;
			case 11: SerializeValue(std::string()); break;
			case 12: SerializeValue(double()); break;
			case 13: SerializeValue(Vector2f64()); break;
			case 14: SerializeValue(Vector3f64()); break;
			case 15: SerializeValue(Vector4f64()); break;
			case 16: SerializeValue(Vector2u32()); break;
			case 17: SerializeValue(Vector3u32()); break;
			case 18: SerializeValue(Vector4u32()); break;
			case 19: SerializeValue(Vector2<bool>()); break;
			case 20: SerializeValue(Vector3<bool>()); break;
			case 21: SerializeValue(Vector4<bool>()); break;
			default: throw std::runtime_error("unexpected data type");
		}
	}

	void SerializerBase::Serialize(IdentifierExpression& node)
	{
		Value(node.identifier);
	}

	void SerializerBase::Serialize(IntrinsicExpression& node)
	{
		Enum(node.intrinsic);
		Container(node.parameters);
		for (auto& param : node.parameters)
			Node(param);
	}

	void SerializerBase::Serialize(IntrinsicFunctionExpression& node)
	{
		SizeT(node.intrinsicId);
	}

	void SerializerBase::Serialize(StructTypeExpression& node)
	{
		SizeT(node.structTypeId);
	}

	void SerializerBase::Serialize(TypeExpression& node)
	{
		SizeT(node.typeId);
	}

	void SerializerBase::Serialize(FunctionExpression& node)
	{
		SizeT(node.funcId);
	}

	void SerializerBase::Serialize(SwizzleExpression& node)
	{
		SizeT(node.componentCount);
		Node(node.expression);

		for (std::size_t i = 0; i < node.componentCount; ++i)
			Enum(node.components[i]);
	}

	void SerializerBase::Serialize(VariableValueExpression& node)
	{
		SizeT(node.variableId);
	}

	void SerializerBase::Serialize(UnaryExpression& node)
	{
		Enum(node.op);
		Node(node.expression);
	}

	void SerializerBase::Serialize(BranchStatement& node)
	{
		Container(node.condStatements);
		for (auto& condStatement : node.condStatements)
		{
			Node(condStatement.condition);
			Node(condStatement.statement);
		}

		Node(node.elseStatement);
		Value(node.isConst);
	}

	void SerializerBase::Serialize(BreakStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void SerializerBase::Serialize(ConditionalStatement& node)
	{
		Node(node.condition);
		Node(node.statement);
	}

	void SerializerBase::Serialize(ContinueStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void SerializerBase::Serialize(DeclareAliasStatement& node)
	{
		OptVal(node.aliasIndex);
		Value(node.name);
		Node(node.expression);
	}

	void SerializerBase::Serialize(DeclareConstStatement& node)
	{
		OptVal(node.constIndex);
		Value(node.name);
		ExprValue(node.isExported);
		ExprValue(node.type);
		Node(node.expression);
	}

	void SerializerBase::Serialize(DeclareExternalStatement& node)
	{
		ExprValue(node.bindingSet);

		if (IsVersionGreaterOrEqual(4))
			ExprValue(node.autoBinding);

		if (IsVersionGreaterOrEqual(3))
			Value(node.tag);

		Container(node.externalVars);
		for (auto& extVar : node.externalVars)
		{
			Value(extVar.name);
			OptVal(extVar.varIndex);
			ExprValue(extVar.type);
			ExprValue(extVar.bindingIndex);
			ExprValue(extVar.bindingSet);
			SourceLoc(extVar.sourceLocation);
			if (IsVersionGreaterOrEqual(3))
				Value(extVar.tag);
		}
	}

	void SerializerBase::Serialize(DeclareFunctionStatement& node)
	{
		Value(node.name);
		ExprValue(node.returnType);
		ExprValue(node.depthWrite);
		ExprValue(node.earlyFragmentTests);
		ExprValue(node.entryStage);
		ExprValue(node.isExported);
		OptVal(node.funcIndex);

		if (IsVersionGreaterOrEqual(6))
			ExprValue(node.workgroupSize);

		Container(node.parameters);
		for (auto& parameter : node.parameters)
		{
			Value(parameter.name);
			ExprValue(parameter.type);
			OptVal(parameter.varIndex);
			SourceLoc(parameter.sourceLocation);
		}

		Container(node.statements);
		for (auto& statement : node.statements)
			Node(statement);
	}

	void SerializerBase::Serialize(DeclareOptionStatement& node)
	{
		OptVal(node.optIndex);
		Value(node.optName);
		ExprValue(node.optType);
		Node(node.defaultValue);
	}

	void SerializerBase::Serialize(DeclareStructStatement& node)
	{
		OptVal(node.structIndex);
		ExprValue(node.isExported);

		Value(node.description.name);
		ExprValue(node.description.layout);
		if (IsVersionGreaterOrEqual(3))
			Value(node.description.tag);

		Container(node.description.members);
		for (auto& member : node.description.members)
		{
			Value(member.name);
			ExprValue(member.type);
			ExprValue(member.builtin);
			ExprValue(member.cond);
			ExprValue(member.locationIndex);
			SourceLoc(member.sourceLocation);
			if (IsVersionGreaterOrEqual(3))
				Value(member.tag);
			if (IsVersionGreaterOrEqual(7))
				Value(member.originalName);
		}
	}
	
	void SerializerBase::Serialize(DeclareVariableStatement& node)
	{
		OptVal(node.varIndex);
		Value(node.varName);
		ExprValue(node.varType);
		Node(node.initialExpression);
	}

	void SerializerBase::Serialize(DiscardStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void SerializerBase::Serialize(ExpressionStatement& node)
	{
		Node(node.expression);
	}

	void SerializerBase::Serialize(ForStatement& node)
	{
		OptVal(node.varIndex);
		ExprValue(node.unroll);
		Value(node.varName);
		Node(node.fromExpr);
		Node(node.toExpr);
		Node(node.stepExpr);
		Node(node.statement);
	}

	void SerializerBase::Serialize(ForEachStatement& node)
	{
		OptVal(node.varIndex);
		ExprValue(node.unroll);
		Value(node.varName);
		Node(node.expression);
		Node(node.statement);
	}

	void SerializerBase::Serialize(ImportStatement& node)
	{
		Value(node.moduleName);
		Container(node.identifiers);
		for (auto& identifierEntry : node.identifiers)
		{
			Value(identifierEntry.identifier);
			Value(identifierEntry.renamedIdentifier);
			SourceLoc(identifierEntry.identifierLoc);
			SourceLoc(identifierEntry.renamedIdentifierLoc);
		}
	}

	void SerializerBase::Serialize(MultiStatement& node)
	{
		Container(node.statements);
		for (auto& statement : node.statements)
			Node(statement);
	}

	void SerializerBase::Serialize(NoOpStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void SerializerBase::Serialize(ReturnStatement& node)
	{
		Node(node.returnExpr);
	}

	void SerializerBase::Serialize(ScopedStatement& node)
	{
		Node(node.statement);
	}

	void SerializerBase::Serialize(WhileStatement& node)
	{
		ExprValue(node.unroll);
		Node(node.condition);
		Node(node.body);
	}

	void SerializerBase::SerializeExpressionCommon(Expression& expr)
	{
		OptType(expr.cachedExpressionType);
	}

	void SerializerBase::SerializeNodeCommon(Ast::Node& node)
	{
		SourceLoc(node.sourceLocation);
	}

	void SerializerBase::SerializeStatementCommon(Statement& /*stmt*/)
	{
	}

	void ShaderAstSerializer::Serialize(const Module& module)
	{
		m_serializer.Serialize(s_shaderAstMagicNumber);
		m_serializer.Serialize(s_shaderAstCurrentVersion);

		SerializeModule(const_cast<Module&>(module)); //< won't be used for writing
	}
	
	bool ShaderAstSerializer::IsVersionGreaterOrEqual(std::uint32_t /*version*/) const
	{
		return true; //< we're writing the last binary version
	}

	bool ShaderAstSerializer::IsWriting() const
	{
		return true;
	}

	void ShaderAstSerializer::Node(ExpressionPtr& node)
	{
		NodeType nodeType = (node) ? node->GetType() : NodeType::None;
		m_serializer.Serialize(static_cast<std::int32_t>(nodeType));

		if (node)
		{
			ShaderSerializerVisitor visitor(*this);
			node->Visit(visitor);
		}
	}

	void ShaderAstSerializer::Node(StatementPtr& node)
	{
		NodeType nodeType = (node) ? node->GetType() : NodeType::None;
		m_serializer.Serialize(static_cast<std::int32_t>(nodeType));

		if (node)
		{
			ShaderSerializerVisitor visitor(*this);
			node->Visit(visitor);
		}
	}
	
	void ShaderAstSerializer::SerializeModule(Module& module)
	{
		Metadata(const_cast<Module::Metadata&>(*module.metadata)); //< won't be used for writing

		Container(module.importedModules);
		for (auto& importedModule : module.importedModules)
		{
			Value(importedModule.identifier);
			SerializeModule(*importedModule.module);
		}

		ShaderSerializerVisitor visitor(*this);
		module.rootNode->Visit(visitor);
	}

	void ShaderAstSerializer::SharedString(std::shared_ptr<const std::string>& val)
	{
		bool hasValue = (val != nullptr);
		Value(hasValue);

		if (hasValue)
		{
			auto it = m_stringIndices.find(*val);
			bool newString = (it == m_stringIndices.end());
			Value(newString);

			if (newString)
			{
				Value(const_cast<std::string&>(*val)); //< won't be used for writing
				m_stringIndices.emplace(*val, Nz::SafeCast<std::uint32_t>(m_stringIndices.size()));
			}
			else
				Value(it->second); //< string index
		}
	}

	void ShaderAstSerializer::Type(ExpressionType& type)
	{
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, NoType>)
				m_serializer.Serialize(std::uint8_t(0));
			else if constexpr (std::is_same_v<T, PrimitiveType>)
			{
				m_serializer.Serialize(std::uint8_t(1));
				Enum(arg);
			}
			else if constexpr (std::is_same_v<T, MatrixType>)
			{
				m_serializer.Serialize(std::uint8_t(3));
				SizeT(arg.columnCount);
				SizeT(arg.rowCount);
				Enum(arg.type);
			}
			else if constexpr (std::is_same_v<T, SamplerType>)
			{
				m_serializer.Serialize(std::uint8_t(4));
				Enum(arg.dim);
				Enum(arg.sampledType);
				if (IsVersionGreaterOrEqual(5))
					Value(arg.depth);
			}
			else if constexpr (std::is_same_v<T, StructType>)
			{
				m_serializer.Serialize(std::uint8_t(5));
				SizeT(arg.structIndex);
			}
			else if constexpr (std::is_same_v<T, UniformType>)
			{
				m_serializer.Serialize(std::uint8_t(6));
				SizeT(arg.containedType.structIndex);
			}
			else if constexpr (std::is_same_v<T, VectorType>)
			{
				m_serializer.Serialize(std::uint8_t(7));
				SizeT(arg.componentCount);
				Enum(arg.type);
			}
			else if constexpr (std::is_same_v<T, ArrayType>)
			{
				m_serializer.Serialize(std::uint8_t(8));
				Value(arg.length);
				Type(arg.containedType->type);
			}
			else if constexpr (std::is_same_v<T, Ast::Type>)
			{
				m_serializer.Serialize(std::uint8_t(9));
				SizeT(arg.typeIndex);
			}
			else if constexpr (std::is_same_v<T, Ast::FunctionType>)
			{
				m_serializer.Serialize(std::uint8_t(10));
				SizeT(arg.funcIndex);
			}
			else if constexpr (std::is_same_v<T, Ast::IntrinsicFunctionType>)
			{
				m_serializer.Serialize(std::uint8_t(11));
				Enum(arg.intrinsic);
			}
			else if constexpr (std::is_same_v<T, Ast::MethodType>)
			{
				m_serializer.Serialize(std::uint8_t(12));
				Type(arg.objectType->type);
				SizeT(arg.methodIndex);
			}
			else if constexpr (std::is_same_v<T, Ast::AliasType>)
			{
				m_serializer.Serialize(std::uint8_t(13));
				SizeT(arg.aliasIndex);
				Type(arg.targetType->type);
			}
			else if constexpr (std::is_same_v<T, Ast::StorageType>)
			{
				m_serializer.Serialize(std::uint8_t(14));
				SizeT(arg.containedType.structIndex);
			}
			else if constexpr (std::is_same_v<T, Ast::DynArrayType>)
			{
				m_serializer.Serialize(std::uint8_t(15));
				Type(arg.containedType->type);
			}
			else if constexpr (std::is_same_v<T, Ast::TextureType>)
			{
				m_serializer.Serialize(std::uint8_t(16));
				Enum(arg.accessPolicy);
				Enum(arg.format);
				Enum(arg.dim);
				Enum(arg.baseType);
			}
			else if constexpr (std::is_same_v<T, PushConstantType>)
			{
				m_serializer.Serialize(std::uint8_t(17));
				SizeT(arg.containedType.structIndex);
			}
			else
				static_assert(Nz::AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, type);
	}

	void ShaderAstSerializer::Value(bool& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(double& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(float& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(std::string& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(std::int32_t& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(Vector2<bool>& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
	}

	void ShaderAstSerializer::Value(Vector3<bool>& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
	}

	void ShaderAstSerializer::Value(Vector4<bool>& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
		m_serializer.Serialize(val.w());
	}

	void ShaderAstSerializer::Value(Vector2f32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
	}

	void ShaderAstSerializer::Value(Vector3f32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
	}

	void ShaderAstSerializer::Value(Vector4f32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
		m_serializer.Serialize(val.w());
	}

	void ShaderAstSerializer::Value(Vector2f64& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
	}

	void ShaderAstSerializer::Value(Vector3f64& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
	}

	void ShaderAstSerializer::Value(Vector4f64& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
		m_serializer.Serialize(val.w());
	}

	void ShaderAstSerializer::Value(Vector2i32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
	}

	void ShaderAstSerializer::Value(Vector3i32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
	}

	void ShaderAstSerializer::Value(Vector4i32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
		m_serializer.Serialize(val.w());
	}

	void ShaderAstSerializer::Value(Vector2u32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
	}

	void ShaderAstSerializer::Value(Vector3u32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
	}

	void ShaderAstSerializer::Value(Vector4u32& val)
	{
		m_serializer.Serialize(val.x());
		m_serializer.Serialize(val.y());
		m_serializer.Serialize(val.z());
		m_serializer.Serialize(val.w());
	}

	void ShaderAstSerializer::Value(std::uint8_t& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(std::uint16_t& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(std::uint32_t& val)
	{
		m_serializer.Serialize(val);
	}

	void ShaderAstSerializer::Value(std::uint64_t& val)
	{
		m_serializer.Serialize(val);
	}

	ModulePtr ShaderAstUnserializer::Unserialize()
	{
		std::uint32_t magicNumber = 0;
		m_version = 0;
		m_unserializer.Unserialize(magicNumber);
		if (magicNumber != s_shaderAstMagicNumber)
			throw std::runtime_error("invalid shader file");

		m_unserializer.Unserialize(m_version);
		if (m_version > s_shaderAstCurrentVersion)
			throw std::runtime_error("unsupported version");

		ModulePtr module = std::make_shared<Module>();
		SerializeModule(*module);

		return module;
	}

	bool ShaderAstUnserializer::IsVersionGreaterOrEqual(std::uint32_t version) const
	{
		return m_version >= version;
	}

	bool ShaderAstUnserializer::IsWriting() const
	{
		return false;
	}

	void ShaderAstUnserializer::Node(ExpressionPtr& node)
	{
		std::int32_t nodeTypeInt = - 1;
		m_unserializer.Unserialize(nodeTypeInt);

		if (nodeTypeInt < static_cast<std::int32_t>(NodeType::None) || nodeTypeInt > static_cast<std::int32_t>(NodeType::Max))
			throw std::runtime_error("invalid node type");

		NodeType nodeType = static_cast<NodeType>(nodeTypeInt);
		switch (nodeType)
		{
			case NodeType::None: break;

#define NZSL_SHADERAST_EXPRESSION(Node) case NodeType:: Node##Expression : node = std::make_unique<Node##Expression>(); break;
#include <NZSL/Ast/NodeList.hpp>

			default: throw std::runtime_error("unexpected node type");
		}

		if (node)
		{
			ShaderSerializerVisitor visitor(*this);
			node->Visit(visitor);
		}
	}

	void ShaderAstUnserializer::Node(StatementPtr& node)
	{
		std::int32_t nodeTypeInt = -1;
		m_unserializer.Unserialize(nodeTypeInt);

		if (nodeTypeInt < static_cast<std::int32_t>(NodeType::None) || nodeTypeInt > static_cast<std::int32_t>(NodeType::Max))
			throw std::runtime_error("invalid node type");

		NodeType nodeType = static_cast<NodeType>(nodeTypeInt);
		switch (nodeType)
		{
			case NodeType::None: break;

#define NZSL_SHADERAST_STATEMENT(Node) case NodeType:: Node##Statement : node = std::make_unique<Node##Statement>(); break;
#include <NZSL/Ast/NodeList.hpp>

			default: throw std::runtime_error("unexpected node type");
		}

		if (node)
		{
			ShaderSerializerVisitor visitor(*this);
			node->Visit(visitor);
		}
	}

	void ShaderAstUnserializer::SerializeModule(Module& module)
	{
		std::shared_ptr<Module::Metadata> metadata = std::make_shared<Module::Metadata>();
		Metadata(*metadata);

		std::vector<Module::ImportedModule> importedModules;
		Container(importedModules);
		for (auto& importedModule : importedModules)
		{
			Value(const_cast<std::string&>(importedModule.identifier)); //< not used for writing

			importedModule.module = std::make_shared<Module>();
			SerializeModule(*importedModule.module);
		}

		MultiStatementPtr rootNode = std::make_unique<MultiStatement>();

		ShaderSerializerVisitor visitor(*this);
		rootNode->Visit(visitor);

		module = Module(std::move(metadata), std::move(rootNode), std::move(importedModules));
	}

	void ShaderAstUnserializer::SharedString(std::shared_ptr<const std::string>& val)
	{
		bool hasValue;
		Value(hasValue);

		if (hasValue)
		{
			bool newString;
			Value(newString);

			if (newString)
			{
				std::string newStr;
				Value(newStr);

				val = std::make_shared<const std::string>(std::move(newStr));
				m_strings.emplace_back(val);
			}
			else
			{
				std::uint32_t strIndex;
				Value(strIndex);

				assert(strIndex < m_strings.size());
				val = m_strings[strIndex];
			}
		}
	}

	void ShaderAstUnserializer::Type(ExpressionType& type)
	{
#ifdef NAZARA_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

		std::uint8_t typeIndex;
		Value(typeIndex);

		switch (typeIndex)
		{
			case 0: //< NoType
				type = NoType{};
				break;

			case 1: //< PrimitiveType
			{
				PrimitiveType primitiveType;
				Enum(primitiveType);

				type = primitiveType;
				break;
			}

			case 3: //< MatrixType
			{
				std::size_t columnCount, rowCount;
				PrimitiveType primitiveType;
				SizeT(columnCount);
				SizeT(rowCount);
				Enum(primitiveType);

				type = MatrixType {
					columnCount,
					rowCount,
					primitiveType
				};
				break;
			}

			case 4: //< SamplerType
			{
				ImageType dim;
				PrimitiveType sampledType;
				bool depth = false;
				Enum(dim);
				Enum(sampledType);
				if (IsVersionGreaterOrEqual(5))
					Value(depth);

				type = SamplerType {
					dim,
					sampledType,
					depth
				};
				break;
			}

			case 5: //< StructType
			{
				std::uint32_t structIndex;
				Value(structIndex);

				type = StructType{
					structIndex
				};
				break;
			}

			case 6: //< UniformType
			{
				std::size_t structIndex;
				SizeT(structIndex);

				type = UniformType {
					StructType {
						structIndex
					}
				};
				break;
			}

			case 7: //< VectorType
			{
				std::size_t componentCount;
				PrimitiveType componentType;
				SizeT(componentCount);
				Enum(componentType);

				type = VectorType{
					componentCount,
					componentType
				};
				break;
			}

			case 8: //< ArrayType
			{
				std::uint32_t length;
				ExpressionType containedType;
				Value(length);
				Type(containedType);

				ArrayType arrayType;
				arrayType.length = length;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = std::move(containedType);

				type = std::move(arrayType);
				break;
			}

			case 9: //< Type
			{
				std::size_t containedTypeIndex;
				SizeT(containedTypeIndex);

				type = Ast::Type{
					containedTypeIndex
				};
				break;
			}

			case 10: //< FunctionType
			{
				std::size_t funcIndex;
				SizeT(funcIndex);

				type = FunctionType {
					funcIndex
				};
				break;
			}

			case 11: //< IntrinsicFunctionType
			{
				IntrinsicType intrinsicType;
				Enum(intrinsicType);

				type = IntrinsicFunctionType {
					intrinsicType
				};
				break;
			}

			case 12: //< MethodType
			{
				ExpressionType objectType;
				Type(objectType);

				std::size_t methodIndex;
				SizeT(methodIndex);

				MethodType methodType;
				methodType.objectType = std::make_unique<ContainedType>();
				methodType.objectType->type = std::move(objectType);
				methodType.methodIndex = methodIndex;

				type = std::move(methodType);
				break;
			}

			case 13: //< AliasType
			{
				std::size_t aliasIndex;
				ExpressionType containedType;
				SizeT(aliasIndex);
				Type(containedType);

				AliasType aliasType;
				aliasType.aliasIndex = aliasIndex;
				aliasType.targetType = std::make_unique<ContainedType>();
				aliasType.targetType->type = std::move(containedType);

				type = std::move(aliasType);
				break;
			}
			
			case 14: //< StorageType
			{
				std::size_t structIndex;
				SizeT(structIndex);

				type = StorageType{
					StructType {
						structIndex
					}
				};
				break;
			}

			case 15: //< DynArrayType
			{
				ExpressionType containedType;
				Type(containedType);

				DynArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = std::move(containedType);

				type = std::move(arrayType);
				break;
			}

			case 16: //< TextureType
			{
				AccessPolicy accessPolicy;
				ImageFormat format;
				ImageType dim;
				PrimitiveType baseType;
				Enum(accessPolicy);
				Enum(format);
				Enum(dim);
				Enum(baseType);

				type = TextureType{
					accessPolicy,
					format,
					dim,
					baseType
				};
				break;
			}

			case 17: //< PushConstantType
			{
				std::size_t structIndex;
				SizeT(structIndex);

				type = PushConstantType {
					StructType {
						structIndex
					}
				};
				break;
			}


			default:
				throw std::runtime_error("unexpected type index " + std::to_string(typeIndex));
		}

#ifdef NAZARA_COMPILER_GCC
#pragma GCC diagnostic pop
#endif
	}

	void ShaderAstUnserializer::Value(bool& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(double& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(float& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(std::string& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(std::int32_t& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(Vector2<bool>& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
	}

	void ShaderAstUnserializer::Value(Vector3<bool>& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
	}

	void ShaderAstUnserializer::Value(Vector4<bool>& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
		m_unserializer.Unserialize(val.w());
	}

	void ShaderAstUnserializer::Value(Vector2f32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
	}

	void ShaderAstUnserializer::Value(Vector3f32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
	}

	void ShaderAstUnserializer::Value(Vector4f32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
		m_unserializer.Unserialize(val.w());
	}

	void ShaderAstUnserializer::Value(Vector2f64& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
	}
	
	void ShaderAstUnserializer::Value(Vector3f64& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
	}

	void ShaderAstUnserializer::Value(Vector4f64& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
		m_unserializer.Unserialize(val.w());
	}

	void ShaderAstUnserializer::Value(Vector2i32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
	}

	void ShaderAstUnserializer::Value(Vector3i32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
	}

	void ShaderAstUnserializer::Value(Vector4i32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
		m_unserializer.Unserialize(val.w());
	}

	void ShaderAstUnserializer::Value(Vector2u32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
	}

	void ShaderAstUnserializer::Value(Vector3u32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
	}

	void ShaderAstUnserializer::Value(Vector4u32& val)
	{
		m_unserializer.Unserialize(val.x());
		m_unserializer.Unserialize(val.y());
		m_unserializer.Unserialize(val.z());
		m_unserializer.Unserialize(val.w());
	}

	void ShaderAstUnserializer::Value(std::uint8_t& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(std::uint16_t& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(std::uint32_t& val)
	{
		m_unserializer.Unserialize(val);
	}

	void ShaderAstUnserializer::Value(std::uint64_t& val)
	{
		m_unserializer.Unserialize(val);
	}


	void SerializeShader(AbstractSerializer& serializer, const Module& module)
	{
		ShaderAstSerializer astSerializer(serializer);
		astSerializer.Serialize(module);
	}

	ModulePtr UnserializeShader(AbstractUnserializer& unserializer)
	{
		ShaderAstUnserializer astUnserializer(unserializer);
		return astUnserializer.Unserialize();
	}
}
