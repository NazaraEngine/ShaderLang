// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline IdentifierResolverTransformer::IdentifierResolverTransformer() :
	Transformer(true)
	{
	}

	inline bool IdentifierResolverTransformer::Transform(Module& module, Context& context, std::string* error)
	{
		return Transform(module, context, {}, error);
	}


	template<typename... Args>
	std::string IdentifierResolverTransformer::ToString(const std::variant<Args...>& value, const SourceLocation& sourceLocation) const
	{
		return std::visit([&](auto&& arg)
		{
			return ToString(arg, sourceLocation);
		}, value);
	}
}
