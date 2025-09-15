// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_LOOPUNROLLTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_LOOPUNROLLTRANSFORMER_HPP

#include <NazaraUtils/FixedVector.hpp>
#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API LoopUnrollTransformer final : public Transformer
	{
		public:
			struct Options;

			inline LoopUnrollTransformer();

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool unrollForLoops = true;
				bool unrollForEachLoops = true;
			};

		private:
			using Transformer::Transform;

			ExpressionTransformation Transform(IdentifierValueExpression&& expression) override;

			StatementTransformation Transform(ForEachStatement&& statement) override;
			StatementTransformation Transform(ForStatement&& statement) override;

			struct VariableRemapping
			{
				IdentifierType targetIdentifierType;
				std::size_t sourceVariableIndex;
				std::size_t targetIdentifierIndex;
			};

			std::size_t m_currentModuleId;
			Nz::HybridVector<VariableRemapping, 4> m_variableMappings;
			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/LoopUnrollTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_LOOPUNROLLTRANSFORMER_HPP
