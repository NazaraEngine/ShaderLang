// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline bool UniformStructToStd140Transformer::Transform(Module& module, TransformerContext& context, std::string* error)
	{
		return Transform(module, context, {}, error);
	}
}
