// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl::Ast
{
	TransformerContext::TransformerContext() :
	aliases("alias"),
	constants("constant"),
	namedExternalBlocks("namedExternalBlock"),
	functions("function"),
	intrinsics("intrinsic"),
	modules("module"),
	structs("struct"),
	types("type"),
	variables("variable")
	{
	}

	void TransformerContext::Reset()
	{
		optionValues.clear();
		aliases.Clear();
		constants.Clear();
		namedExternalBlocks.Clear();
		functions.Clear();
		intrinsics.Clear();
		modules.Clear();
		structs.Clear();
		types.Clear();
		variables.Clear();
		allowUnknownIdentifiers = false;
		partialCompilation = false;
	}
}
