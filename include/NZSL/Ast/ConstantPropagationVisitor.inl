// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl::Ast
{
	inline ExpressionPtr ConstantPropagationVisitor::Process(Expression& expression)
	{
		m_options = {};
		return CloneExpression(expression);
	}

	inline ExpressionPtr ConstantPropagationVisitor::Process(Expression& expression, const Options& options)
	{
		m_options = options;
		return CloneExpression(expression);
	}

	inline StatementPtr ConstantPropagationVisitor::Process(Statement& statement)
	{
		m_options = {};
		return CloneStatement(statement);
	}

	inline StatementPtr ConstantPropagationVisitor::Process(Statement& statement, const Options& options)
	{
		m_options = options;
		return CloneStatement(statement);
	}

	inline ExpressionPtr PropagateConstants(Expression& ast)
	{
		ConstantPropagationVisitor optimize;
		return optimize.Process(ast);
	}

	inline ExpressionPtr PropagateConstants(Expression& ast, const ConstantPropagationVisitor::Options& options)
	{
		ConstantPropagationVisitor optimize;
		return optimize.Process(ast, options);
	}

	inline ModulePtr PropagateConstants(const Module& shaderModule)
	{
		ConstantPropagationVisitor optimize;
		return optimize.Process(shaderModule);
	}

	inline ModulePtr PropagateConstants(const Module& shaderModule, const ConstantPropagationVisitor::Options& options)
	{
		ConstantPropagationVisitor optimize;
		return optimize.Process(shaderModule, options);
	}

	inline StatementPtr PropagateConstants(Statement& ast)
	{
		ConstantPropagationVisitor optimize;
		return optimize.Process(ast);
	}

	inline StatementPtr PropagateConstants(Statement& ast, const ConstantPropagationVisitor::Options& options)
	{
		ConstantPropagationVisitor optimize;
		return optimize.Process(ast, options);
	}
}
