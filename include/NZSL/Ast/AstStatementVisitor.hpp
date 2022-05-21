// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ASTSTATEMENTVISITOR_HPP
#define NZSL_AST_ASTSTATEMENTVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/Nodes.hpp>

namespace nzsl::ShaderAst
{
	class NZSL_API AstStatementVisitor
	{
		public:
			AstStatementVisitor() = default;
			AstStatementVisitor(const AstStatementVisitor&) = delete;
			AstStatementVisitor(AstStatementVisitor&&) = delete;
			virtual ~AstStatementVisitor();

#define NZSL_SHADERAST_STATEMENT(NodeType) virtual void Visit(ShaderAst::NodeType& node) = 0;
#include <NZSL/Ast/AstNodeList.hpp>

			AstStatementVisitor& operator=(const AstStatementVisitor&) = delete;
			AstStatementVisitor& operator=(AstStatementVisitor&&) = delete;
	};
}

#endif // NZSL_AST_ASTSTATEMENTVISITOR_HPP
