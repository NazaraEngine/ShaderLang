// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/IndexRemapperVisitor.hpp>

namespace nzsl::Ast
{
	StatementPtr RemapIndices(Statement& statement, const IndexRemapperVisitor::Options& options)
	{
		IndexRemapperVisitor visitor;
		return visitor.Clone(statement, options);
	}
}

