// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVEXPRESSIONSTORE_HPP
#define NZSL_SPIRVEXPRESSIONSTORE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/SpirvData.hpp>
#include <NZSL/Ast/AstExpressionVisitorExcept.hpp>
#include <NZSL/Ast/Enums.hpp>

namespace nzsl
{
	class SpirvAstVisitor;
	class SpirvBlock;
	class SpirvWriter;

	class NZSL_API SpirvExpressionStore : public ShaderAst::AstExpressionVisitorExcept
	{
		public:
			inline SpirvExpressionStore(SpirvWriter& writer, SpirvAstVisitor& visitor, SpirvBlock& block);
			SpirvExpressionStore(const SpirvExpressionStore&) = delete;
			SpirvExpressionStore(SpirvExpressionStore&&) = delete;
			~SpirvExpressionStore() = default;

			void Store(ShaderAst::ExpressionPtr& node, std::uint32_t resultId);

			using AstExpressionVisitorExcept::Visit;
			void Visit(ShaderAst::AccessIndexExpression& node) override;
			void Visit(ShaderAst::SwizzleExpression& node) override;
			void Visit(ShaderAst::VariableValueExpression& node) override;

			SpirvExpressionStore& operator=(const SpirvExpressionStore&) = delete;
			SpirvExpressionStore& operator=(SpirvExpressionStore&&) = delete;

		private:
			struct Pointer
			{
				SpirvStorageClass storage;
				std::uint32_t pointerId;
			};

			struct SwizzledPointer : Pointer
			{
				ShaderAst::VectorType swizzledType;
				std::array<std::uint32_t, 4> swizzleIndices;
				std::size_t componentCount;
			};

			SpirvAstVisitor& m_visitor;
			SpirvBlock& m_block;
			SpirvWriter& m_writer;
			std::variant<std::monostate, Pointer, SwizzledPointer> m_value;
	};
}

#include <NZSL/SpirvExpressionStore.inl>

#endif // NZSL_SPIRVEXPRESSIONSTORE_HPP
