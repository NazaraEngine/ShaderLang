// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVEXPRESSIONLOAD_HPP
#define NZSL_SPIRVEXPRESSIONLOAD_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <NZSL/Ast/ExpressionVisitorExcept.hpp>
#include <vector>

namespace nzsl
{
	class SpirvAstVisitor;
	class SpirvBlock;
	class SpirvWriter;

	class NZSL_API SpirvExpressionLoad : public Ast::ExpressionVisitorExcept
	{
		public:
			inline SpirvExpressionLoad(SpirvWriter& writer, SpirvAstVisitor& visitor, SpirvBlock& block);
			SpirvExpressionLoad(const SpirvExpressionLoad&) = delete;
			SpirvExpressionLoad(SpirvExpressionLoad&&) = delete;
			~SpirvExpressionLoad() = default;

			std::uint32_t Evaluate(Ast::Expression& node);

			using ExpressionVisitorExcept::Visit;
			void Visit(Ast::AccessIndexExpression& node) override;
			void Visit(Ast::ConstantExpression& node) override;
			void Visit(Ast::VariableValueExpression& node) override;

			SpirvExpressionLoad& operator=(const SpirvExpressionLoad&) = delete;
			SpirvExpressionLoad& operator=(SpirvExpressionLoad&&) = delete;

		private:
			struct CompositeExtraction
			{
				std::vector<std::int32_t> indices;
				std::uint32_t typeId;
				std::uint32_t valueId;
			};

			struct PointerChainAccess
			{
				std::vector<std::uint32_t> indicesId;
				const Ast::ExpressionType* exprType;
				SpirvStorageClass storage;
				std::uint32_t pointerId;
				std::uint32_t pointedTypeId;
			};

			struct Pointer
			{
				SpirvStorageClass storage;
				std::uint32_t pointerId;
				std::uint32_t pointedTypeId;
			};

			struct Value
			{
				std::uint32_t valueId;
			};

			SpirvAstVisitor& m_visitor;
			SpirvBlock& m_block;
			SpirvWriter& m_writer;
			std::variant<std::monostate, CompositeExtraction, Pointer, PointerChainAccess, Value> m_value;
	};
}

#include <NZSL/SpirV/SpirvExpressionLoad.inl>

#endif // NZSL_SPIRVEXPRESSIONLOAD_HPP
