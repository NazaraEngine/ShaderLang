// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_STRUCTASSIGNMENTTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_STRUCTASSIGNMENTTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API StructAssignmentTransformer final : public Transformer
	{
		public:
			struct Options;

			StructAssignmentTransformer() = default;

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool splitWrappedArrayAssignation = false;
				bool splitWrappedStructAssignation = false;
			};

		private:
			using Transformer::Transform;

			ExpressionTransformation Transform(AssignExpression&& assign) override;
			StatementTransformation Transform(DeclareStructStatement&& declStruct) override;
			StatementTransformation Transform(DeclareVariableStatement&& declVariable) override;

			const Options* m_options;
			std::unordered_map<std::size_t /*structIndex*/, const StructDescription*> m_structDescs;
	};
}

#include <NZSL/Ast/Transformations/StructAssignmentTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_STRUCTASSIGNMENTTRANSFORMER_HPP
