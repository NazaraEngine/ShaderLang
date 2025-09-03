// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <stdexcept>

namespace nzsl::ShaderBuilder
{
	inline Ast::AccessFieldExpressionPtr Impl::AccessField::operator()(Ast::ExpressionPtr expr, std::uint32_t fieldIndex) const
	{
		auto accessFieldNode = std::make_unique<Ast::AccessFieldExpression>();
		accessFieldNode->expr = std::move(expr);
		accessFieldNode->fieldIndex = fieldIndex;

		return accessFieldNode;
	}

	inline Ast::AccessIdentifierExpressionPtr Impl::AccessMember::operator()(Ast::ExpressionPtr expr, std::string memberIdentifier, const SourceLocation& sourceLocation) const
	{
		auto accessMemberNode = std::make_unique<Ast::AccessIdentifierExpression>();
		accessMemberNode->expr = std::move(expr);
		auto& identifierEntry = accessMemberNode->identifiers.emplace_back();
		identifierEntry.identifier = std::move(memberIdentifier);
		identifierEntry.sourceLocation = sourceLocation;

		return accessMemberNode;
	}

	inline Ast::AccessIdentifierExpressionPtr Impl::AccessMember::operator()(Ast::ExpressionPtr expr, std::vector<std::string> memberIdentifiers) const
	{
		auto accessMemberNode = std::make_unique<Ast::AccessIdentifierExpression>();
		accessMemberNode->expr = std::move(expr);
		accessMemberNode->identifiers.reserve(memberIdentifiers.size());
		for (std::string& identifier : memberIdentifiers)
		{
			auto& identifierEntry = accessMemberNode->identifiers.emplace_back();
			identifierEntry.identifier = std::move(identifier);
		}

		return accessMemberNode;
	}

	inline Ast::AccessIndexExpressionPtr Impl::AccessIndex::operator()(Ast::ExpressionPtr expr, std::int32_t index) const
	{
		auto accessMemberNode = std::make_unique<Ast::AccessIndexExpression>();
		accessMemberNode->expr = std::move(expr);
		accessMemberNode->indices.push_back(ShaderBuilder::ConstantValue(index));

		return accessMemberNode;
	}

	inline Ast::AccessIndexExpressionPtr Impl::AccessIndex::operator()(Ast::ExpressionPtr expr, const std::vector<std::int32_t>& indexConstants) const
	{
		auto accessMemberNode = std::make_unique<Ast::AccessIndexExpression>();
		accessMemberNode->expr = std::move(expr);

		accessMemberNode->indices.reserve(indexConstants.size());
		for (std::int32_t index : indexConstants)
			accessMemberNode->indices.push_back(ShaderBuilder::ConstantValue(index));

		return accessMemberNode;
	}

	inline Ast::AccessIndexExpressionPtr Impl::AccessIndex::operator()(Ast::ExpressionPtr expr, Ast::ExpressionPtr indexExpression) const
	{
		auto accessMemberNode = std::make_unique<Ast::AccessIndexExpression>();
		accessMemberNode->expr = std::move(expr);
		accessMemberNode->indices.push_back(std::move(indexExpression));

		return accessMemberNode;
	}

	inline Ast::AccessIndexExpressionPtr Impl::AccessIndex::operator()(Ast::ExpressionPtr expr, std::vector<Ast::ExpressionPtr> indexExpressions) const
	{
		auto accessMemberNode = std::make_unique<Ast::AccessIndexExpression>();
		accessMemberNode->expr = std::move(expr);
		accessMemberNode->indices = std::move(indexExpressions);

		return accessMemberNode;
	}

	inline Ast::AssignExpressionPtr Impl::Assign::operator()(Ast::AssignType op, Ast::ExpressionPtr left, Ast::ExpressionPtr right) const
	{
		auto assignNode = std::make_unique<Ast::AssignExpression>();
		assignNode->op = op;
		assignNode->left = std::move(left);
		assignNode->right = std::move(right);

		return assignNode;
	}

	inline Ast::BinaryExpressionPtr Impl::Binary::operator()(Ast::BinaryType op, Ast::ExpressionPtr left, Ast::ExpressionPtr right) const
	{
		auto binaryNode = std::make_unique<Ast::BinaryExpression>();
		binaryNode->op = op;
		binaryNode->left = std::move(left);
		binaryNode->right = std::move(right);

		return binaryNode;
	}

	template<bool Const>
	Ast::BranchStatementPtr Impl::Branch<Const>::operator()(Ast::ExpressionPtr condition, Ast::StatementPtr truePath, Ast::StatementPtr falsePath) const
	{
		auto branchNode = std::make_unique<Ast::BranchStatement>();

		auto& condStatement = branchNode->condStatements.emplace_back();
		condStatement.condition = std::move(condition);
		condStatement.statement = std::move(truePath);

		branchNode->elseStatement = std::move(falsePath);
		branchNode->isConst = Const;

		return branchNode;
	}

	template<bool Const>
	Ast::BranchStatementPtr Impl::Branch<Const>::operator()(std::vector<Ast::BranchStatement::ConditionalStatement> condStatements, Ast::StatementPtr elseStatement) const
	{
		auto branchNode = std::make_unique<Ast::BranchStatement>();
		branchNode->condStatements = std::move(condStatements);
		branchNode->elseStatement = std::move(elseStatement);
		branchNode->isConst = Const;

		return branchNode;
	}

	inline Ast::CallFunctionExpressionPtr Impl::CallFunction::operator()(std::string functionName, std::vector<Ast::CallFunctionExpression::Parameter> parameters) const
	{
		auto callFunctionExpression = std::make_unique<Ast::CallFunctionExpression>();
		callFunctionExpression->targetFunction = ShaderBuilder::Identifier(std::move(functionName));
		callFunctionExpression->parameters = std::move(parameters);

		return callFunctionExpression;
	}

	inline Ast::CallFunctionExpressionPtr Impl::CallFunction::operator()(Ast::ExpressionPtr functionExpr, std::vector<Ast::CallFunctionExpression::Parameter> parameters) const
	{
		auto callFunctionExpression = std::make_unique<Ast::CallFunctionExpression>();
		callFunctionExpression->targetFunction = std::move(functionExpr);
		callFunctionExpression->parameters = std::move(parameters);

		return callFunctionExpression;
	}

	inline Ast::CastExpressionPtr Impl::Cast::operator()(Ast::ExpressionValue<Ast::ExpressionType> targetType, Ast::ExpressionPtr expression) const
	{
		auto castNode = std::make_unique<Ast::CastExpression>();
		castNode->targetType = std::move(targetType);
		castNode->expressions.push_back(std::move(expression));

		return castNode;
	}

	inline Ast::CastExpressionPtr Impl::Cast::operator()(Ast::ExpressionValue<Ast::ExpressionType> targetType, std::vector<Ast::ExpressionPtr> expressions) const
	{
		auto castNode = std::make_unique<Ast::CastExpression>();
		castNode->targetType = std::move(targetType);
		castNode->expressions = std::move(expressions);

		return castNode;
	}

	inline Ast::ConditionalExpressionPtr Impl::ConditionalExpression::operator()(Ast::ExpressionPtr condition, Ast::ExpressionPtr truePath, Ast::ExpressionPtr falsePath) const
	{
		auto condExprNode = std::make_unique<Ast::ConditionalExpression>();
		condExprNode->condition = std::move(condition);
		condExprNode->falsePath = std::move(falsePath);
		condExprNode->truePath = std::move(truePath);

		return condExprNode;
	}

	inline Ast::ConditionalStatementPtr Impl::ConditionalStatement::operator()(Ast::ExpressionPtr condition, Ast::StatementPtr statement) const
	{
		auto condStatementNode = std::make_unique<Ast::ConditionalStatement>();
		condStatementNode->condition = std::move(condition);
		condStatementNode->statement = std::move(statement);

		return condStatementNode;
	}

	inline Ast::ConstantValueExpressionPtr Impl::ConstantValue::operator()(Ast::ConstantSingleValue value) const
	{
		auto constantNode = std::make_unique<Ast::ConstantValueExpression>();
		constantNode->value = std::move(value);
		constantNode->cachedExpressionType = Ast::GetConstantType(constantNode->value);

		return constantNode;
	}

	inline Ast::ConstantValueExpressionPtr Impl::ConstantValue::operator()(Ast::ConstantSingleValue value, const SourceLocation& sourceLocation) const
	{
		auto constantNode = std::make_unique<Ast::ConstantValueExpression>();
		constantNode->value = std::move(value);
		constantNode->cachedExpressionType = Ast::GetConstantType(constantNode->value);
		constantNode->sourceLocation = sourceLocation;

		return constantNode;
	}

	template<typename T>
	Ast::ConstantValueExpressionPtr Impl::ConstantValue::operator()(Ast::ExpressionType type, T value) const
	{
		assert(IsPrimitiveType(type));

		switch (std::get<Ast::PrimitiveType>(type))
		{
			case Ast::PrimitiveType::Boolean:        return ShaderBuilder::ConstantValue(value != T(0));
			case Ast::PrimitiveType::Float32:        return ShaderBuilder::ConstantValue(Nz::SafeCast<float>(value));
			case Ast::PrimitiveType::Float64:        return ShaderBuilder::ConstantValue(Nz::SafeCast<double>(value));
			case Ast::PrimitiveType::Int32:          return ShaderBuilder::ConstantValue(Nz::SafeCast<std::int32_t>(value));
			case Ast::PrimitiveType::UInt32:         return ShaderBuilder::ConstantValue(Nz::SafeCast<std::uint32_t>(value));
			case Ast::PrimitiveType::String:         return ShaderBuilder::ConstantValue(value);
			case Ast::PrimitiveType::FloatLiteral:   return ShaderBuilder::ConstantValue(Ast::FloatLiteral{ Nz::SafeCast<double>(value) });
			case Ast::PrimitiveType::IntLiteral:     return ShaderBuilder::ConstantValue(Ast::IntLiteral{ Nz::SafeCast<std::int64_t>(value) });
		}

		throw std::runtime_error("unexpected primitive type");
	}

	template<typename T>
	Ast::ConstantValueExpressionPtr Impl::ConstantValue::operator()(Ast::ExpressionType type, T value, const SourceLocation& sourceLocation) const
	{
		assert(IsPrimitiveType(type));

		switch (std::get<Ast::PrimitiveType>(type))
		{
			case Ast::PrimitiveType::Boolean:        return ShaderBuilder::ConstantValue(value != T(0), sourceLocation);
			case Ast::PrimitiveType::Float32:        return ShaderBuilder::ConstantValue(Nz::SafeCast<float>(value), sourceLocation);
			case Ast::PrimitiveType::Float64:        return ShaderBuilder::ConstantValue(Nz::SafeCast<double>(value), sourceLocation);
			case Ast::PrimitiveType::Int32:          return ShaderBuilder::ConstantValue(Nz::SafeCast<std::int32_t>(value), sourceLocation);
			case Ast::PrimitiveType::UInt32:         return ShaderBuilder::ConstantValue(Nz::SafeCast<std::uint32_t>(value), sourceLocation);
			case Ast::PrimitiveType::String:         return ShaderBuilder::ConstantValue(value);
			case Ast::PrimitiveType::FloatLiteral:   return ShaderBuilder::ConstantValue(Ast::FloatLiteral{ Nz::SafeCast<double>(value) });
			case Ast::PrimitiveType::IntLiteral:     return ShaderBuilder::ConstantValue(Ast::IntLiteral{ Nz::SafeCast<std::int64_t>(value) });
		}

		throw std::runtime_error("unexpected primitive type");
	}

	inline Ast::ConstantArrayValueExpressionPtr Impl::ConstantArrayValue::operator()(Ast::ConstantArrayValue values) const
	{
		auto constantNode = std::make_unique<Ast::ConstantArrayValueExpression>();
		constantNode->values = std::move(values);
		constantNode->cachedExpressionType = Ast::GetConstantType(constantNode->values);

		return constantNode;
	}

	inline Ast::DeclareAliasStatementPtr Impl::DeclareAlias::operator()(std::string name, Ast::ExpressionPtr expression) const
	{
		auto declareAliasNode = std::make_unique<Ast::DeclareAliasStatement>();
		declareAliasNode->name = std::move(name);
		declareAliasNode->expression = std::move(expression);

		return declareAliasNode;
	}

	inline Ast::DeclareConstStatementPtr Impl::DeclareConst::operator()(std::string name, Ast::ExpressionPtr initialValue) const
	{
		auto declareConstNode = std::make_unique<Ast::DeclareConstStatement>();
		declareConstNode->name = std::move(name);
		declareConstNode->expression = std::move(initialValue);

		return declareConstNode;
	}

	inline Ast::DeclareConstStatementPtr Impl::DeclareConst::operator()(std::string name, Ast::ExpressionValue<Ast::ExpressionType> type, Ast::ExpressionPtr initialValue) const
	{
		auto declareConstNode = std::make_unique<Ast::DeclareConstStatement>();
		declareConstNode->name = std::move(name);
		declareConstNode->type = std::move(type);
		declareConstNode->expression = std::move(initialValue);

		return declareConstNode;
	}

	inline Ast::DeclareFunctionStatementPtr Impl::DeclareFunction::operator()(std::string name, Ast::StatementPtr statement) const
	{
		auto declareFunctionNode = std::make_unique<Ast::DeclareFunctionStatement>();
		declareFunctionNode->name = std::move(name);
		declareFunctionNode->statements.push_back(std::move(statement));

		return declareFunctionNode;
	}

	inline Ast::DeclareFunctionStatementPtr Impl::DeclareFunction::operator()(std::string name, std::vector<Ast::DeclareFunctionStatement::Parameter> parameters, std::vector<Ast::StatementPtr> statements, Ast::ExpressionValue<Ast::ExpressionType> returnType) const
	{
		auto declareFunctionNode = std::make_unique<Ast::DeclareFunctionStatement>();
		declareFunctionNode->name = std::move(name);
		declareFunctionNode->parameters = std::move(parameters);
		declareFunctionNode->returnType = std::move(returnType);
		declareFunctionNode->statements = std::move(statements);

		return declareFunctionNode;
	}

	inline Ast::DeclareFunctionStatementPtr Impl::DeclareFunction::operator()(std::optional<ShaderStageType> entryStage, std::string name, Ast::StatementPtr statement) const
	{
		auto declareFunctionNode = std::make_unique<Ast::DeclareFunctionStatement>();
		declareFunctionNode->name = std::move(name);
		declareFunctionNode->statements.push_back(std::move(statement));

		if (entryStage)
			declareFunctionNode->entryStage = *entryStage;

		return declareFunctionNode;
	}

	inline Ast::DeclareFunctionStatementPtr Impl::DeclareFunction::operator()(std::optional<ShaderStageType> entryStage, std::string name, std::vector<Ast::DeclareFunctionStatement::Parameter> parameters, std::vector<Ast::StatementPtr> statements, Ast::ExpressionValue<Ast::ExpressionType> returnType) const
	{
		auto declareFunctionNode = std::make_unique<Ast::DeclareFunctionStatement>();
		declareFunctionNode->name = std::move(name);
		declareFunctionNode->parameters = std::move(parameters);
		declareFunctionNode->returnType = std::move(returnType);
		declareFunctionNode->statements = std::move(statements);

		if (entryStage)
			declareFunctionNode->entryStage = *entryStage;

		return declareFunctionNode;
	}

	inline Ast::DeclareOptionStatementPtr Impl::DeclareOption::operator()(std::string name, Ast::ExpressionValue<Ast::ExpressionType> type, Ast::ExpressionPtr initialValue) const
	{
		auto declareOptionNode = std::make_unique<Ast::DeclareOptionStatement>();
		declareOptionNode->optName = std::move(name);
		declareOptionNode->optType = std::move(type);
		declareOptionNode->defaultValue = std::move(initialValue);

		return declareOptionNode;
	}

	inline Ast::DeclareStructStatementPtr Impl::DeclareStruct::operator()(Ast::StructDescription description, Ast::ExpressionValue<bool> isExported) const
	{
		auto declareStructNode = std::make_unique<Ast::DeclareStructStatement>();
		declareStructNode->description = std::move(description);
		declareStructNode->isExported = std::move(isExported);

		return declareStructNode;
	}

	inline Ast::DeclareVariableStatementPtr Impl::DeclareVariable::operator()(std::string name, Ast::ExpressionPtr initialValue) const
	{
		auto declareVariableNode = std::make_unique<Ast::DeclareVariableStatement>();
		declareVariableNode->varName = std::move(name);
		declareVariableNode->initialExpression = std::move(initialValue);

		return declareVariableNode;
	}

	inline Ast::DeclareVariableStatementPtr Impl::DeclareVariable::operator()(std::string name, Ast::ExpressionValue<Ast::ExpressionType> type, Ast::ExpressionPtr initialValue) const
	{
		auto declareVariableNode = std::make_unique<Ast::DeclareVariableStatement>();
		declareVariableNode->varName = std::move(name);
		declareVariableNode->varType = std::move(type);
		declareVariableNode->initialExpression = std::move(initialValue);

		return declareVariableNode;
	}

	inline Ast::ExpressionStatementPtr Impl::ExpressionStatement::operator()(Ast::ExpressionPtr expression) const
	{
		auto expressionStatementNode = std::make_unique<Ast::ExpressionStatement>();
		expressionStatementNode->sourceLocation = expression->sourceLocation;
		expressionStatementNode->expression = std::move(expression);

		return expressionStatementNode;
	}

	inline Ast::ForStatementPtr Impl::For::operator()(std::string varName, Ast::ExpressionPtr fromExpression, Ast::ExpressionPtr toExpression, Ast::StatementPtr statement) const
	{
		auto forNode = std::make_unique<Ast::ForStatement>();
		forNode->fromExpr = std::move(fromExpression);
		forNode->statement = std::move(statement);
		forNode->toExpr = std::move(toExpression);
		forNode->varName = std::move(varName);

		return forNode;
	}

	inline Ast::ForStatementPtr Impl::For::operator()(std::string varName, Ast::ExpressionPtr fromExpression, Ast::ExpressionPtr toExpression, Ast::ExpressionPtr stepExpression, Ast::StatementPtr statement) const
	{
		auto forNode = std::make_unique<Ast::ForStatement>();
		forNode->fromExpr = std::move(fromExpression);
		forNode->statement = std::move(statement);
		forNode->stepExpr = std::move(stepExpression);
		forNode->toExpr = std::move(toExpression);
		forNode->varName = std::move(varName);

		return forNode;
	}

	Ast::ForEachStatementPtr Impl::ForEach::operator()(std::string varName, Ast::ExpressionPtr expression, Ast::StatementPtr statement) const
	{
		auto forEachNode = std::make_unique<Ast::ForEachStatement>();
		forEachNode->expression = std::move(expression);
		forEachNode->statement = std::move(statement);
		forEachNode->varName = std::move(varName);

		return forEachNode;
	}

	inline Ast::IdentifierExpressionPtr Impl::Identifier::operator()(std::string name) const
	{
		auto identifierNode = std::make_unique<Ast::IdentifierExpression>();
		identifierNode->identifier = std::move(name);

		return identifierNode;
	}

	inline Ast::IdentifierValueExpressionPtr Impl::IdentifierValue::operator()(Ast::IdentifierType type, std::size_t id) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = type;
		identifierValue->identifierIndex = id;

		switch (type)
		{
			case Ast::IdentifierType::ExternalBlock: identifierValue->cachedExpressionType = Ast::NamedExternalBlockType{ id }; break;
			case Ast::IdentifierType::Function: identifierValue->cachedExpressionType = Ast::FunctionType{ id }; break;
			case Ast::IdentifierType::Module: identifierValue->cachedExpressionType = Ast::ModuleType{ id }; break;
			case Ast::IdentifierType::Struct: identifierValue->cachedExpressionType = Ast::StructType{ id }; break;
			case Ast::IdentifierType::Type: identifierValue->cachedExpressionType = Ast::Type{ id }; break;
			default: break;
		}

		return identifierValue;
	}

	inline Ast::IdentifierValueExpressionPtr Impl::IdentifierValue::operator()(Ast::IdentifierType type, std::size_t id, const SourceLocation& sourceLocation) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = type;
		identifierValue->identifierIndex = id;
		identifierValue->sourceLocation = sourceLocation;
		
		switch (type)
		{
			case Ast::IdentifierType::ExternalBlock: identifierValue->cachedExpressionType = Ast::NamedExternalBlockType{ id }; break;
			case Ast::IdentifierType::Function: identifierValue->cachedExpressionType = Ast::FunctionType{ id }; break;
			case Ast::IdentifierType::Module: identifierValue->cachedExpressionType = Ast::ModuleType{ id }; break;
			case Ast::IdentifierType::Struct: identifierValue->cachedExpressionType = Ast::StructType{ id }; break;
			case Ast::IdentifierType::Type: identifierValue->cachedExpressionType = Ast::Type{ id }; break;
			default: break;
		}

		return identifierValue;
	}

	inline Ast::IdentifierValueExpressionPtr Impl::IdentifierValue::operator()(Ast::IdentifierType type, std::size_t id, Ast::ExpressionType expressionType) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = type;
		identifierValue->identifierIndex = id;
		identifierValue->cachedExpressionType = std::move(expressionType);

		return identifierValue;
	}

	inline Ast::IdentifierValueExpressionPtr Impl::IdentifierValue::operator()(Ast::IdentifierType type, std::size_t id, Ast::ExpressionType expressionType, const SourceLocation& sourceLocation) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = type;
		identifierValue->identifierIndex = id;
		identifierValue->cachedExpressionType = std::move(expressionType);
		identifierValue->sourceLocation = sourceLocation;

		return identifierValue;
	}

	template<Ast::IdentifierType Type>
	Ast::IdentifierValueExpressionPtr Impl::IdentifierValueWithType<Type>::operator()(std::size_t id) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = Type;
		identifierValue->identifierIndex = id;
		
		switch (Type)
		{
			case Ast::IdentifierType::ExternalBlock: identifierValue->cachedExpressionType = Ast::NamedExternalBlockType{ id }; break;
			case Ast::IdentifierType::Function: identifierValue->cachedExpressionType = Ast::FunctionType{ id }; break;
			case Ast::IdentifierType::Module: identifierValue->cachedExpressionType = Ast::ModuleType{ id }; break;
			case Ast::IdentifierType::Struct: identifierValue->cachedExpressionType = Ast::StructType{ id }; break;
			case Ast::IdentifierType::Type: identifierValue->cachedExpressionType = Ast::Type{ id }; break;
			default: break;
		}

		return identifierValue;
	}

	template<Ast::IdentifierType Type>
	Ast::IdentifierValueExpressionPtr Impl::IdentifierValueWithType<Type>::operator()(std::size_t id, const SourceLocation& sourceLocation) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = Type;
		identifierValue->identifierIndex = id;
		identifierValue->sourceLocation = sourceLocation;
		
		switch (Type)
		{
			case Ast::IdentifierType::ExternalBlock: identifierValue->cachedExpressionType = Ast::NamedExternalBlockType{ id }; break;
			case Ast::IdentifierType::Function: identifierValue->cachedExpressionType = Ast::FunctionType{ id }; break;
			case Ast::IdentifierType::Module: identifierValue->cachedExpressionType = Ast::ModuleType{ id }; break;
			case Ast::IdentifierType::Struct: identifierValue->cachedExpressionType = Ast::StructType{ id }; break;
			case Ast::IdentifierType::Type: identifierValue->cachedExpressionType = Ast::Type{ id }; break;
			default: break;
		}

		return identifierValue;
	}

	template<Ast::IdentifierType Type>
	Ast::IdentifierValueExpressionPtr Impl::IdentifierValueWithType<Type>::operator()(std::size_t id, Ast::ExpressionType expressionType) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = Type;
		identifierValue->identifierIndex = id;
		identifierValue->cachedExpressionType = std::move(expressionType);

		return identifierValue;
	}

	template<Ast::IdentifierType Type>
	Ast::IdentifierValueExpressionPtr Impl::IdentifierValueWithType<Type>::operator()(std::size_t id, Ast::ExpressionType expressionType, const SourceLocation& sourceLocation) const
	{
		auto identifierValue = std::make_unique<Ast::IdentifierValueExpression>();
		identifierValue->identifierType = Type;
		identifierValue->identifierIndex = id;
		identifierValue->cachedExpressionType = std::move(expressionType);
		identifierValue->sourceLocation = sourceLocation;

		return identifierValue;
	}

	inline Ast::ImportStatementPtr Impl::Import::operator()(std::string modulePath, std::string moduleIdentifier) const
	{
		auto importNode = std::make_unique<Ast::ImportStatement>();
		importNode->moduleName = std::move(modulePath);
		importNode->moduleIdentifier = std::move(moduleIdentifier);

		return importNode;
	}

	inline Ast::ImportStatementPtr Impl::Import::operator()(std::string moduleName, std::vector<Ast::ImportStatement::Identifier> identifiers) const
	{
		auto importNode = std::make_unique<Ast::ImportStatement>();
		importNode->moduleName = std::move(moduleName);
		importNode->identifiers = std::move(identifiers);

		return importNode;
	}

	inline Ast::IntrinsicExpressionPtr Impl::Intrinsic::operator()(Ast::IntrinsicType intrinsicType, std::vector<Ast::ExpressionPtr> parameters) const
	{
		auto intrinsicExpression = std::make_unique<Ast::IntrinsicExpression>();
		intrinsicExpression->intrinsic = intrinsicType;
		intrinsicExpression->parameters = std::move(parameters);

		return intrinsicExpression;
	}

	inline Ast::MultiStatementPtr Impl::Multi::operator()(std::vector<Ast::StatementPtr> statements) const
	{
		auto multiStatement = std::make_unique<Ast::MultiStatement>();
		multiStatement->statements = std::move(statements);

		return multiStatement;
	}

	template<typename T>
	std::unique_ptr<T> Impl::NoParam<T>::operator()() const
	{
		return std::make_unique<T>();
	}

	inline Ast::ReturnStatementPtr Impl::Return::operator()(Ast::ExpressionPtr expr) const
	{
		auto returnNode = std::make_unique<Ast::ReturnStatement>();
		returnNode->returnExpr = std::move(expr);

		return returnNode;
	}

	inline Ast::ScopedStatementPtr Impl::Scoped::operator()(Ast::StatementPtr statement) const
	{
		auto scopedNode = std::make_unique<Ast::ScopedStatement>();
		scopedNode->sourceLocation = statement->sourceLocation;
		scopedNode->statement = std::move(statement);

		return scopedNode;
	}

	inline Ast::SwizzleExpressionPtr Impl::Swizzle::operator()(Ast::ExpressionPtr expression, std::array<std::uint32_t, 4> swizzleComponents, std::size_t componentCount) const
	{
		assert(componentCount > 0);
		assert(componentCount <= 4);

		auto swizzleNode = std::make_unique<Ast::SwizzleExpression>();
		swizzleNode->expression = std::move(expression);
		swizzleNode->componentCount = componentCount;
		swizzleNode->components = swizzleComponents;

		return swizzleNode;
	}

	inline Ast::SwizzleExpressionPtr Impl::Swizzle::operator()(Ast::ExpressionPtr expression, std::vector<std::uint32_t> swizzleComponents) const
	{
		auto swizzleNode = std::make_unique<Ast::SwizzleExpression>();
		swizzleNode->expression = std::move(expression);

		assert(swizzleComponents.size() <= swizzleNode->components.size());
		swizzleNode->componentCount = swizzleComponents.size();
		for (std::size_t i = 0; i < swizzleNode->componentCount; ++i)
		{
			assert(swizzleComponents[i] <= 4);
			swizzleNode->components[i] = swizzleComponents[i];
		}

		return swizzleNode;
	}

	inline Ast::TypeConstantExpressionPtr Impl::TypeConstant::operator()(Ast::ExpressionType expressionType, Ast::TypeConstant typeConstant) const
	{
		auto typeConstantNode = std::make_unique<Ast::TypeConstantExpression>();
		typeConstantNode->type = std::move(expressionType);
		typeConstantNode->typeConstant = typeConstant;

		typeConstantNode->cachedExpressionType = typeConstantNode->type;

		return typeConstantNode;
	}

	inline Ast::UnaryExpressionPtr Impl::Unary::operator()(Ast::UnaryType op, Ast::ExpressionPtr expression) const
	{
		auto unaryNode = std::make_unique<Ast::UnaryExpression>();
		unaryNode->expression = std::move(expression);
		unaryNode->op = op;

		return unaryNode;
	}

	inline Ast::WhileStatementPtr Impl::While::operator()(Ast::ExpressionPtr condition, Ast::StatementPtr body) const
	{
		auto whileNode = std::make_unique<Ast::WhileStatement>();
		whileNode->condition = std::move(condition);
		whileNode->body = std::move(body);

		return whileNode;
	}
}
