// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirvAstVisitor.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <Nazara/Utils/CallOnExit.hpp>
#include <Nazara/Utils/StackArray.hpp>
#include <Nazara/Utils/StackVector.hpp>
#include <NZSL/SpirvExpressionLoad.hpp>
#include <NZSL/SpirvExpressionStore.hpp>
#include <NZSL/SpirvSection.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <SpirV/GLSL.std.450.h>

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
		return Nz::Retrieve(m_variables, varIndex);
	}

	void SpirvAstVisitor::Visit(Ast::AccessIndexExpression& node)
	{
		SpirvExpressionLoad accessMemberVisitor(m_writer, *this, *m_currentBlock);
		PushResultId(accessMemberVisitor.Evaluate(node));
	}

	void SpirvAstVisitor::Visit(Ast::AssignExpression& node)
	{
		if (node.op != Ast::AssignType::Simple)
			throw std::runtime_error("unexpected assign expression (should have been removed by sanitization)");

		std::uint32_t resultId = EvaluateExpression(*node.right);

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

		bool swapOperands = false;

		SpirvOp op = [&]
		{
			switch (node.op)
			{
				case Ast::BinaryType::Add:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
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
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
							return SpirvOp::OpFDiv;

						case Ast::PrimitiveType::Int32:
							return SpirvOp::OpSDiv;

						case Ast::PrimitiveType::UInt32:
							return SpirvOp::OpUDiv;

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
							return SpirvOp::OpIMul;

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
				
				case Ast::BinaryType::CompGt:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
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
				
				case Ast::BinaryType::CompLe:
				{
					switch (leftTypeBase)
					{
						case Ast::PrimitiveType::Float32:
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
			}

			assert(false);
			throw std::runtime_error("unexpected binary operation");
		}();

		if (swapOperands)
			std::swap(leftOperand, rightOperand);

		if (node.op == Ast::BinaryType::Divide)
		{
			//TODO: Handle other cases
			if (IsVectorType(leftType) && IsPrimitiveType(rightType))
			{
				const Ast::VectorType& leftVec = std::get<Ast::VectorType>(leftType);

				std::uint32_t vecType = m_writer.GetTypeId(leftType);

				std::uint32_t rightAsVec = m_writer.AllocateResultId();
				m_currentBlock->AppendVariadic(SpirvOp::OpCompositeConstruct, [&](auto&& append)
				{
					append(vecType);
					append(rightAsVec);

					for (std::size_t i = 0; i < leftVec.componentCount; ++i)
						append(rightOperand);
				});

				rightOperand = rightAsVec;
			}
			else if (leftType != rightType)
				throw std::runtime_error("unexpected division operands");
		}

		m_currentBlock->Append(op, m_writer.GetTypeId(resultType), resultId, leftOperand, rightOperand);
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

	void SpirvAstVisitor::Visit(Ast::CallFunctionExpression& node)
	{
		std::size_t functionIndex = std::get<Ast::FunctionType>(*GetExpressionType(*node.targetFunction)).funcIndex;

		std::uint32_t funcId = 0;
		for (const auto& [funcIndex, func] : m_funcData)
		{
			if (funcIndex == functionIndex)
			{
				funcId = func.funcId;
				break;
			}
		}
		assert(funcId != 0);

		const FuncData& funcData = Nz::Retrieve(m_funcData, m_funcIndex);
		const auto& funcCall = funcData.funcCalls[m_funcCallIndex++];

		Nz::StackArray<std::uint32_t> parameterIds = NazaraStackArrayNoInit(std::uint32_t, node.parameters.size());
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			std::uint32_t resultId = EvaluateExpression(*node.parameters[i]);
			std::uint32_t varId = funcData.variables[funcCall.firstVarIndex + i].varId;
			m_currentBlock->Append(SpirvOp::OpStore, varId, resultId);

			parameterIds[i] = varId;
		}

		std::uint32_t resultId = AllocateResultId();
		m_currentBlock->AppendVariadic(SpirvOp::OpFunctionCall, [&](auto&& appender)
		{
			appender(m_writer.GetTypeId(*Ast::GetExpressionType(node)));
			appender(resultId);
			appender(funcId);

			for (std::size_t i = 0; i < node.parameters.size(); ++i)
				appender(parameterIds[i]);
		});

		PushResultId(resultId);
	}

	void SpirvAstVisitor::Visit(Ast::CastExpression& node)
	{
		const Ast::ExpressionType& targetExprType = node.targetType.GetResultingValue();
		if (IsPrimitiveType(targetExprType))
		{
			Ast::PrimitiveType targetType = std::get<Ast::PrimitiveType>(targetExprType);

			assert(node.expressions.size() == 1);
			Ast::ExpressionPtr& expression = node.expressions[0];

			assert(expression->cachedExpressionType.has_value());
			const Ast::ExpressionType& exprType = expression->cachedExpressionType.value();
			assert(IsPrimitiveType(exprType));
			Ast::PrimitiveType fromType = std::get<Ast::PrimitiveType>(exprType);

			std::uint32_t fromId = EvaluateExpression(*expression);
			if (targetType == fromType)
				return PushResultId(fromId);

			std::optional<SpirvOp> castOp;
			switch (targetType)
			{
				case Ast::PrimitiveType::Boolean:
					throw std::runtime_error("unsupported cast to boolean");

				case Ast::PrimitiveType::Float32:
				{
					switch (fromType)
					{
						case Ast::PrimitiveType::Boolean:
							throw std::runtime_error("unsupported cast from boolean");

						case Ast::PrimitiveType::Float32:
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
					switch (fromType)
					{
						case Ast::PrimitiveType::Boolean:
							throw std::runtime_error("unsupported cast from boolean");

						case Ast::PrimitiveType::Float32:
							castOp = SpirvOp::OpConvertFToS;
							break;

						case Ast::PrimitiveType::Int32:
							break; //< Already handled

						case Ast::PrimitiveType::UInt32:
							throw std::runtime_error("unsupported cast from int32");

						case Ast::PrimitiveType::String:
							throw std::runtime_error("unexpected string type");
					}
					break;
				}

				case Ast::PrimitiveType::UInt32:
				{
					switch (fromType)
					{
						case Ast::PrimitiveType::Boolean:
							throw std::runtime_error("unsupported cast from boolean");

						case Ast::PrimitiveType::Float32:
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
			m_currentBlock->Append(*castOp, m_writer.GetTypeId(targetType), resultId, fromId);

			PushResultId(resultId);
		}
		else
		{
			assert(IsArrayType(targetExprType) || IsVectorType(targetExprType));
			Nz::StackVector<std::uint32_t> exprResults = NazaraStackVector(std::uint32_t, node.expressions.size());

			for (auto& exprPtr : node.expressions)
				exprResults.push_back(EvaluateExpression(*exprPtr));

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
		SpirvExpressionLoad accessMemberVisitor(m_writer, *this, *m_currentBlock);
		PushResultId(accessMemberVisitor.Evaluate(node));
	}

	void SpirvAstVisitor::Visit(Ast::ConstantValueExpression& node)
	{
		PushResultId(m_writer.GetSingleConstantId(node.value));
	}

	void SpirvAstVisitor::Visit(Ast::DeclareConstStatement& /*node*/)
	{
		/* Handled by the previsitor - nothing to do */
	}

	void SpirvAstVisitor::Visit(Ast::DeclareExternalStatement& node)
	{
		for (auto&& extVar : node.externalVars)
		{
			assert(extVar.varIndex);
			RegisterExternalVariable(*extVar.varIndex, extVar.type.GetResultingValue());
		}
	}

	void SpirvAstVisitor::Visit(Ast::DeclareFunctionStatement& node)
	{
		assert(node.funcIndex);
		m_funcIndex = *node.funcIndex;
		m_funcCallIndex = 0;

		auto& func = m_funcData[m_funcIndex];

		m_instructions.Append(SpirvOp::OpFunction, func.returnTypeId, func.funcId, 0, func.funcTypeId);

		if (!func.parameters.empty())
		{
			assert(node.parameters.size() == func.parameters.size());
			for (std::size_t i = 0; i < func.parameters.size(); ++i)
			{
				std::uint32_t paramResultId = m_writer.AllocateResultId();
				m_instructions.Append(SpirvOp::OpFunctionParameter, func.parameters[i].pointerTypeId, paramResultId);

				RegisterVariable(*node.parameters[i].varIndex, func.parameters[i].typeId, paramResultId, SpirvStorageClass::Function);
			}
		}

		auto contentBlock = std::make_unique<SpirvBlock>(m_writer);
		m_currentBlock = contentBlock.get();

		m_functionBlocks.clear();
		m_functionBlocks.emplace_back(std::move(contentBlock));

		Nz::CallOnExit resetCurrentBlock([&] { m_currentBlock = nullptr; });

		for (auto& var : func.variables)
		{
			var.varId = m_writer.AllocateResultId();
			m_currentBlock->Append(SpirvOp::OpVariable, var.typeId, var.varId, SpirvStorageClass::Function);
		}

		if (func.entryPointData)
		{
			auto& entryPointData = *func.entryPointData;
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

	void SpirvAstVisitor::Visit(Ast::DeclareStructStatement& node)
	{
		assert(node.structIndex);
		RegisterStruct(*node.structIndex, &node.description);
	}

	void SpirvAstVisitor::Visit(Ast::DeclareVariableStatement& node)
	{
		const auto& func = m_funcData[m_funcIndex];

		std::uint32_t typeId = m_writer.GetTypeId(node.varType.GetResultingValue());

		assert(node.varIndex);
		auto varIt = func.varIndexToVarId.find(*node.varIndex);
		std::uint32_t varId = func.variables[varIt->second].varId;

		RegisterVariable(*node.varIndex, typeId, varId, SpirvStorageClass::Function);

		if (node.initialExpression)
		{
			std::uint32_t value = EvaluateExpression(*node.initialExpression);
			m_currentBlock->Append(SpirvOp::OpStore, varId, value);
		}
	}

	void SpirvAstVisitor::Visit(Ast::DiscardStatement& /*node*/)
	{
		m_currentBlock->Append(SpirvOp::OpKill);
	}

	void SpirvAstVisitor::Visit(Ast::ExpressionStatement& node)
	{
		node.expression->Visit(*this);

		PopResultId();
	}

	void SpirvAstVisitor::Visit(Ast::IntrinsicExpression& node)
	{
		switch (node.intrinsic)
		{
			case Ast::IntrinsicType::ArraySize:
			{
				// First parameter must be an AccessIndex from the external variable to the member index
				std::uint32_t typeId = m_writer.GetTypeId(Ast::PrimitiveType::UInt32);

				assert(node.parameters.size() == 1);
				const Ast::ExpressionPtr& firstParameter = node.parameters.front();
				assert(firstParameter->GetType() == Ast::NodeType::AccessIndexExpression);
				const Ast::AccessIndexExpression& accessIndex = static_cast<const Ast::AccessIndexExpression&>(*firstParameter);

				assert(accessIndex.expr->GetType() == Ast::NodeType::VariableValueExpression);
				const Ast::VariableValueExpression& structVar = static_cast<const Ast::VariableValueExpression&>(*accessIndex.expr);

				std::uint32_t structId = GetVariable(structVar.variableId).pointerId;

				assert(accessIndex.indices.size() == 1);
				assert(accessIndex.indices[0]->GetType() == Ast::NodeType::ConstantValueExpression);
				const Ast::ConstantValueExpression& memberConstant = static_cast<const Ast::ConstantValueExpression&>(*accessIndex.indices[0]);
				assert(std::holds_alternative<std::int32_t>(memberConstant.value));
				std::uint32_t arrayMemberIndex = Nz::SafeCast<std::uint32_t>(std::get<std::int32_t>(memberConstant.value));

				std::uint32_t resultId = m_writer.AllocateResultId();
				m_currentBlock->Append(SpirvOp::OpArrayLength, typeId, resultId, structId, arrayMemberIndex);

				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::CrossProduct:
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				const Ast::ExpressionType* parameterType = GetExpressionType(*node.parameters[0]);
				assert(parameterType);
				assert(IsVectorType(*parameterType));

				std::uint32_t typeId = m_writer.GetTypeId(*parameterType);

				std::uint32_t firstParam = EvaluateExpression(*node.parameters[0]);
				std::uint32_t secondParam = EvaluateExpression(*node.parameters[1]);
				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpExtInst, typeId, resultId, glslInstructionSet, GLSLstd450Cross, firstParam, secondParam);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::DotProduct:
			{
				const Ast::ExpressionType* vecExprType = GetExpressionType(*node.parameters[0]);
				assert(vecExprType);
				assert(IsVectorType(*vecExprType));

				const Ast::VectorType& vecType = std::get<Ast::VectorType>(*vecExprType);

				std::uint32_t typeId = m_writer.GetTypeId(vecType.type);

				std::uint32_t vec1 = EvaluateExpression(*node.parameters[0]);
				std::uint32_t vec2 = EvaluateExpression(*node.parameters[1]);

				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpDot, typeId, resultId, vec1, vec2);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::Exp:
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				const Ast::ExpressionType* parameterType = GetExpressionType(*node.parameters[0]);
				assert(parameterType);
				assert(IsPrimitiveType(*parameterType) || IsVectorType(*parameterType));
				std::uint32_t typeId = m_writer.GetTypeId(*parameterType);

				std::uint32_t param = EvaluateExpression(*node.parameters[0]);
				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpExtInst, typeId, resultId, glslInstructionSet, GLSLstd450Exp, param);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::Length:
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				const Ast::ExpressionType* vecExprType = GetExpressionType(*node.parameters[0]);
				assert(vecExprType);
				assert(IsVectorType(*vecExprType));

				const Ast::VectorType& vecType = std::get<Ast::VectorType>(*vecExprType);
				std::uint32_t typeId = m_writer.GetTypeId(vecType.type);

				std::uint32_t vec = EvaluateExpression(*node.parameters[0]);

				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpExtInst, typeId, resultId, glslInstructionSet, GLSLstd450Length, vec);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::Max:
			case Ast::IntrinsicType::Min:
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				const Ast::ExpressionType* parameterType = GetExpressionType(*node.parameters[0]);
				assert(parameterType);
				assert(IsPrimitiveType(*parameterType) || IsVectorType(*parameterType));
				std::uint32_t typeId = m_writer.GetTypeId(*parameterType);

				Ast::PrimitiveType basicType;
				if (IsPrimitiveType(*parameterType))
					basicType = std::get<Ast::PrimitiveType>(*parameterType);
				else if (IsVectorType(*parameterType))
					basicType = std::get<Ast::VectorType>(*parameterType).type;
				else
					throw std::runtime_error("unexpected expression type");

				GLSLstd450 op;
				switch (basicType)
				{
					case Ast::PrimitiveType::Boolean:
						throw std::runtime_error("unexpected boolean for max/min intrinsic");

					case Ast::PrimitiveType::Float32:
						op = (node.intrinsic == Ast::IntrinsicType::Max) ? GLSLstd450FMax : GLSLstd450FMin;
						break;

					case Ast::PrimitiveType::Int32:
						op = (node.intrinsic == Ast::IntrinsicType::Max) ? GLSLstd450SMax : GLSLstd450SMin;
						break;

					case Ast::PrimitiveType::UInt32:
						op = (node.intrinsic == Ast::IntrinsicType::Max) ? GLSLstd450UMax : GLSLstd450UMin;
						break;

					case Ast::PrimitiveType::String:
						throw std::runtime_error("unexpected string type");
				}

				std::uint32_t firstParam = EvaluateExpression(*node.parameters[0]);
				std::uint32_t secondParam = EvaluateExpression(*node.parameters[1]);
				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpExtInst, typeId, resultId, glslInstructionSet, op, firstParam, secondParam);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::Normalize:
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				const Ast::ExpressionType* vecExprType = GetExpressionType(*node.parameters[0]);
				assert(vecExprType);
				assert(IsVectorType(*vecExprType));

				const Ast::VectorType& vecType = std::get<Ast::VectorType>(*vecExprType);
				std::uint32_t typeId = m_writer.GetTypeId(vecType);

				std::uint32_t vec = EvaluateExpression(*node.parameters[0]);

				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpExtInst, typeId, resultId, glslInstructionSet, GLSLstd450Normalize, vec);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::Pow:
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				const Ast::ExpressionType* parameterType = GetExpressionType(*node.parameters[0]);
				assert(parameterType);
				assert(IsPrimitiveType(*parameterType) || IsVectorType(*parameterType));
				std::uint32_t typeId = m_writer.GetTypeId(*parameterType);

				std::uint32_t firstParam = EvaluateExpression(*node.parameters[0]);
				std::uint32_t secondParam = EvaluateExpression(*node.parameters[1]);
				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpExtInst, typeId, resultId, glslInstructionSet, GLSLstd450Pow, firstParam, secondParam);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::Reflect:
			{
				std::uint32_t glslInstructionSet = m_writer.GetExtendedInstructionSet("GLSL.std.450");

				const Ast::ExpressionType* parameterType = GetExpressionType(*node.parameters[0]);
				assert(parameterType);
				assert(IsVectorType(*parameterType));
				std::uint32_t typeId = m_writer.GetTypeId(*parameterType);

				std::uint32_t firstParam = EvaluateExpression(*node.parameters[0]);
				std::uint32_t secondParam = EvaluateExpression(*node.parameters[1]);
				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpExtInst, typeId, resultId, glslInstructionSet, GLSLstd450Reflect, firstParam, secondParam);
				PushResultId(resultId);
				break;
			}

			case Ast::IntrinsicType::SampleTexture:
			{
				std::uint32_t typeId = m_writer.GetTypeId(Ast::VectorType{4, Ast::PrimitiveType::Float32});

				std::uint32_t samplerId = EvaluateExpression(*node.parameters[0]);
				std::uint32_t coordinatesId = EvaluateExpression(*node.parameters[1]);
				std::uint32_t resultId = m_writer.AllocateResultId();

				m_currentBlock->Append(SpirvOp::OpImageSampleImplicitLod, typeId, resultId, samplerId, coordinatesId);
				PushResultId(resultId);
				break;
			}

			default:
				throw std::runtime_error("not yet implemented");
		}
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
			const auto& func = m_funcData[m_funcIndex];
			if (func.entryPointData)
			{
				auto& entryPointData = *func.entryPointData;
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

				m_currentBlock->Append(SpirvOp::OpReturn);
			}
			else
				m_currentBlock->Append(SpirvOp::OpReturnValue, EvaluateExpression(*node.returnExpr));
		}
		else
			m_currentBlock->Append(SpirvOp::OpReturn);
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
				case Ast::UnaryType::LogicalNot:
				{
					assert(IsPrimitiveType(*exprType));
					assert(std::get<Ast::PrimitiveType>(*resultType) == Ast::PrimitiveType::Boolean);

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
					PushResultId(operand); //< No-op
					break;
			}

			throw std::runtime_error("unexpected unary operation");
		}();

		PushResultId(resultId);
	}

	void SpirvAstVisitor::Visit(Ast::VariableValueExpression& node)
	{
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

		m_currentBlock->Append(SpirvOp::OpBranch, headerBlock->GetLabelId());
		m_currentBlock = headerBlock.get();

		std::uint32_t expressionId = EvaluateExpression(*node.condition);

		SpirvLoopControl loopControl;
		if (node.unroll.HasValue())
		{
			switch (node.unroll.GetResultingValue())
			{
				case Ast::LoopUnroll::Always:
					// it shouldn't be possible to have this attribute as the loop gets unrolled in the sanitizer
					throw std::runtime_error("unexpected unroll attribute");

				case Ast::LoopUnroll::Hint:
					loopControl = SpirvLoopControl::Unroll;
					break;

				case Ast::LoopUnroll::Never:
					loopControl = SpirvLoopControl::DontUnroll;
					break;
			}
		}
		else
			loopControl = SpirvLoopControl::None;

		m_currentBlock->Append(SpirvOp::OpLoopMerge, mergeBlock->GetLabelId(), bodyBlock->GetLabelId(), loopControl);
		m_currentBlock->Append(SpirvOp::OpBranchConditional, expressionId, bodyBlock->GetLabelId(), mergeBlock->GetLabelId());

		std::uint32_t headerLabelId = headerBlock->GetLabelId();

		m_currentBlock = bodyBlock.get();
		m_functionBlocks.emplace_back(std::move(headerBlock));
		m_functionBlocks.emplace_back(std::move(bodyBlock));

		node.body->Visit(*this);

		// Jump back to header block to test condition
		m_currentBlock->Append(SpirvOp::OpBranch, headerLabelId);

		m_functionBlocks.emplace_back(std::move(mergeBlock));
		m_currentBlock = m_functionBlocks.back().get();
	}

	void SpirvAstVisitor::HandleStatementList(const std::vector<Ast::StatementPtr>& statements)
	{
		for (auto& statement : statements)
		{
			// Handle termination statements
			switch (statement->GetType())
			{
				case Ast::NodeType::DiscardStatement:
				case Ast::NodeType::ReturnStatement:
					statement->Visit(*this);
					return; //< stop processing statements after this one

				default:
					statement->Visit(*this);
					break;
			}
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
}
