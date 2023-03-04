// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Compare.hpp>
#include <stdexcept>

namespace nzsl::Ast
{
	inline bool Compare(const Expression& lhs, const Expression& rhs, const ComparisonParams& params)
	{
		if (lhs.GetType() != rhs.GetType())
			return false;

		if (params.compareSourceLoc && !Compare(lhs.sourceLocation, rhs.sourceLocation, params))
			return false;

		switch (lhs.GetType())
		{
			case NodeType::None: break;

#define NZSL_SHADERAST_EXPRESSION(Node) case NodeType::Node##Expression: return Compare(static_cast<const Node##Expression&>(lhs), static_cast<const Node##Expression&>(lhs), params);
#include <NZSL/Ast/NodeList.hpp>

			default: throw std::runtime_error("unexpected node type");
		}

		return true;
	}

	bool Compare(const Module& lhs, const Module& rhs, const ComparisonParams& params)
	{
		if (!Compare(*lhs.metadata, *rhs.metadata, params))
			return false;

		if (!Compare(lhs.importedModules, rhs.importedModules, params))
			return false;

		if (!Compare(*lhs.rootNode, *rhs.rootNode, params))
			return false;

		return true;
	}

	bool Compare(const Module::ImportedModule& lhs, const Module::ImportedModule& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.identifier, rhs.identifier, params))
			return false;

		if (!Compare(*lhs.module, *rhs.module, params))
			return false;

		return true;
	}

	bool Compare(const Module::Metadata& lhs, const Module::Metadata& rhs, const ComparisonParams& params)
	{
		if (params.compareModuleName && !Compare(lhs.moduleName, rhs.moduleName, params))
			return false;

		if (!Compare(lhs.shaderLangVersion, rhs.shaderLangVersion, params))
			return false;

		if (!Compare(lhs.enabledFeatures, rhs.enabledFeatures, params))
			return false;

		if (!Compare(lhs.author, rhs.author, params))
			return false;

		if (!Compare(lhs.description, rhs.description, params))
			return false;

		if (!Compare(lhs.license, rhs.license, params))
			return false;

		return true;
	}

	inline bool Compare(const Statement& lhs, const Statement& rhs, const ComparisonParams& params)
	{
		if (lhs.GetType() != rhs.GetType())
			return false;

		if (params.compareSourceLoc && !Compare(lhs.sourceLocation, rhs.sourceLocation, params))
			return false;

		switch (lhs.GetType())
		{
			case NodeType::None: break;

#define NZSL_SHADERAST_STATEMENT(Node) case NodeType::Node##Statement: return Compare(static_cast<const Node##Statement&>(lhs), static_cast<const Node##Statement&>(lhs), params);
#include <NZSL/Ast/NodeList.hpp>

			default: throw std::runtime_error("unexpected node type");
		}

		return false;
	}

	template<typename T>
	bool Compare(const T& lhs, const T& rhs, const ComparisonParams& /*params*/)
	{
		return lhs == rhs;
	}

	template<typename T, std::size_t S>
	bool Compare(const std::array<T, S>& lhs, const std::array<T, S>& rhs, const ComparisonParams& params)
	{
		for (std::size_t i = 0; i < S; ++i)
		{
			if (!Compare(lhs[i], rhs[i], params))
				return false;
		}

		return true;
	}

	template<typename T>
	bool Compare(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs, const ComparisonParams& params)
	{
		if (lhs == nullptr)
			return rhs == nullptr;
		else if (rhs == nullptr)
			return false;

		return Compare(*lhs, *rhs, params);
	}

	template<typename T>
	bool Compare(const std::vector<T>& lhs, const std::vector<T>& rhs, const ComparisonParams& params)
	{
		if (lhs.size() != rhs.size())
			return false;

		for (std::size_t i = 0; i < lhs.size(); ++i)
		{
			if (!Compare(lhs[i], rhs[i], params))
				return false;
		}

		return true;
	}

	template<typename T>
	bool Compare(const std::unique_ptr<T>& lhs, const std::unique_ptr<T>& rhs, const ComparisonParams& params)
	{
		if (lhs == nullptr)
			return rhs == nullptr;
		else if (rhs == nullptr)
			return false;

		return Compare(*lhs, *rhs, params);
	}

	template<typename T>
	bool Compare(const ExpressionValue<T>& lhs, const ExpressionValue<T>& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.HasValue(), rhs.HasValue(), params))
			return false;

		if (!Compare(lhs.IsResultingValue(), rhs.IsResultingValue(), params))
			return false;

		if (!Compare(lhs.IsExpression(), rhs.IsExpression(), params))
			return false;

		if (lhs.IsExpression())
		{
			if (!Compare(lhs.GetExpression(), rhs.GetExpression(), params))
				return false;
		}
		else if (lhs.IsResultingValue())
		{
			if (!Compare(lhs.GetResultingValue(), rhs.GetResultingValue(), params))
				return false;
		}

		return true;
	}

	bool Compare(const AccessIdentifierExpression::Identifier& lhs, const AccessIdentifierExpression::Identifier& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.identifier, rhs.identifier, params))
			return false;

		if (!Compare(lhs.sourceLocation, rhs.sourceLocation, params))
			return false;

		return true;
	}

	inline bool Compare(const BranchStatement::ConditionalStatement& lhs, const BranchStatement::ConditionalStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.condition, rhs.condition, params))
			return false;

		if (!Compare(lhs.statement, rhs.statement, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareExternalStatement::ExternalVar& lhs, const DeclareExternalStatement::ExternalVar& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.bindingIndex, rhs.bindingIndex, params))
			return false;

		if (!Compare(lhs.bindingSet, rhs.bindingSet, params))
			return false;

		if (!Compare(lhs.name, rhs.name, params))
			return false;

		if (!Compare(lhs.type, rhs.type, params))
			return false;

		if (!Compare(lhs.sourceLocation, rhs.sourceLocation, params))
			return false;

		if (!Compare(lhs.tag, rhs.tag, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareFunctionStatement::Parameter& lhs, const DeclareFunctionStatement::Parameter& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.name, rhs.name, params))
			return false;

		if (!Compare(lhs.type, rhs.type, params))
			return false;

		if (!Compare(lhs.sourceLocation, rhs.sourceLocation, params))
			return false;

		return true;
	}
	
	inline bool Compare(const ImportStatement::Identifier& lhs, const ImportStatement::Identifier& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.identifier, rhs.identifier, params))
			return false;

		if (!Compare(lhs.identifierLoc, rhs.identifierLoc, params))
			return false;

		if (!Compare(lhs.renamedIdentifier, rhs.renamedIdentifier, params))
			return false;

		if (!Compare(lhs.renamedIdentifierLoc, rhs.renamedIdentifierLoc, params))
			return false;
		
		return true;
	}

	inline bool Compare(const SourceLocation& lhs, const SourceLocation& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.endColumn, rhs.endColumn, params))
			return false;

		if (!Compare(lhs.endLine, rhs.endLine, params))
			return false;

		if (!Compare(lhs.startColumn, rhs.startColumn, params))
			return false;

		if (!Compare(lhs.startLine, rhs.startLine, params))
			return false;

		if (!Compare(lhs.file, rhs.file, params))
			return false;

		return true;
	}

	inline bool Compare(const StructDescription& lhs, const StructDescription& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.layout, rhs.layout, params))
			return false;

		if (!Compare(lhs.name, rhs.name, params))
			return false;

		if (!Compare(lhs.tag, rhs.tag, params))
			return false;

		if (!Compare(lhs.members, rhs.members, params))
			return false;

		return true;
	}

	inline bool Compare(const StructDescription::StructMember& lhs, const StructDescription::StructMember& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.builtin, rhs.builtin, params))
			return false;

		if (!Compare(lhs.cond, rhs.cond, params))
			return false;

		if (!Compare(lhs.locationIndex, rhs.locationIndex, params))
			return false;

		if (!Compare(lhs.name, rhs.name, params))
			return false;

		if (!Compare(lhs.type, rhs.type, params))
			return false;

		if (!Compare(lhs.sourceLocation, rhs.sourceLocation, params))
			return false;

		if (!Compare(lhs.tag, rhs.tag, params))
			return false;

		return true;
	}

	inline bool Compare(const AccessIdentifierExpression& lhs, const AccessIdentifierExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(*lhs.expr, *rhs.expr, params))
			return false;

		if (!Compare(lhs.identifiers, rhs.identifiers, params))
			return false;

		return true;
	}

	inline bool Compare(const AccessIndexExpression& lhs, const AccessIndexExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(*lhs.expr, *rhs.expr, params))
			return false;

		if (!Compare(lhs.indices, rhs.indices, params))
			return false;

		return true;
	}

	bool Compare(const AliasValueExpression& lhs, const AliasValueExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.aliasId, rhs.aliasId, params))
			return false;

		return true;
	}

	inline bool Compare(const AssignExpression& lhs, const AssignExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.op, rhs.op, params))
			return false;

		if (!Compare(lhs.left, rhs.left, params))
			return false;

		if (!Compare(lhs.right, rhs.right, params))
			return false;

		return true;
	}

	inline bool Compare(const BinaryExpression& lhs, const BinaryExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.op, rhs.op, params))
			return false;

		if (!Compare(lhs.left, rhs.left, params))
			return false;

		if (!Compare(lhs.right, rhs.right, params))
			return false;

		return true;
	}

	inline bool Compare(const CallFunctionExpression& lhs, const CallFunctionExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.targetFunction, rhs.targetFunction, params))
			return false;

		if (!Compare(lhs.parameters, rhs.parameters, params))
			return false;

		return true;
	}

	inline bool Compare(const CallMethodExpression& lhs, const CallMethodExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.methodName, rhs.methodName, params))
			return false;

		if (!Compare(lhs.object, rhs.object, params))
			return false;

		if (!Compare(lhs.parameters, rhs.parameters, params))
			return false;

		return true;
	}

	inline bool Compare(const CastExpression& lhs, const CastExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.targetType, rhs.targetType, params))
			return false;

		if (!Compare(lhs.expressions, rhs.expressions, params))
			return false;

		return true;
	}

	inline bool Compare(const ConditionalExpression& lhs, const ConditionalExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.condition, rhs.condition, params))
			return false;

		if (!Compare(lhs.truePath, rhs.truePath, params))
			return false;

		if (!Compare(lhs.falsePath, rhs.falsePath, params))
			return false;

		return true;
	}

	inline bool Compare(const ConstantExpression& lhs, const ConstantExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.constantId, rhs.constantId, params))
			return false;

		return true;
	}
	
	bool Compare(const ConstantArrayValueExpression& lhs, const ConstantArrayValueExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.values, rhs.values, params))
			return false;

		return true;
	}

	inline bool Compare(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.value, rhs.value, params))
			return false;

		return true;
	}

	inline bool Compare(const FunctionExpression& lhs, const FunctionExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.funcId, rhs.funcId, params))
			return false;

		return true;
	}

	inline bool Compare(const IdentifierExpression& lhs, const IdentifierExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.identifier, rhs.identifier, params))
			return false;

		return true;
	}

	inline bool Compare(const IntrinsicExpression& lhs, const IntrinsicExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.intrinsic, rhs.intrinsic, params))
			return false;

		if (!Compare(lhs.parameters, rhs.parameters, params))
			return false;

		return true;
	}

	inline bool Compare(const IntrinsicFunctionExpression& lhs, const IntrinsicFunctionExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.intrinsicId, rhs.intrinsicId, params))
			return false;

		return true;
	}

	inline bool Compare(const StructTypeExpression& lhs, const StructTypeExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.structTypeId, rhs.structTypeId, params))
			return false;

		return true;
	}

	inline bool Compare(const SwizzleExpression& lhs, const SwizzleExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.componentCount, rhs.componentCount, params))
			return false;

		if (!Compare(lhs.expression, rhs.expression, params))
			return false;

		if (!Compare(lhs.components, rhs.components, params))
			return false;

		return true;
	}

	bool Compare(const TypeExpression& lhs, const TypeExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.typeId, rhs.typeId, params))
			return false;

		return true;
	}

	inline bool Compare(const VariableValueExpression& lhs, const VariableValueExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.variableId, rhs.variableId, params))
			return false;

		return true;
	}

	inline bool Compare(const UnaryExpression& lhs, const UnaryExpression& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.op, rhs.op, params))
			return false;

		if (!Compare(lhs.expression, rhs.expression, params))
			return false;

		return true;
	}

	inline bool Compare(const BranchStatement& lhs, const BranchStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.isConst, rhs.isConst, params))
			return false;

		if (!Compare(lhs.elseStatement, rhs.elseStatement, params))
			return false;

		if (!Compare(lhs.condStatements, rhs.condStatements, params))
			return false;

		return true;
	}

	bool Compare(const BreakStatement& /*lhs*/, const BreakStatement& /*rhs*/, const ComparisonParams& /*params*/)
	{
		return true;
	}

	inline bool Compare(const ConditionalStatement& lhs, const ConditionalStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.condition, rhs.condition, params))
			return false;

		if (!Compare(lhs.statement, rhs.statement, params))
			return false;

		return true;
	}

	bool Compare(const ContinueStatement& /*lhs*/, const ContinueStatement& /*rhs*/, const ComparisonParams& /*params*/)
	{
		return true;
	}

	bool Compare(const DeclareAliasStatement& lhs, const DeclareAliasStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.name, rhs.name, params))
			return false;

		if (!Compare(lhs.expression, rhs.expression, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareConstStatement& lhs, const DeclareConstStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.name, rhs.name, params))
			return false;

		if (!Compare(lhs.type, rhs.type, params))
			return false;

		if (!Compare(lhs.expression, rhs.expression, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareExternalStatement& lhs, const DeclareExternalStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.bindingSet, rhs.bindingSet, params))
			return false;

		if (!Compare(lhs.autoBinding, rhs.autoBinding, params))
			return false;

		if (!Compare(lhs.tag, rhs.tag, params))
			return false;

		if (!Compare(lhs.externalVars, rhs.externalVars, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareFunctionStatement& lhs, const DeclareFunctionStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.depthWrite, rhs.depthWrite, params))
			return false;

		if (!Compare(lhs.earlyFragmentTests, rhs.earlyFragmentTests, params))
			return false;

		if (!Compare(lhs.entryStage, rhs.entryStage, params))
			return false;

		if (!Compare(lhs.isExported, rhs.isExported, params))
			return false;

		if (!Compare(lhs.name, rhs.name, params))
			return false;

		if (!Compare(lhs.parameters, rhs.parameters, params))
			return false;

		if (!Compare(lhs.returnType, rhs.returnType, params))
			return false;

		if (!Compare(lhs.statements, rhs.statements, params))
			return false;

		if (!Compare(lhs.workgroupSize, rhs.workgroupSize, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareOptionStatement& lhs, const DeclareOptionStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.optName, rhs.optName, params))
			return false;

		if (!Compare(lhs.optType, rhs.optType, params))
			return false;

		if (!Compare(lhs.defaultValue, rhs.defaultValue, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareStructStatement& lhs, const DeclareStructStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.description, rhs.description, params))
			return false;

		return true;
	}

	inline bool Compare(const DeclareVariableStatement& lhs, const DeclareVariableStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.varName, rhs.varName, params))
			return false;

		if (!Compare(lhs.varType, rhs.varType, params))
			return false;

		if (!Compare(lhs.initialExpression, rhs.initialExpression, params))
			return false;

		return true;
	}

	inline bool Compare(const DiscardStatement& /*lhs*/, const DiscardStatement& /*rhs*/, const ComparisonParams& /*params*/)
	{
		return true;
	}

	inline bool Compare(const ExpressionStatement& lhs, const ExpressionStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.expression, rhs.expression, params))
			return false;

		return true;
	}

	bool Compare(const ForStatement& lhs, const ForStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.varName, rhs.varName, params))
			return false;

		if (!Compare(lhs.unroll, rhs.unroll, params))
			return false;

		if (!Compare(lhs.fromExpr, rhs.fromExpr, params))
			return false;

		if (!Compare(lhs.toExpr, rhs.toExpr, params))
			return false;

		if (!Compare(lhs.stepExpr, rhs.stepExpr, params))
			return false;

		if (!Compare(lhs.statement, rhs.statement, params))
			return false;

		return true;
	}

	bool Compare(const ForEachStatement& lhs, const ForEachStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.varName, rhs.varName, params))
			return false;

		if (!Compare(lhs.unroll, rhs.unroll, params))
			return false;

		if (!Compare(lhs.expression, rhs.expression, params))
			return false;

		if (!Compare(lhs.statement, rhs.statement, params))
			return false;

		return true;
	}

	bool Compare(const ImportStatement& lhs, const ImportStatement& rhs, const ComparisonParams& params)
	{
		if (params.compareModuleName && !Compare(lhs.moduleName, rhs.moduleName, params))
			return false;

		if (!Compare(lhs.identifiers, rhs.identifiers, params))
			return false;

		return true;
	}

	inline bool Compare(const MultiStatement& lhs, const MultiStatement& rhs, const ComparisonParams& params)
	{
		auto it = lhs.statements.begin();
		auto it2 = rhs.statements.begin();
		while (it != lhs.statements.end() || it2 != rhs.statements.end())
		{
			if (params.ignoreNoOp)
			{
				while (it != lhs.statements.end() && (*it)->GetType() == NodeType::NoOpStatement)
					++it;

				while (it2 != rhs.statements.end() && (*it2)->GetType() == NodeType::NoOpStatement)
					++it2;
			}

			if (it == lhs.statements.end())
			{
				if (it2 != rhs.statements.end())
					return false;

				break;
			}
			
			if (it2 == rhs.statements.end())
				return false;

			if (!Compare(*it, *it2, params))
				return false;

			++it;
			++it2;
		}

		return true;
	}

	inline bool Compare(const NoOpStatement& /*lhs*/, const NoOpStatement& /*rhs*/, const ComparisonParams& /*params*/)
	{
		return true;
	}

	inline bool Compare(const ReturnStatement& lhs, const ReturnStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.returnExpr, rhs.returnExpr, params))
			return false;

		return true;
	}

	bool Compare(const ScopedStatement& lhs, const ScopedStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.statement, rhs.statement, params))
			return false;

		return true;
	}

	inline bool Compare(const WhileStatement& lhs, const WhileStatement& rhs, const ComparisonParams& params)
	{
		if (!Compare(lhs.unroll, rhs.unroll, params))
			return false;

		if (!Compare(lhs.condition, rhs.condition, params))
			return false;

		if (!Compare(lhs.body, rhs.body, params))
			return false;

		return true;
	}
}
