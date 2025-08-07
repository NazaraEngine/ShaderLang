// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline bool StructAssignmentTransformer::Transform(Module& module, TransformerContext& context, std::string* error)
	{
		return Transform(module, context, {}, error);
	}
}
