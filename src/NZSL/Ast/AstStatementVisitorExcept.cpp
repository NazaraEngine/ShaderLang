// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/AstStatementVisitorExcept.hpp>

namespace nzsl::Ast
{
#define NZSL_SHADERAST_STATEMENT(Node) void AstStatementVisitorExcept::Visit(Ast::Node& /*node*/) \
	{ \
		throw std::runtime_error("unexpected " #Node " node"); \
	}
#include <NZSL/Ast/AstNodeList.hpp>
}
