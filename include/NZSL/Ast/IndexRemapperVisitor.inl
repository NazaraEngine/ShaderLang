// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline void RemapIndices(Statement& statement, const IndexRemapperVisitor::Options& options)
	{
		IndexRemapperVisitor visitor;
		return visitor.Remap(statement, options);
	}
}
