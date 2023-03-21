// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirV/SpirvExpressionStore.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/StackArray.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/SpirV/SpirvAstVisitor.hpp>
#include <NZSL/SpirV/SpirvBlock.hpp>
#include <cassert>
#include <numeric>

namespace nzsl
{
	void SpirvExpressionStore::Store(Ast::ExpressionPtr& node, std::uint32_t resultId)
	{
		node->Visit(*this);
		
		std::visit(Nz::Overloaded
		{
			[&](const Pointer& pointer)
			{
				m_block.Append(SpirvOp::OpStore, pointer.pointerId, resultId);
			},
			[&](const SwizzledPointer& swizzledPointer)
			{
				if (swizzledPointer.componentCount > 1)
				{
					std::size_t vectorSize = swizzledPointer.swizzledType.componentCount;

					std::uint32_t exprTypeId = m_writer.GetTypeId(swizzledPointer.swizzledType);

					// Load original value (which will then be shuffled with new value)
					std::uint32_t originalVecId = m_visitor.AllocateResultId();
					m_block.Append(SpirvOp::OpLoad, exprTypeId, originalVecId, swizzledPointer.pointerId);

					// Build a new composite type using OpVectorShuffle and store it
					Nz::StackArray<std::uint32_t> indices = NazaraStackArrayNoInit(std::uint32_t, vectorSize);
					std::iota(indices.begin(), indices.end(), std::uint32_t(0u)); //< init with regular swizzle (0,1,2,3)

					// override with swizzle components
					for (std::size_t i = 0; i < swizzledPointer.componentCount; ++i)
						indices[swizzledPointer.swizzleIndices[i]] = Nz::SafeCast<std::uint32_t>(vectorSize + i);

					std::uint32_t shuffleResultId = m_visitor.AllocateResultId();
					m_block.AppendVariadic(SpirvOp::OpVectorShuffle, [&](const auto& appender)
					{
						appender(exprTypeId);
						appender(shuffleResultId);

						appender(originalVecId);
						appender(resultId);

						for (std::uint32_t index : indices)
							appender(index);
					});

					// Store result
					m_block.Append(SpirvOp::OpStore, swizzledPointer.pointerId, shuffleResultId);
				}
				else
				{
					assert(swizzledPointer.componentCount == 1);

					const Ast::ExpressionType* exprType = GetExpressionType(*node);
					assert(exprType);

					std::uint32_t pointerType = m_writer.RegisterPointerType(*exprType, swizzledPointer.storage); //< FIXME

					// Access chain
					std::uint32_t indexId = m_writer.GetSingleConstantId(Nz::SafeCast<std::int32_t>(swizzledPointer.swizzleIndices[0]));

					std::uint32_t pointerId = m_visitor.AllocateResultId();
					m_block.Append(SpirvOp::OpAccessChain, pointerType, pointerId, swizzledPointer.pointerId, indexId);
					m_block.Append(SpirvOp::OpStore, pointerId, resultId);
				}
			},
			[](std::monostate)
			{
				throw std::runtime_error("an internal error occurred");
			}
		}, m_value);
	}

	void SpirvExpressionStore::Visit(Ast::AccessIndexExpression& node)
	{
		node.expr->Visit(*this);

		const Ast::ExpressionType* exprType = GetExpressionType(node);
		assert(exprType);

		std::visit(Nz::Overloaded
		{
			[&](const Pointer& pointer)
			{
				std::uint32_t resultId = m_visitor.AllocateResultId();
				std::uint32_t pointerType = m_writer.RegisterPointerType(*exprType, pointer.storage); //< FIXME

				assert(node.indices.size() == 1);
				std::uint32_t indexId = m_visitor.EvaluateExpression(*node.indices.front());

				m_block.Append(SpirvOp::OpAccessChain, pointerType, resultId, pointer.pointerId, indexId); 

				m_value = Pointer { pointer.storage, resultId };
			},
			[](std::monostate)
			{
				throw std::runtime_error("an internal error occurred");
			}
		}, m_value);
	}

	void SpirvExpressionStore::Visit(Ast::SwizzleExpression& node)
	{
		node.expression->Visit(*this);

		std::visit(Nz::Overloaded
		{
			[&](const Pointer& pointer)
			{
				const Ast::ExpressionType* expressionType = GetExpressionType(*node.expression);
				assert(expressionType);
				assert(IsVectorType(*expressionType));

				SwizzledPointer swizzledPointer;
				swizzledPointer.pointerId = pointer.pointerId;
				swizzledPointer.storage = pointer.storage;
				swizzledPointer.swizzledType = std::get<Ast::VectorType>(*expressionType);
				swizzledPointer.componentCount = node.componentCount;
				swizzledPointer.swizzleIndices = node.components;

				m_value = swizzledPointer;
			},
			[&](SwizzledPointer& swizzledPointer)
			{
				// Swizzle the swizzle, keep common components
				std::array<std::uint32_t, 4> newIndices;
				newIndices.fill(0); //< keep compiler happy

				for (std::size_t i = 0; i < node.componentCount; ++i)
				{
					assert(node.components[i] < swizzledPointer.componentCount);
					newIndices[i] = swizzledPointer.swizzleIndices[node.components[i]];
				}

				swizzledPointer.componentCount = node.componentCount;
				swizzledPointer.swizzleIndices = newIndices;
			},
			[](std::monostate)
			{
				throw std::runtime_error("an internal error occurred");
			}
		}, m_value);
	}

	void SpirvExpressionStore::Visit(Ast::VariableValueExpression& node)
	{
		const auto& var = m_visitor.GetVariable(node.variableId);
		m_value = Pointer{ var.storageClass, var.pointerId };
	}
}
