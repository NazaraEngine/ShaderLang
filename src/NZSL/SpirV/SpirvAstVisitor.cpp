// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirV/SpirvAstVisitor.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackArray.hpp>
#include <NazaraUtils/StackVector.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/SpirV/SpirvExpressionLoad.hpp>
#include <NZSL/SpirV/SpirvExpressionStore.hpp>
#include <NZSL/SpirV/SpirvGenData.hpp>
#include <NZSL/SpirV/SpirvSection.hpp>

namespace nzsl
{
	std::uint32_t SpirvAstVisitor::AllocateResultId()
	{
		return m_writer.AllocateResultId();
	}

	std::uint32_t SpirvAstVisitor::EvaluateExpression(Ast::Expression& expr)
	{
		expr.Visit(*this);

		assert(m_resultIds.size() == 1);
		return PopResultId();
	}

	auto SpirvAstVisitor::GetVariable(std::size_t varIndex) const -> const SpirvVariable&
	{
		if (auto it = m_variables.find(varIndex); it != m_variables.end())
			return it->second;
		else
			return m_writer.GetExtVar(varIndex);
	}

	void SpirvAstVisitor::Visit(Ast::AccessIndexExpression& node)
	{
		HandleSourceLocation(node.sourceLocation);

		SpirvExpressionLoad accessMemberVisitor(m_writer, *this, *m_currentBlock);
		PushResultId(accessMemberVisitor.Evaluate(node));
	}

	void SpirvAstVisitor::Visit(Ast::AssignExpression& node)
	{
		if (node.op != Ast::AssignType::Simple)
			throw std::runtime_error("unexpected assign expression (should have been removed by sanitization)");

		std::uint32_t resultId = EvaluateExpression(*node.right);

		HandleSourceLocation(node.sourceLocation);

		SpirvExpressionStore storeVisitor(m_writer, *this, *m_currentBlock);
		storeVisitor.Store(node.left, resultId);

		PushResultId(resultId);
	}

	void SpirvAstVisitor::Visit(Ast::BinaryExpression& node)
	{
		auto RetrieveBaseType = [](const Ast::ExpressionType& exprType)
		{
			if (IsPrimitiveType(exprType))
				return std::get<Ast::PrimitiveType>(exprType);
			else if (IsVectorType(exprType))
				return std::get<Ast::VectorType>(exprType).type;
			else if (IsMatrixType(exprType))
				return std::get<Ast::MatrixType>(exprType).type;
			else
				throw std::runtime_error("unexpected type");
		};

		const Ast::ExpressionType& resultType = *GetExpressionType(node);
		const Ast::ExpressionType& leftType = *GetExpressionType(*node.left);
		const Ast::ExpressionType& rightType = *GetExpressionType(*node.right);

		Ast::PrimitiveType leftTypeBase = RetrieveBaseType(leftType);
		//Ast::PrimitiveType rightTypeBase = RetrieveBaseType(rightType);


		std::uint32_t leftOperand = EvaluateExpression(*node.left);
		std::uint32_t rightOperand = EvaluateExpression(*node.right);
		std::uint32_t resultId = m_writer.AllocateResultId();

		HandleSourceLocation(node.sourceLocation);

		bool compositeVecLeft = false;
		bool compositeVecRight = false;
		bool swapOperands = false;
		std::uint32_t resultTypeId = m_writer.GetTypeId(resultType);

		SpirvOp op = [&]
		{
			switch (node.op)
			{
				case Ast::BinaryType::Add:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFAdd;

						case Ast::PrimitiveType::Int32:
						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpIAdd;

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}

				case Ast::BinaryType::Subtract:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFSub;

						case Ast::PrimitiveType::Int32:
						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpISub;

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}

				case Ast::BinaryType::Divide:
				case Ast::BinaryType::Modulo:
				{
					if (IsPrimitiveType(leftType) && IsVectorType(rightType))
						compositeVecLeft = true; // primitive / vec = vec
					else if (IsVectorType(leftType) && IsPrimitiveType(rightType))
						compositeVecRight = true; // vec / primitive = vec

					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return (node.op == Ast::BinaryType::Divide) ? SpirvOp::OpFDiv : SpirvOp::OpFMod;

						case Ast::PrimitiveType::Int32:
							return (node.op == Ast::BinaryType::Divide) ? SpirvOp::OpSDiv : SpirvOp::OpSMod;

						case Ast::PrimitiveType::UInt32:
							return (node.op == Ast::BinaryType::Divide) ? SpirvOp::OpUDiv : SpirvOp::OpUMod;

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}

				case Ast::BinaryType::Multiply:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
						{
							if (IsPrimitiveType(leftType))
							{
								// Handle float * matrix|vector as matrix|vector * float
								if (IsMatrixType(rightType))
								{
									swapOperands = true;
									return SpirvOp::OpMatrixTimesScalar;
								}
								else if (IsVectorType(rightType))
								{
									swapOperands = true;
									return SpirvOp::OpVectorTimesScalar;
								}
							}
							else if (IsPrimitiveType(rightType))
							{
								if (IsMatrixType(leftType))
									return SpirvOp::OpMatrixTimesScalar;
								else if (IsVectorType(leftType))
									return SpirvOp::OpVectorTimesScalar;
							}
							else if (IsMatrixType(leftType))
							{
								if (IsMatrixType(rightType))
									return SpirvOp::OpMatrixTimesMatrix;
								else if (IsVectorType(rightType))
									return SpirvOp::OpMatrixTimesVector;
							}
							else if (IsMatrixType(rightType))
							{
								assert(IsVectorType(leftType));
								return SpirvOp::OpVectorTimesMatrix;
							}

							return SpirvOp::OpFMul;
						}

						case Ast::PrimitiveType::Int32:
						case Ast::PrimitiveType::UInt32:
						{
							if (IsVectorType(resultType))
							{
								if (IsPrimitiveType(leftType))
									compositeVecLeft = true;
								else if (IsPrimitiveType(rightType))
									compositeVecRight = true;
							}

							return SpirvOp::OpIMul;
						}

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}

				case Ast::BinaryType::CompEq:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Boolean:
							return SpirvOp::OpLogicalEqual;

						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFOrdEqual;

						case Ast::PrimitiveType::Int32:
						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpIEqual;

						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}
				
				case Ast::BinaryType::CompGe:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFOrdGreaterThanEqual;

						case Ast::PrimitiveType::Int32:
							return SpirvOp::OpSGreaterThanEqual;

						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpUGreaterThanEqual;

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}
				
				case Ast::BinaryType::CompGt:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFOrdGreaterThan;

						case Ast::PrimitiveType::Int32:
							return SpirvOp::OpSGreaterThan;

						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpUGreaterThan;

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}
				
				case Ast::BinaryType::CompLe:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFOrdLessThanEqual;

						case Ast::PrimitiveType::Int32:
							return SpirvOp::OpSLessThanEqual;

						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpULessThanEqual;

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}
				
				case Ast::BinaryType::CompLt:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFOrdLessThan;

						case Ast::PrimitiveType::Int32:
							return SpirvOp::OpSLessThan;

						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpULessThan;

						case Ast::PrimitiveType::Boolean:
						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}
				
				case Ast::BinaryType::CompNe:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Boolean:
							return SpirvOp::OpLogicalNotEqual;

						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							return SpirvOp::OpFOrdNotEqual;

						case Ast::PrimitiveType::Int32:
						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpINotEqual;

						case Ast::PrimitiveType::String:
							break;
					}

					break;
				}

				case Ast::BinaryType::LogicalAnd:
					return SpirvOp::OpLogicalAnd;

				case Ast::BinaryType::LogicalOr:
					return SpirvOp::OpLogicalOr;

				case Ast::BinaryType::BitwiseAnd:
					return SpirvOp::OpBitwiseAnd;

				case Ast::BinaryType::BitwiseOr:
					return SpirvOp::OpBitwiseOr;

				case Ast::BinaryType::BitwiseXor:
					return SpirvOp::OpBitwiseXor;

				case Ast::BinaryType::ShiftLeft:
					return SpirvOp::OpShiftLeftLogical;

				case Ast::BinaryType::ShiftRight:
					return (leftTypeBase == Ast::PrimitiveType::Int32) ? SpirvOp::OpShiftRightArithmetic : SpirvOp::OpShiftRightLogical;
			}

			assert(false);
			throw std::runtime_error("unexpected binary operation");
		}();

		if (compositeVecLeft || compositeVecRight)
		{
			const Ast::VectorType& vecType = std::get<Ast::VectorType>(resultType);

			std::uint32_t vecTypeId = m_writer.GetTypeId(vecType);

			if (compositeVecLeft)
			{
				std::uint32_t leftAsVec = m_writer.AllocateResultId();
				m_currentBlock->AppendVariadic(SpirvOp::OpCompositeConstruct, [&](auto&& append)
				{
					append(vecTypeId);
					append(leftAsVec);

					for (std::size_t i = 0; i < vecType.componentCount; ++i)
						append(leftOperand);
				});

				leftOperand = leftAsVec;
			}
			
			if (compositeVecRight)
			{
				std::uint32_t rightAsVec = m_writer.AllocateResultId();
				m_currentBlock->AppendVariadic(SpirvOp::OpCompositeConstruct, [&](auto&& append)
				{
					append(vecTypeId);
					append(rightAsVec);

					for (std::size_t i = 0; i < vecType.componentCount; ++i)
						append(rightOperand);
				});

				rightOperand = rightAsVec;
			}
		}

		if (swapOperands)
			std::swap(leftOperand, rightOperand);

		m_currentBlock->Append(op, resultTypeId, resultId, leftOperand, rightOperand);

		PushResultId(resultId);
	}

	void SpirvAstVisitor::Visit(Ast::BranchStatement& node)
	{
		assert(node.condStatements.size() == 1); //< sanitization splits multiple branches
		auto& condStatement = node.condStatements.front();

		auto mergeBlock = std::make_unique<SpirvBlock>(m_writer);
		auto contentBlock = std::make_unique<SpirvBlock>(m_writer);
		auto elseBlock = std::make_unique<SpirvBlock>(m_writer);

		std::uint32_t conditionId = EvaluateExpression(*condStatement.condition);

		HandleSourceLocation(node.sourceLocation);

		m_currentBlock->Append(SpirvOp::OpSelectionMerge, mergeBlock->GetLabelId(), SpirvSelectionControl::None);
		// FIXME: Can we use merge block directly in OpBranchConditional if no else statement?
		m_currentBlock->Append(SpirvOp::OpBranchConditional, conditionId, contentBlock->GetLabelId(), elseBlock->GetLabelId());

		m_functionBlocks.emplace_back(std::move(contentBlock));
		m_currentBlock = m_functionBlocks.back().get();

		condStatement.statement->Visit(*this);

		if (!m_currentBlock->IsTerminated())
			m_currentBlock->Append(SpirvOp::OpBranch, mergeBlock->GetLabelId());

		m_functionBlocks.emplace_back(std::move(elseBlock));
		m_currentBlock = m_functionBlocks.back().get();

		if (node.elseStatement)
			node.elseStatement->Visit(*this);

		if (!m_currentBlock->IsTerminated())
			m_currentBlock->Append(SpirvOp::OpBranch, mergeBlock->GetLabelId());

		m_functionBlocks.emplace_back(std::move(mergeBlock));
		m_currentBlock = m_functionBlocks.back().get();
	}

	void SpirvAstVisitor::Visit(Ast::BreakStatement& node)
	{
		if (!m_breakTarget)
			throw std::runtime_error("break statement outside of while");

		HandleSourceLocation(node.sourceLocation);

		m_currentBlock->Append(SpirvOp::OpBranch, *m_breakTarget);
	}

	void SpirvAstVisitor::Visit(Ast::CallFunctionExpression& node)
	{
		std::size_t functionIndex = std::get<Ast::FunctionType>(*GetExpressionType(*node.targetFunction)).funcIndex;

		const auto& targetFunc = m_functionRetriever(functionIndex);

		const auto& funcCall = m_currentFunc->funcCalls[m_funcCallIndex++];

		Nz::StackArray<std::uint32_t> parameterIds = NazaraStackArrayNoInit(std::uint32_t, node.parameters.size());
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			std::uint32_t resultId = EvaluateExpression(*node.parameters[i]);
			std::uint32_t varId = m_currentFunc->variables[funcCall.firstVarIndex + i].varId;
			m_currentBlock->Append(SpirvOp::OpStore, varId, resultId);

			parameterIds[i] = varId;
		}

		HandleSourceLocation(node.sourceLocation);

		std::uint32_t resultId = AllocateResultId();
		m_currentBlock->AppendVariadic(SpirvOp::OpFunctionCall, [&](auto&& appender)
		{
			appender(m_writer.GetTypeId(*Ast::GetExpressionType(node)));
			appender(resultId);
			appender(targetFunc.funcId);

			for (std::size_t i = 0; i < node.parameters.size(); ++i)
				appender(parameterIds[i]);
		});

		PushResultId(resultId);
	}

	void SpirvAstVisitor::Visit(Ast::CastExpression& node)
	{
		const Ast::ExpressionType& targetExprType = node.targetType.GetResultingValue();
		if (IsPrimitiveType(targetExprType) || (IsVectorType(targetExprType) && node.expressions.size() == 1))
		{
			Ast::PrimitiveType targetBaseType;
			if (IsPrimitiveType(targetExprType))
				targetBaseType = std::get<Ast::PrimitiveType>(targetExprType);
			else
			{
				assert(IsVectorType(targetExprType));
				targetBaseType = std::get<Ast::VectorType>(targetExprType).type;
			}

			assert(node.expressions.size() == 1);
			Ast::ExpressionPtr& expression = node.expressions[0];

			assert(expression->cachedExpressionType.has_value());
			const Ast::ExpressionType& fromExprType = expression->cachedExpressionType.value();

			Ast::PrimitiveType fromBaseType;
			if (IsPrimitiveType(fromExprType))
				fromBaseType = std::get<Ast::PrimitiveType>(fromExprType);
			else
			{
				assert(IsVectorType(fromExprType));
				fromBaseType = std::get<Ast::VectorType>(fromExprType).type;
			}

			std::uint32_t fromId = EvaluateExpression(*expression);
			if (targetBaseType == fromBaseType)
				return PushResultId(fromId);

			HandleSourceLocation(node.sourceLocation);

			std::optional<SpirvOp> castOp;
			switch (targetBaseType)
			{
				case Ast::PrimitiveType::Boolean:
					throw std::runtime_error("unsupported cast to boolean");

				case Ast::PrimitiveType::Float32:
				{
					switch (fromBaseType)
					{
						case Ast::PrimitiveType::Boolean:
							throw std::runtime_error("unsupported cast from boolean");

						case Ast::PrimitiveType::Float32:
							break; //< Already handled

						case Ast::PrimitiveType::Float64:
							castOp = SpirvOp::OpFConvert;
							break;

						case Ast::PrimitiveType::Int32:
							castOp = SpirvOp::OpConvertSToF;
							break;

						case Ast::PrimitiveType::UInt32:
							castOp = SpirvOp::OpConvertUToF;
							break;

						case Ast::PrimitiveType::String:
							throw std::runtime_error("unexpected string type");
					}
					break;
				}
				
				case Ast::PrimitiveType::Float64:
				{
					switch (fromBaseType)
					{
						case Ast::PrimitiveType::Boolean:
							throw std::runtime_error("unsupported cast from boolean");

						case Ast::PrimitiveType::Float32:
							castOp = SpirvOp::OpFConvert;
							break;

						case Ast::PrimitiveType::Float64:
							break; //< Already handled

						case Ast::PrimitiveType::Int32:
							castOp = SpirvOp::OpConvertSToF;
							break;

						case Ast::PrimitiveType::UInt32:
							castOp = SpirvOp::OpConvertUToF;
							break;

						case Ast::PrimitiveType::String:
							throw std::runtime_error("unexpected string type");
					}
					break;
				}

				case Ast::PrimitiveType::Int32:
				{
					switch (fromBaseType)
					{
						case Ast::PrimitiveType::Boolean:
							throw std::runtime_error("unsupported cast from boolean");

						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							castOp = SpirvOp::OpConvertFToS;
							break;

						case Ast::PrimitiveType::Int32:
							break; //< Already handled

						case Ast::PrimitiveType::UInt32:
							castOp = SpirvOp::OpBitcast;
							break;

						case Ast::PrimitiveType::String:
							throw std::runtime_error("unexpected string type");
					}
					break;
				}

				case Ast::PrimitiveType::UInt32:
				{
					switch (fromBaseType)
					{
						case Ast::PrimitiveType::Boolean:
							throw std::runtime_error("unsupported cast from boolean");

						case Ast::PrimitiveType::Float32:
						case Ast::PrimitiveType::Float64:
							castOp = SpirvOp::OpConvertFToU;
							break;

						case Ast::PrimitiveType::Int32:
							castOp = SpirvOp::OpBitcast;
							break;

						case Ast::PrimitiveType::UInt32:
							break; //< Already handled

						case Ast::PrimitiveType::String:
							throw std::runtime_error("unexpected string type");
					}
					break;
				}

				case Ast::PrimitiveType::String:
					throw std::runtime_error("unexpected string type");
			}

			assert(castOp);

			std::uint32_t resultId = m_writer.AllocateResultId();
			m_currentBlock->Append(*castOp, m_writer.GetTypeId(targetExprType), resultId, fromId);

			PushResultId(resultId);
		}
		else
		{
			assert(IsArrayType(targetExprType) || IsVectorType(targetExprType));
			Nz::StackVector<std::uint32_t> exprResults = NazaraStackVector(std::uint32_t, node.expressions.size());

			for (auto& exprPtr : node.expressions)
				exprResults.push_back(EvaluateExpression(*exprPtr));

			HandleSourceLocation(node.sourceLocation);

			std::uint32_t resultId = m_writer.AllocateResultId();

			m_currentBlock->AppendVariadic(SpirvOp::OpCompositeConstruct, [&](const auto& appender)
			{
				appender(m_writer.GetTypeId(targetExprType));
				appender(resultId);

				for (std::uint32_t exprResultId : exprResults)
					appender(exprResultId);
			});

			PushResultId(resultId);
		}
	}

	void SpirvAstVisitor::Visit(Ast::ConstantExpression& node)
	{
		HandleSourceLocation(node.sourceLocation);

		SpirvExpressionLoad accessMemberVisitor(m_writer, *this, *m_currentBlock);
		PushResultId(accessMemberVisitor.Evaluate(node));
	}

	void SpirvAstVisitor::Visit(Ast::ConstantValueExpression& node)
	{
		PushResultId(m_writer.GetSingleConstantId(node.value));
	}

	void SpirvAstVisitor::Visit(Ast::ContinueStatement& node)
	{
		if (!m_continueTarget)
			throw std::runtime_error("continue statement outside of while");

		HandleSourceLocation(node.sourceLocation);

		m_currentBlock->Append(SpirvOp::OpBranch, *m_continueTarget);
	}

	void SpirvAstVisitor::Visit(Ast::DeclareConstStatement& /*node*/)
	{
		/* Handled by the previsitor - nothing to do */
	}

	void SpirvAstVisitor::Visit(Ast::DeclareExternalStatement& /*node*/)
	{
		/* Handled by the previsitor - nothing to do */
	}

	void SpirvAstVisitor::Visit(Ast::DeclareFunctionStatement& node)
	{
		assert(node.funcIndex);
		m_currentFunc = &m_functionRetriever(*node.funcIndex);
		m_funcCallIndex = 0;

		HandleSourceLocation(node.sourceLocation);

		m_instructions.Append(SpirvOp::OpFunction, m_currentFunc->returnTypeId, m_currentFunc->funcId, 0, m_currentFunc->funcTypeId);

		if (!m_currentFunc->parameters.empty())
		{
			assert(node.parameters.size() == m_currentFunc->parameters.size());
			for (std::size_t i = 0; i < m_currentFunc->parameters.size(); ++i)
			{
				HandleSourceLocation(node.parameters[i].sourceLocation);

				std::uint32_t paramResultId = m_writer.AllocateResultId();
				m_instructions.Append(SpirvOp::OpFunctionParameter, m_currentFunc->parameters[i].pointerTypeId, paramResultId);

				RegisterVariable(*node.parameters[i].varIndex, m_currentFunc->parameters[i].typeId, paramResultId, SpirvStorageClass::Function);
			}
		}

		auto contentBlock = std::make_unique<SpirvBlock>(m_writer);
		m_currentBlock = contentBlock.get();

		m_functionBlocks.clear();
		m_functionBlocks.emplace_back(std::move(contentBlock));

		Nz::CallOnExit resetCurrentBlock([&] { m_currentBlock = nullptr; });

		ResetSourceLocation();

		for (auto& var : m_currentFunc->variables)
		{
			HandleSourceLocation(var.sourceLocation);

			var.varId = m_writer.AllocateResultId();
			m_currentBlock->Append(SpirvOp::OpVariable, var.typeId, var.varId, SpirvStorageClass::Function);
		}

		if (m_currentFunc->entryPointData)
		{
			auto& entryPointData = *m_currentFunc->entryPointData;
			if (entryPointData.inputStruct)
			{
				auto& inputStruct = *entryPointData.inputStruct;

				std::uint32_t paramId = m_writer.AllocateResultId();
				m_currentBlock->Append(SpirvOp::OpVariable, inputStruct.pointerId, paramId, SpirvStorageClass::Function);

				for (const auto& input : entryPointData.inputs)
				{
					std::uint32_t resultId = m_writer.AllocateResultId();
					m_currentBlock->Append(SpirvOp::OpAccessChain, input.memberPointerId, resultId, paramId, input.memberIndexConstantId);
					m_currentBlock->Append(SpirvOp::OpCopyMemory, resultId, input.varId);
				}

				RegisterVariable(*node.parameters.front().varIndex, inputStruct.typeId, paramId, SpirvStorageClass::Function);
			}
		}

		HandleStatementList(node.statements);

		// Add implicit return
		if (!m_functionBlocks.back()->IsTerminated())
			m_functionBlocks.back()->Append(SpirvOp::OpReturn);

		for (std::unique_ptr<SpirvBlock>& block : m_functionBlocks)
			m_instructions.AppendSection(*block);

		m_instructions.Append(SpirvOp::OpFunctionEnd);
	}

	void SpirvAstVisitor::Visit(Ast::DeclareOptionStatement& /*node*/)
	{
		/* nothing to do */
	}

	void SpirvAstVisitor::Visit(Ast::DeclareStructStatement& /*node*/)
	{
		/* nothing to do (handled by pre-visitor) */
	}

	void SpirvAstVisitor::Visit(Ast::DeclareVariableStatement& node)
	{
		std::uint32_t typeId = m_writer.GetTypeId(node.varType.GetResultingValue());

		assert(node.varIndex);
		auto varIt = m_currentFunc->varIndexToVarId.find(*node.varIndex);
		std::uint32_t varId = m_currentFunc->variables[varIt->second].varId;

		RegisterVariable(*node.varIndex, typeId, varId, SpirvStorageClass::Function);

		if (node.initialExpression)
		{
			std::uint32_t value = EvaluateExpression(*node.initialExpression);
			m_currentBlock->Append(SpirvOp::OpStore, varId, value);
		}
	}

	void SpirvAstVisitor::Visit(Ast::DiscardStatement& node)
	{
		HandleSourceLocation(node.sourceLocation);

		m_currentBlock->Append(SpirvOp::OpKill);
	}

	void SpirvAstVisitor::Visit(Ast::ExpressionStatement& node)
	{
		node.expression->Visit(*this);

		PopResultId();
	}

	void SpirvAstVisitor::Visit(Ast::IntrinsicExpression& node)
	{
		auto it = SpirvGenData::s_intrinsicData.find(node.intrinsic);
		if (it == SpirvGenData::s_intrinsicData.end())
			throw std::runtime_error("unknown intrinsic value " + std::to_string(Nz::UnderlyingCast(node.intrinsic)));

		std::visit([&](auto&& arg)
		{
			using namespace SpirvGenData;

			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, SpirvGlslStd450Op> || std::is_same_v<T, SpirvGlslStd450Selector>)
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				SpirvGlslStd450Op op;
				if constexpr (std::is_same_v<T, SpirvGlslStd450Selector>)
					op = arg(node);
				else
					op = arg;

				std::uint32_t resultTypeId = m_writer.GetTypeId(ResolveAlias(EnsureExpressionType(node)));

				Nz::StackArray<std::uint32_t> parameterIds = NazaraStackArrayNoInit(std::uint32_t, node.parameters.size());
				for (std::size_t i = 0; i < node.parameters.size(); ++i)
					parameterIds[i] = EvaluateExpression(*node.parameters[i]);

				HandleSourceLocation(node.sourceLocation);

				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->AppendVariadic(SpirvOp::OpExtInst, [&](auto&& append)
				{
					append(resultTypeId);
					append(resultId);
					append(glslInstructionSet);
					append(op);

					for (std::uint32_t parameterId : parameterIds)
						append(parameterId);
				});

				PushResultId(resultId);
			}
			else if constexpr (std::is_same_v<T, SpirvOp>)
			{
				const Ast::ExpressionType& resultType = ResolveAlias(EnsureExpressionType(node));
				std::uint32_t resultTypeId;
				if (!IsNoType(resultType))
					resultTypeId = m_writer.GetTypeId(resultType);
				else
					resultTypeId = 0;

				Nz::StackArray<std::uint32_t> parameterIds = NazaraStackArrayNoInit(std::uint32_t, node.parameters.size());
				for (std::size_t i = 0; i < node.parameters.size(); ++i)
					parameterIds[i] = EvaluateExpression(*node.parameters[i]);

				std::uint32_t resultId;
				if (resultTypeId != 0)
					resultId = m_writer.AllocateResultId();
				else
					resultId = 0;

				HandleSourceLocation(node.sourceLocation);

				m_currentBlock->AppendVariadic(arg, [&](auto&& append)
				{
					if (resultTypeId != 0)
					{
						append(resultTypeId);
						assert(resultId != 0);
						append(resultId);
					}

					for (std::uint32_t parameterId : parameterIds)
						append(parameterId);
				});

				PushResultId(resultId);
			}
			else if constexpr (std::is_same_v<T, SpirvCodeGenerator>)
			{
				std::invoke(arg, this, node);
			}
			else
				static_assert(Nz::AlwaysFalse<T>(), "non-exhaustive visitor");
		}, it->second.op);
	}

	void SpirvAstVisitor::Visit(Ast::NoOpStatement& /*node*/)
	{
		// nothing to do
	}

	void SpirvAstVisitor::Visit(Ast::MultiStatement& node)
	{
		HandleStatementList(node.statements);
	}

	void SpirvAstVisitor::Visit(Ast::ReturnStatement& node)
	{
		if (node.returnExpr)
		{
			// Handle entry point return
			if (m_currentFunc->entryPointData)
			{
				auto& entryPointData = *m_currentFunc->entryPointData;
				if (entryPointData.outputStructTypeId)
				{
					std::uint32_t paramId = EvaluateExpression(*node.returnExpr);
					for (const auto& output : entryPointData.outputs)
					{
						std::uint32_t resultId = m_writer.AllocateResultId();
						m_currentBlock->Append(SpirvOp::OpCompositeExtract, output.typeId, resultId, paramId, output.memberIndex);
						m_currentBlock->Append(SpirvOp::OpStore, output.varId, resultId);
					}
				}

				HandleSourceLocation(node.sourceLocation);

				m_currentBlock->Append(SpirvOp::OpReturn);
			}
			else
			{
				HandleSourceLocation(node.sourceLocation);

				m_currentBlock->Append(SpirvOp::OpReturnValue, EvaluateExpression(*node.returnExpr));
			}
		}
		else
		{
			HandleSourceLocation(node.sourceLocation);

			m_currentBlock->Append(SpirvOp::OpReturn);
		}
	}

	void SpirvAstVisitor::Visit(Ast::ScopedStatement& node)
	{
		node.statement->Visit(*this);
	}

	void SpirvAstVisitor::Visit(Ast::SwizzleExpression& node)
	{
		const Ast::ExpressionType* swizzledExpressionType = GetExpressionType(*node.expression);
		assert(swizzledExpressionType);

		std::uint32_t exprResultId = EvaluateExpression(*node.expression);

		const Ast::ExpressionType* targetExprType = GetExpressionType(node);
		assert(targetExprType);

		HandleSourceLocation(node.sourceLocation);

		if (node.componentCount > 1)
		{
			assert(IsVectorType(*targetExprType));

			const Ast::VectorType& targetType = std::get<Ast::VectorType>(*targetExprType);

			std::uint32_t resultId = m_writer.AllocateResultId();
			if (IsVectorType(*swizzledExpressionType))
			{
				// Swizzling a vector is implemented via OpVectorShuffle using the same vector twice as operands
				m_currentBlock->AppendVariadic(SpirvOp::OpVectorShuffle, [&](const auto& appender)
				{
					appender(m_writer.GetTypeId(targetType));
					appender(resultId);
					appender(exprResultId);
					appender(exprResultId);

					for (std::size_t i = 0; i < node.componentCount; ++i)
						appender(node.components[i]);
				});
			}
			else
			{
				assert(IsPrimitiveType(*swizzledExpressionType));

				// Swizzling a primitive to a vector (a.xxx) can be implemented using OpCompositeConstruct
				m_currentBlock->AppendVariadic(SpirvOp::OpCompositeConstruct, [&](const auto& appender)
				{
					appender(m_writer.GetTypeId(targetType));
					appender(resultId);

					for (std::size_t i = 0; i < node.componentCount; ++i)
						appender(exprResultId);
				});
			}

			PushResultId(resultId);
		}
		else if (IsVectorType(*swizzledExpressionType))
		{
			assert(IsPrimitiveType(*targetExprType));
			Ast::PrimitiveType targetType = std::get<Ast::PrimitiveType>(*targetExprType);

			// Extract a single component from the vector
			assert(node.componentCount == 1);

			std::uint32_t resultId = m_writer.AllocateResultId();
			m_currentBlock->Append(SpirvOp::OpCompositeExtract, m_writer.GetTypeId(targetType), resultId, exprResultId, node.components[0]);

			PushResultId(resultId);
		}
		else
		{
			// Swizzling a primitive to itself (a.x for example), don't do anything
			assert(IsPrimitiveType(*swizzledExpressionType));
			assert(IsPrimitiveType(*targetExprType));
			assert(node.componentCount == 1);
			assert(node.components[0] == 0);

			PushResultId(exprResultId);
		}
	}

	void SpirvAstVisitor::Visit(Ast::UnaryExpression& node)
	{
		const Ast::ExpressionType* resultType = GetExpressionType(node);
		assert(resultType);

		const Ast::ExpressionType* exprType = GetExpressionType(*node.expression);
		assert(exprType);

		std::uint32_t operand = EvaluateExpression(*node.expression);

		std::uint32_t resultId = [&]
		{
			switch (node.op)
			{
				case Ast::UnaryType::BitwiseNot: 
				{
					assert(IsPrimitiveType(*exprType));
					assert(std::get<Ast::PrimitiveType>(*resultType) == Ast::PrimitiveType::Int32 || std::get<Ast::PrimitiveType>(*resultType) == Ast::PrimitiveType::UInt32);

					HandleSourceLocation(node.sourceLocation);
					std::uint32_t resultId = m_writer.AllocateResultId();
					m_currentBlock->Append(SpirvOp::OpNot, m_writer.GetTypeId(*resultType), resultId, operand);

					return resultId;
				}

				case Ast::UnaryType::LogicalNot:
				{
					assert(IsPrimitiveType(*exprType));
					assert(std::get<Ast::PrimitiveType>(*resultType) == Ast::PrimitiveType::Boolean);

					HandleSourceLocation(node.sourceLocation);

					std::uint32_t resultId = m_writer.AllocateResultId();
					m_currentBlock->Append(SpirvOp::OpLogicalNot, m_writer.GetTypeId(*resultType), resultId, operand);

					return resultId;
				}

				case Ast::UnaryType::Minus:
				{
					Ast::PrimitiveType basicType;
					if (IsPrimitiveType(*exprType))
						basicType = std::get<Ast::PrimitiveType>(*exprType);
					else if (IsVectorType(*exprType))
						basicType = std::get<Ast::VectorType>(*exprType).type;
					else
						throw std::runtime_error("unexpected expression type");

					HandleSourceLocation(node.sourceLocation);

					std::uint32_t resultId = m_writer.AllocateResultId();

					switch (basicType)
					{
						case Ast::PrimitiveType::Float32:
							m_currentBlock->Append(SpirvOp::OpFNegate, m_writer.GetTypeId(*resultType), resultId, operand);
							return resultId;

						case Ast::PrimitiveType::Int32:
						case Ast::PrimitiveType::UInt32:
							m_currentBlock->Append(SpirvOp::OpSNegate, m_writer.GetTypeId(*resultType), resultId, operand);
							return resultId;

						default:
							break;
					}
					break;
				}

				case Ast::UnaryType::Plus:
					return operand;
			}

			throw std::runtime_error("unexpected unary operation");
		}();

		PushResultId(resultId);
	}

	void SpirvAstVisitor::Visit(Ast::VariableValueExpression& node)
	{
		HandleSourceLocation(node.sourceLocation);

		SpirvExpressionLoad loadVisitor(m_writer, *this, *m_currentBlock);
		PushResultId(loadVisitor.Evaluate(node));
	}

	void SpirvAstVisitor::Visit(Ast::WhileStatement& node)
	{
		assert(node.condition);
		assert(node.body);

		auto headerBlock = std::make_unique<SpirvBlock>(m_writer);
		auto bodyBlock = std::make_unique<SpirvBlock>(m_writer);
		auto mergeBlock = std::make_unique<SpirvBlock>(m_writer);
		auto continueBlock = std::make_unique<SpirvBlock>(m_writer);

		HandleSourceLocation(node.sourceLocation);

		m_currentBlock->Append(SpirvOp::OpBranch, headerBlock->GetLabelId());
		m_currentBlock = headerBlock.get();

		std::uint32_t expressionId = EvaluateExpression(*node.condition);

		SpirvLoopControl loopControl = [&]
		{
			if (node.unroll.HasValue())
			{
				switch (node.unroll.GetResultingValue())
				{
					case Ast::LoopUnroll::Always:
						// it shouldn't be possible to have this attribute as the loop gets unrolled in the sanitizer
						break;

					case Ast::LoopUnroll::Hint:
						return SpirvLoopControl::Unroll;

					case Ast::LoopUnroll::Never:
						return SpirvLoopControl::DontUnroll;
				}

				throw std::runtime_error("unexpected unroll attribute");
			}
			else
				return SpirvLoopControl::None;
		}();

		m_currentBlock->Append(SpirvOp::OpLoopMerge, mergeBlock->GetLabelId(), continueBlock->GetLabelId(), loopControl);
		m_currentBlock->Append(SpirvOp::OpBranchConditional, expressionId, bodyBlock->GetLabelId(), mergeBlock->GetLabelId());

		std::uint32_t headerLabelId = headerBlock->GetLabelId();
		std::uint32_t continueLabelId = continueBlock->GetLabelId();

		m_currentBlock = bodyBlock.get();
		m_functionBlocks.emplace_back(std::move(headerBlock));
		m_functionBlocks.emplace_back(std::move(bodyBlock));

		m_breakTarget = mergeBlock->GetLabelId();
		m_continueTarget = continueLabelId;
		{
			node.body->Visit(*this);
		}
		m_breakTarget = std::nullopt;
		m_continueTarget = std::nullopt;

		// Continue block sole purpose is to jump back to the header block
		continueBlock->Append(SpirvOp::OpBranch, headerLabelId);
		m_functionBlocks.emplace_back(std::move(continueBlock));

		// Jump to continue block
		m_currentBlock->Append(SpirvOp::OpBranch, continueLabelId);

		m_functionBlocks.emplace_back(std::move(mergeBlock));
		m_currentBlock = m_functionBlocks.back().get();
	}

	void SpirvAstVisitor::BuildArraySizeIntrinsic(const Ast::IntrinsicExpression& node)
	{
		// First parameter must be an AccessIndex from the external variable to the member index
		std::uint32_t typeId = m_writer.GetTypeId(Ast::PrimitiveType::UInt32);

		if (node.parameters.size() != 1)
			throw std::runtime_error("ArraySize intrinsic: unexpected parameter count");

		const Ast::ExpressionPtr& firstParameter = node.parameters.front();
		if (firstParameter->GetType() != Ast::NodeType::AccessIndexExpression)
			throw std::runtime_error("ArraySize intrinsic: parameter is not AccessIndex");

		const Ast::AccessIndexExpression& accessIndex = Nz::SafeCast<const Ast::AccessIndexExpression&>(*firstParameter);
		if (accessIndex.expr->GetType() != Ast::NodeType::VariableValueExpression)
			throw std::runtime_error("ArraySize intrinsic: AccessIndex expr is not a variable");

		if (accessIndex.indices.size() != 1)
			throw std::runtime_error("ArraySize intrinsic: AcessIndex should have exactly one index");

		if (accessIndex.indices[0]->GetType() != Ast::NodeType::ConstantValueExpression)
			throw std::runtime_error("ArraySize intrinsic: AcessIndex index must be a constant value");

		const Ast::VariableValueExpression& structVar = static_cast<const Ast::VariableValueExpression&>(*accessIndex.expr);

		std::uint32_t structId = m_writer.GetExtVar(structVar.variableId).pointerId;

		const Ast::ConstantValueExpression& memberConstant = Nz::SafeCast<const Ast::ConstantValueExpression&>(*accessIndex.indices[0]);
		if (!std::holds_alternative<std::int32_t>(memberConstant.value))
			throw std::runtime_error("ArraySize intrinsic: AcessIndex index constant value must be a int32");

		std::uint32_t arrayMemberIndex = Nz::SafeCast<std::uint32_t>(std::get<std::int32_t>(memberConstant.value));

		HandleSourceLocation(node.sourceLocation);

		std::uint32_t resultId = m_writer.AllocateResultId();
		m_currentBlock->Append(SpirvOp::OpArrayLength, typeId, resultId, structId, arrayMemberIndex);

		PushResultId(resultId);
	}

	void SpirvAstVisitor::BuildSelectIntrinsic(const Ast::IntrinsicExpression& node)
	{
		if (node.parameters.size() != 3)
			throw std::runtime_error("ArraySize intrinsic: unexpected parameter count");

		std::uint32_t resultTypeId = m_writer.GetTypeId(ResolveAlias(EnsureExpressionType(node)));

		std::array<std::uint32_t, 3> parameterIds;
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
			parameterIds[i] = EvaluateExpression(*node.parameters[i]);

		const Ast::ExpressionType& condType = ResolveAlias(EnsureExpressionType(*node.parameters[0]));
		const Ast::ExpressionType& paramType = ResolveAlias(EnsureExpressionType(*node.parameters[1]));
		assert(paramType == ResolveAlias(EnsureExpressionType(*node.parameters[2])));

		HandleSourceLocation(node.sourceLocation);

		// OpSelect cannot select parameters using a single boolean before SPIR-V 1.4
		if (!m_writer.IsVersionGreaterOrEqual(1, 4) && IsVectorType(paramType) && !IsVectorType(condType))
		{
			const Ast::VectorType& paramVec = std::get<Ast::VectorType>(paramType);
			Ast::VectorType condVec{ paramVec.componentCount, Ast::PrimitiveType::Boolean };

			std::uint32_t condVecId = m_writer.RegisterType(condVec);

			std::uint32_t bVecId = m_writer.AllocateResultId();

			m_currentBlock->AppendVariadic(SpirvOp::OpCompositeConstruct, [&](const auto& appender)
			{
				appender(condVecId);
				appender(bVecId);

				for (std::size_t i = 0; i < paramVec.componentCount; ++i)
					appender(parameterIds[0]);
			});

			parameterIds[0] = bVecId;
		}

		std::uint32_t resultId = m_writer.AllocateResultId();

		m_currentBlock->AppendVariadic(SpirvOp::OpSelect, [&](auto&& append)
		{
			append(resultTypeId);
			append(resultId);

			for (std::uint32_t parameterId : parameterIds)
				append(parameterId);
		});

		PushResultId(resultId);
	}

	SpirvGlslStd450Op SpirvAstVisitor::SelectAbs(const Ast::IntrinsicExpression& node)
	{
		if (node.parameters.size() != 1)
			throw std::runtime_error("abs intrinsic: unexpected parameter count");

		const Ast::ExpressionType& parameterType = EnsureExpressionType(*node.parameters[0]);

		Ast::PrimitiveType basicType;
		if (IsPrimitiveType(parameterType))
			basicType = std::get<Ast::PrimitiveType>(parameterType);
		else if (IsVectorType(parameterType))
			basicType = std::get<Ast::VectorType>(parameterType).type;
		else
			throw std::runtime_error("unexpected expression type");

		switch (basicType)
		{
			case Ast::PrimitiveType::Float32:
			case Ast::PrimitiveType::Float64:
				return SpirvGlslStd450Op::FAbs;

			case Ast::PrimitiveType::Int32:
				return SpirvGlslStd450Op::SAbs;

			case Ast::PrimitiveType::Boolean:
			case Ast::PrimitiveType::String:
			case Ast::PrimitiveType::UInt32:
				break;
		}

		throw std::runtime_error("unexpected type " + ToString(basicType) + " for abs intrinsic");
	}

	SpirvGlslStd450Op SpirvAstVisitor::SelectClamp(const Ast::IntrinsicExpression& node)
	{
		if (node.parameters.size() != 3)
			throw std::runtime_error("clamp intrinsic: unexpected parameter count");

		const Ast::ExpressionType& parameterType = EnsureExpressionType(*node.parameters[0]);

		Ast::PrimitiveType basicType;
		if (IsPrimitiveType(parameterType))
			basicType = std::get<Ast::PrimitiveType>(parameterType);
		else if (IsVectorType(parameterType))
			basicType = std::get<Ast::VectorType>(parameterType).type;
		else
			throw std::runtime_error("unexpected expression type");

		switch (basicType)
		{
			case Ast::PrimitiveType::Float32:
			case Ast::PrimitiveType::Float64:
				return SpirvGlslStd450Op::FClamp;

			case Ast::PrimitiveType::Int32:
				return SpirvGlslStd450Op::SClamp;

			case Ast::PrimitiveType::UInt32:
				return SpirvGlslStd450Op::UClamp;

			case Ast::PrimitiveType::Boolean:
			case Ast::PrimitiveType::String:
				break;
		}

		throw std::runtime_error("unexpected type " + ToString(basicType) + " for clamp intrinsic");
	}

	SpirvGlslStd450Op SpirvAstVisitor::SelectLerp(const Ast::IntrinsicExpression& node)
	{
		if (node.parameters.size() != 3)
			throw std::runtime_error("lerp intrinsic: unexpected parameter count");

		const Ast::ExpressionType& parameterType = EnsureExpressionType(*node.parameters[0]);

		Ast::PrimitiveType basicType;
		if (IsPrimitiveType(parameterType))
			basicType = std::get<Ast::PrimitiveType>(parameterType);
		else if (IsVectorType(parameterType))
			basicType = std::get<Ast::VectorType>(parameterType).type;
		else
			throw std::runtime_error("unexpected expression type");

		switch (basicType)
		{
			case Ast::PrimitiveType::Float32:
			case Ast::PrimitiveType::Float64:
				return SpirvGlslStd450Op::FMix;

			case Ast::PrimitiveType::Int32:
				return SpirvGlslStd450Op::IMix;

			case Ast::PrimitiveType::Boolean:
			case Ast::PrimitiveType::String:
			case Ast::PrimitiveType::UInt32:
				break;
		}

		throw std::runtime_error("unexpected type " + ToString(basicType) + " for lerp intrinsic");
	}

	SpirvGlslStd450Op SpirvAstVisitor::SelectMaxMin(const Ast::IntrinsicExpression& node)
	{
		assert(node.intrinsic == Ast::IntrinsicType::Max || node.intrinsic == Ast::IntrinsicType::Min);
		bool isMax = (node.intrinsic == Ast::IntrinsicType::Max);

		if (node.parameters.size() != 2)
			throw std::runtime_error("max/min intrinsic: unexpected parameter count");

		const Ast::ExpressionType& parameterType = EnsureExpressionType(*node.parameters[0]);

		Ast::PrimitiveType basicType;
		if (IsPrimitiveType(parameterType))
			basicType = std::get<Ast::PrimitiveType>(parameterType);
		else if (IsVectorType(parameterType))
			basicType = std::get<Ast::VectorType>(parameterType).type;
		else
			throw std::runtime_error("unexpected expression type");

		switch (basicType)
		{
			case Ast::PrimitiveType::Float32:
			case Ast::PrimitiveType::Float64:
				return (isMax) ? SpirvGlslStd450Op::FMax : SpirvGlslStd450Op::FMin;

			case Ast::PrimitiveType::Int32:
				return (isMax) ? SpirvGlslStd450Op::SMax : SpirvGlslStd450Op::SMin;

			case Ast::PrimitiveType::UInt32:
				return (isMax) ? SpirvGlslStd450Op::UMax : SpirvGlslStd450Op::UMin;

			case Ast::PrimitiveType::Boolean:
			case Ast::PrimitiveType::String:
				break;
		}

		throw std::runtime_error("unexpected type " + ToString(basicType) + " for max/min intrinsic");
	}

	SpirvGlslStd450Op SpirvAstVisitor::SelectSign(const Ast::IntrinsicExpression& node)
	{
		if (node.parameters.size() != 1)
			throw std::runtime_error("sign intrinsic: unexpected parameter count");

		const Ast::ExpressionType& parameterType = EnsureExpressionType(*node.parameters[0]);

		Ast::PrimitiveType basicType;
		if (IsPrimitiveType(parameterType))
			basicType = std::get<Ast::PrimitiveType>(parameterType);
		else if (IsVectorType(parameterType))
			basicType = std::get<Ast::VectorType>(parameterType).type;
		else
			throw std::runtime_error("unexpected expression type");

		switch (basicType)
		{
			case Ast::PrimitiveType::Float32:
			case Ast::PrimitiveType::Float64:
				return SpirvGlslStd450Op::FSign;

			case Ast::PrimitiveType::Int32:
				return SpirvGlslStd450Op::SSign;

			case Ast::PrimitiveType::Boolean:
			case Ast::PrimitiveType::String:
			case Ast::PrimitiveType::UInt32:
				break;
		}

		throw std::runtime_error("unexpected type " + ToString(basicType) + " for sign intrinsic");
	}

	void SpirvAstVisitor::HandleSourceLocation(const SourceLocation& sourceLocation)
	{
		if (!m_writer.HasDebugInfo(DebugLevel::Regular))
			return;

		if (!sourceLocation.IsValid() || !sourceLocation.file)
			return;

		if (m_lastLocation.file == sourceLocation.file && m_lastLocation.startLine == sourceLocation.startLine && m_lastLocation.startColumn == sourceLocation.startColumn)
			return;

		std::uint32_t fileId = m_writer.GetSourceFileId(sourceLocation.file);

		if (m_currentBlock)
			m_currentBlock->Append(SpirvOp::OpLine, fileId, sourceLocation.startLine, sourceLocation.startColumn);
		else
			m_instructions.Append(SpirvOp::OpLine, fileId, sourceLocation.startLine, sourceLocation.startColumn);

		m_lastLocation = sourceLocation;
	}

	void SpirvAstVisitor::HandleStatementList(const std::vector<Ast::StatementPtr>& statements)
	{
		for (auto& statement : statements)
		{
			statement->Visit(*this);

			if (m_currentBlock && m_currentBlock->IsTerminated())
				return; //< stop processing statement after a termination instruction (discard/return)
		}
	}

	void SpirvAstVisitor::PushResultId(std::uint32_t value)
	{
		m_resultIds.push_back(value);
	}

	std::uint32_t SpirvAstVisitor::PopResultId()
	{
		if (m_resultIds.empty())
			throw std::runtime_error("invalid operation");

		std::uint32_t resultId = m_resultIds.back();
		m_resultIds.pop_back();

		return resultId;
	}

	void SpirvAstVisitor::ResetSourceLocation()
	{
		if (!m_writer.HasDebugInfo(DebugLevel::Regular))
			return;

		if (!m_lastLocation.IsValid())
			return;

		assert(m_currentBlock);
		m_currentBlock->Append(SpirvOp::OpNoLine);

		m_lastLocation = SourceLocation{};
	}
}
