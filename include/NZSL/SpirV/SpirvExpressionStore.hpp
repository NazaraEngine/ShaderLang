// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRV_SPIRVEXPRESSIONSTORE_HPP
#define NZSL_SPIRV_SPIRVEXPRESSIONSTORE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/ExpressionVisitorExcept.hpp>
#include <NZSL/SpirV/SpirvConstantCache.hpp>
#include <NZSL/SpirV/SpirvData.hpp>

namespace nzsl
{
	class SpirvAstVisitor;
	class SpirvBlock;
	class SpirvWriter;

	class NZSL_API SpirvExpressionStore : public Ast::ExpressionVisitorExcept
	{
		public:
			inline SpirvExpressionStore(SpirvWriter& writer, SpirvAstVisitor& visitor, SpirvBlock& block);
			SpirvExpressionStore(const SpirvExpressionStore&) = delete;
			SpirvExpressionStore(SpirvExpressionStore&&) = delete;
			~SpirvExpressionStore() = default;

			void Store(Ast::ExpressionPtr& node, std::uint32_t resultId);

			using ExpressionVisitorExcept::Visit;
			void Visit(Ast::AccessIndexExpression& node) override;
			void Visit(Ast::SwizzleExpression& node) override;
			void Visit(Ast::VariableValueExpression& node) override;

			SpirvExpressionStore& operator=(const SpirvExpressionStore&) = delete;
			SpirvExpressionStore& operator=(SpirvExpressionStore&&) = delete;

		private:
			struct Pointer
			{
				SpirvConstantCache::TypePtr pointedTypePtr;
				SpirvStorageClass storage;
				std::uint32_t pointerId;
			};

			struct SwizzledPointer : Pointer
			{
				Ast::VectorType swizzledType;
				std::array<std::uint32_t, 4> swizzleIndices;
				std::size_t componentCount;
			};

			SpirvAstVisitor& m_visitor;
			SpirvBlock& m_block;
			SpirvWriter& m_writer;
			std::variant<std::monostate, Pointer, SwizzledPointer> m_value;
	};
}

#include <NZSL/SpirV/SpirvExpressionStore.inl>

#endif // NZSL_SPIRV_SPIRVEXPRESSIONSTORE_HPP
