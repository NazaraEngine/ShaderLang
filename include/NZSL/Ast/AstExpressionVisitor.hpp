// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ASTEXPRESSIONVISITOR_HPP
#define NZSL_AST_ASTEXPRESSIONVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/Nodes.hpp>

namespace nzsl::ShaderAst
{
	class NZSL_API AstExpressionVisitor
	{
		public:
			AstExpressionVisitor() = default;
			AstExpressionVisitor(const AstExpressionVisitor&) = delete;
			AstExpressionVisitor(AstExpressionVisitor&&) = delete;
			virtual ~AstExpressionVisitor();

#define NZSL_SHADERAST_EXPRESSION(Node) virtual void Visit(Node& node) = 0;
#include <NZSL/Ast/AstNodeList.hpp>

			AstExpressionVisitor& operator=(const AstExpressionVisitor&) = delete;
			AstExpressionVisitor& operator=(AstExpressionVisitor&&) = delete;
	};
}

#endif // NZSL_AST_ASTEXPRESSIONVISITOR_HPP
