// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline Transformer::Transformer(bool visitExpressions) :
	m_visitExpressions(visitExpressions)
	{
	}

	inline void Transformer::HandleExpressionValue(ExpressionValue<ExpressionType>& expressionValue)
	{
		Transform(expressionValue);
	}

	template<typename T>
	void Transformer::HandleExpressionValue(ExpressionValue<T>& expressionValue)
	{
		if (expressionValue.IsExpression())
			HandleExpression(expressionValue.GetExpression());
	}

	template<bool Single, typename F>
	void Transformer::HandleStatementList(std::vector<StatementPtr>& statementList, F&& callback)
	{
		std::vector<StatementPtr>* previousStatementList = m_currentStatementList;
		std::size_t previousListIndex = m_currentStatementListIndex;

		m_currentStatementList = &statementList;
		m_currentStatementListIndex = 0;

		if constexpr (Single)
			callback();
		else
		{
			for (; m_currentStatementListIndex < statementList.size(); ++m_currentStatementListIndex)
				callback(statementList[m_currentStatementListIndex]);
		}

		m_currentStatementList = previousStatementList;
		m_currentStatementListIndex = previousListIndex;
	}
}
