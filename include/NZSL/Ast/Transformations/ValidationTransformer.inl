// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline ValidationTransformer::ValidationTransformer() :
	Transformer(TransformerFlag::IgnoreFunctionContent | TransformerFlag::IgnoreLoopContent) //< function and loop content is handled in a second pass
	{
	}

	inline bool ValidationTransformer::Transform(Module& module, Context& context, std::string* error)
	{
		return Transform(module, context, {}, error);
	}
}
