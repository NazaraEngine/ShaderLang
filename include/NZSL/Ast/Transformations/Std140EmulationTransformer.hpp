// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_STD140EMULATION_HPP
#define NZSL_AST_TRANSFORMATIONS_STD140EMULATION_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>
#include <unordered_map>

namespace nzsl::Ast
{
	class NZSL_API Std140EmulationTransformer final : public Transformer
	{
		public:
			struct Options;

			Std140EmulationTransformer() = default;

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
			};

		private:
			using Transformer::Transform;

			ExpressionTransformation Transform(AccessFieldExpression&& accessFieldExpr) override;
			ExpressionTransformation Transform(AccessIndexExpression&& accessIndexExpr) override;
			StatementTransformation Transform(DeclareStructStatement&& declStruct) override;

			DeclareStructStatementPtr DeclareStride16PrimitiveHelper(PrimitiveType type, std::size_t moduleIndex, SourceLocation sourceLocation);
			bool ComputeStructDeclarationPadding(StructDescription& desc, const SourceLocation& sourceLocation) const;
			FieldOffsets ComputeStructFieldOffsets(const StructDescription& desc, const SourceLocation& sourceLocation) const;
			bool HandleStd140Propagation(MultiStatementPtr& multiStatement, std::size_t structIndex, SourceLocation sourceLocation, bool shouldExport);

			std::unordered_map<std::size_t /*structIndex*/, std::size_t /*newStructIndex*/> m_structStd140Map;
			std::unordered_map<PrimitiveType, std::size_t /* structIndex */> m_stride16Structs;
			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/Std140EmulationTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_STD140EMULATION_HPP

