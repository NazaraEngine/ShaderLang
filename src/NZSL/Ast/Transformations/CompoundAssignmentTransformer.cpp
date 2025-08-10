// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/CompoundAssignmentTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool CompoundAssignmentTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto CompoundAssignmentTransformer::Transform(AssignExpression&& assign) -> ExpressionTransformation
	{
		if (assign.op == AssignType::Simple || !m_options->removeCompoundAssignment)
			return VisitChildren{};

		HandleChildren(assign);

		BinaryType binaryType;
		switch (assign.op)
		{
			case AssignType::Simple:             NAZARA_UNREACHABLE(); break;
			case AssignType::CompoundAdd:        binaryType = BinaryType::Add; break;
			case AssignType::CompoundDivide:     binaryType = BinaryType::Divide; break;
			case AssignType::CompoundModulo:     binaryType = BinaryType::Modulo; break;
			case AssignType::CompoundMultiply:   binaryType = BinaryType::Multiply; break;
			case AssignType::CompoundLogicalAnd: binaryType = BinaryType::LogicalAnd; break;
			case AssignType::CompoundLogicalOr:  binaryType = BinaryType::LogicalOr; break;
			case AssignType::CompoundSubtract:   binaryType = BinaryType::Subtract; break;
		}

		assign.op = AssignType::Simple;
		assign.right = ShaderBuilder::Binary(binaryType, Clone(*assign.left), std::move(assign.right));
		assign.right->cachedExpressionType = assign.left->cachedExpressionType;

		return DontVisitChildren{};
	}
}
