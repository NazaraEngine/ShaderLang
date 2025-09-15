// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirV/SpirvExpressionLoad.hpp>
#include <NazaraUtils/StackArray.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/SpirV/SpirvAstVisitor.hpp>
#include <NZSL/SpirV/SpirvBlock.hpp>

namespace nzsl
{
	std::uint32_t SpirvExpressionLoad::EvaluatePointer(Ast::Expression& node)
	{
		node.Visit(*this);

		return std::visit(Nz::Overloaded
		{
			[](const CompositeExtraction& /*extractedValue*/) -> std::uint32_t
			{
				throw std::runtime_error("expected pointer, got value");
			},
			[](const Pointer& pointer) -> std::uint32_t
			{
				return pointer.pointerId;
			},
			[this](const PointerChainAccess& pointerChainAccess) -> std::uint32_t
			{
				std::uint32_t pointerType = m_writer.RegisterPointerType(pointerChainAccess.pointedTypePtr, pointerChainAccess.storage); //< FIXME: We shouldn't register this so late

				std::uint32_t pointerId = m_visitor.AllocateResultId();

				m_block.AppendVariadic(SpirvOp::OpAccessChain, [&](const auto& appender)
				{
					appender(pointerType);
					appender(pointerId);
					appender(pointerChainAccess.pointerId);

					for (std::uint32_t id : pointerChainAccess.indicesId)
						appender(id);
				});

				return pointerId;
			},
			[](const Value& /*value*/) -> std::uint32_t
			{
				throw std::runtime_error("expected pointer, got value");
			},
			[](std::monostate) -> std::uint32_t
			{
				throw std::runtime_error("an internal error occurred");
			}
		}, m_value);
	}

	std::uint32_t SpirvExpressionLoad::EvaluateValue(Ast::Expression& node)
	{
		node.Visit(*this);

		return std::visit(Nz::Overloaded
		{
			[this](const CompositeExtraction& extractedValue) -> std::uint32_t
			{
				std::uint32_t resultId = m_visitor.AllocateResultId();

				m_block.AppendVariadic(SpirvOp::OpCompositeExtract, [&](const auto& appender)
				{
					appender(extractedValue.typeId);
					appender(resultId);
					appender(extractedValue.valueId);

					for (std::uint32_t id : extractedValue.indices)
						appender(id);
				});

				return resultId;
			},
			[this](const Pointer& pointer) -> std::uint32_t
			{
				std::uint32_t resultId = m_visitor.AllocateResultId();
				m_block.Append(SpirvOp::OpLoad, pointer.pointedTypeId, resultId, pointer.pointerId);

				return resultId;
			},
			[this](const PointerChainAccess& pointerChainAccess) -> std::uint32_t
			{
				std::uint32_t pointerType = m_writer.RegisterPointerType(pointerChainAccess.pointedTypePtr, pointerChainAccess.storage); //< FIXME: We shouldn't register this so late

				std::uint32_t pointerId = m_visitor.AllocateResultId();

				m_block.AppendVariadic(SpirvOp::OpAccessChain, [&](const auto& appender)
				{
					appender(pointerType);
					appender(pointerId);
					appender(pointerChainAccess.pointerId);

					for (std::uint32_t id : pointerChainAccess.indicesId)
						appender(id);
				});

				std::uint32_t resultId = m_visitor.AllocateResultId();
				m_block.Append(SpirvOp::OpLoad, m_writer.GetTypeId(*pointerChainAccess.pointedTypePtr), resultId, pointerId);

				return resultId;
			},
			[](const Value& value) -> std::uint32_t
			{
				return value.valueId;
			},
			[](std::monostate) -> std::uint32_t
			{
				throw std::runtime_error("an internal error occurred");
			}
		}, m_value);
	}

	void SpirvExpressionLoad::Visit(Ast::AccessFieldExpression& node)
	{
		node.expr->Visit(*this);

		const Ast::ExpressionType* exprType = GetExpressionType(node);
		assert(exprType);

		std::uint32_t typeId = m_writer.GetTypeId(*exprType);

		std::int32_t compositeIndex = static_cast<std::int32_t>(node.fieldIndex);

		std::visit(Nz::Overloaded
		{
			[&](CompositeExtraction& extractedValue)
			{
				extractedValue.indices.push_back(compositeIndex);
				extractedValue.typeId = typeId;
			},
			[&](const Pointer& pointer)
			{
				std::uint32_t constantId = m_writer.GetSingleConstantId(compositeIndex);

				PointerChainAccess pointerChainAccess;
				pointerChainAccess.exprType = exprType;
				pointerChainAccess.indicesId = { constantId };
				pointerChainAccess.pointedTypeId = pointer.pointedTypeId;
				pointerChainAccess.pointedTypePtr = SpirvConstantCache::GetIndexedType(*pointer.pointedTypePtr, compositeIndex);
				pointerChainAccess.pointerId = pointer.pointerId;
				pointerChainAccess.storage = pointer.storage;

				m_value = std::move(pointerChainAccess);
			},
			[&](PointerChainAccess& pointerChainAccess)
			{
				std::uint32_t constantId = m_writer.GetSingleConstantId(compositeIndex);

				pointerChainAccess.exprType = exprType;
				pointerChainAccess.pointedTypePtr = SpirvConstantCache::GetIndexedType(*pointerChainAccess.pointedTypePtr, compositeIndex);
				pointerChainAccess.indicesId.push_back(constantId);
			},
			[&](const Value& value)
			{
				CompositeExtraction extractedValue;
				extractedValue.indices = { compositeIndex };
				extractedValue.typeId = typeId;
				extractedValue.valueId = value.valueId;

				m_value = std::move(extractedValue);
			},
			[](std::monostate)
			{
				throw std::runtime_error("an internal error occurred");
			}
		}, m_value);
	}

	void SpirvExpressionLoad::Visit(Ast::AccessIndexExpression& node)
	{
		node.expr->Visit(*this);

		const Ast::ExpressionType* exprType = GetExpressionType(node);
		assert(exprType);

		assert(node.indices.size() == 1);
		auto& indexExpr = node.indices.front();

		std::uint32_t indexId = m_visitor.EvaluateExpression(*indexExpr);

		std::visit(Nz::Overloaded
		{
			[&](CompositeExtraction& /*extractedValue*/)
			{
				throw std::runtime_error("unexpected unknown index of value");
			},
			[&](const Pointer& pointer)
			{
				PointerChainAccess pointerChainAccess;
				pointerChainAccess.exprType = exprType;
				pointerChainAccess.indicesId = { indexId };
				pointerChainAccess.pointedTypeId = pointer.pointedTypeId;
				pointerChainAccess.pointedTypePtr = SpirvConstantCache::GetIndexedType(*pointer.pointedTypePtr, -1);
				pointerChainAccess.pointerId = pointer.pointerId;
				pointerChainAccess.storage = pointer.storage;

				m_value = std::move(pointerChainAccess);
			},
			[&](PointerChainAccess& pointerChainAccess)
			{
				pointerChainAccess.exprType = exprType;
				pointerChainAccess.indicesId.push_back(indexId);
				pointerChainAccess.pointedTypePtr = SpirvConstantCache::GetIndexedType(*pointerChainAccess.pointedTypePtr, -1);
			},
			[&](const Value& /*value*/)
			{
				throw std::runtime_error("unexpected unknown index of value");
			},
			[](std::monostate)
			{
				throw std::runtime_error("an internal error occurred");
			}
		}, m_value);
	}

	void SpirvExpressionLoad::Visit(Ast::IdentifierValueExpression& node)
	{
		switch (node.identifierType)
		{
			case Ast::IdentifierType::Constant:
			{
				const auto& var = m_writer.GetConstantVariable(node.identifierIndex);
				m_value = Pointer{ var.typePtr, var.storageClass, var.pointerId, var.typeId };
				break;
			}

			case Ast::IdentifierType::Variable:
			{
				const auto& var = m_visitor.GetVariable(node.identifierIndex);
				m_value = Pointer{ var.typePtr, var.storageClass, var.pointerId, var.typeId };
				break;
			}

			default:
				throw std::runtime_error("unexpected identifier");
		}
	}
}
