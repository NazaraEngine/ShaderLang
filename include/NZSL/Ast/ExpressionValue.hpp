// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_EXPRESSIONVALUE_HPP
#define NZSL_AST_EXPRESSIONVALUE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <memory>
#include <variant>

namespace nzsl::Ast
{
	struct Expression;

	using ExpressionPtr = std::unique_ptr<Expression>;

	template<typename T>
	class ExpressionValue
	{
		public:
			ExpressionValue() = default;
			ExpressionValue(T value);
			ExpressionValue(ExpressionPtr expr);
			ExpressionValue(const ExpressionValue&) = default;
			ExpressionValue(ExpressionValue&&) noexcept = default;
			~ExpressionValue() = default;

			ExpressionPtr&& GetExpression() &&;
			const ExpressionPtr& GetExpression() const &;
			const T& GetResultingValue() const;

			bool IsExpression() const;
			bool IsResultingValue() const;

			bool HasValue() const;

			void Reset();

			ExpressionValue& operator=(const ExpressionValue&) = default;
			ExpressionValue& operator=(ExpressionValue&&) noexcept = default;

		private:
			std::variant<std::monostate, T, ExpressionPtr> m_value;
	};
}

#include <NZSL/Ast/ExpressionValue.inl>

#endif // NZSL_AST_EXPRESSIONVALUE_HPP
