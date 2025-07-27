// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline bool ConstantPropagationTransformer::Transform(ExpressionPtr& expression, TransformerContext& context, std::string* error)
	{
		return Transform(expression, context, {}, error);
	}

	inline bool ConstantPropagationTransformer::Transform(ExpressionPtr& expression, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		return TransformExpression(expression, context, error);
	}

	inline bool ConstantPropagationTransformer::Transform(Module& shaderModule, TransformerContext& context, std::string* error)
	{
		return Transform(shaderModule, context, {}, error);
	}

	inline bool ConstantPropagationTransformer::Transform(Module& shaderModule, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(shaderModule, context, error))
			return false;

		return TransformModule(shaderModule, context, error);
	}

	inline bool ConstantPropagationTransformer::Transform(StatementPtr& statement, TransformerContext& context, std::string* error)
	{
		return Transform(statement, context, {}, error);
	}

	inline bool ConstantPropagationTransformer::Transform(StatementPtr& statement, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		return TransformStatement(statement, context, error);
	}

	template<typename T, typename Other>
	auto ConstantPropagationTransformer::ResolveUntypedIfNecessary(T value)
	{
		if constexpr (IsUntyped_v<T> && IsUntyped_v<Other>)
			return value; // Untyped + Untyped = Untyped
		else if constexpr (IsUntyped_v<T> && std::is_convertible_v<T, Other>)
		{
			if constexpr (std::is_floating_point_v<typename T::Inner> == std::is_floating_point_v<Other>)
				return static_cast<Other>(value); // Take other operand type
			else
				return value; // float + UntypedInteger is not valid
		}
		else
			return value; // Other is Untyped but not us, keep our type
	}
}
