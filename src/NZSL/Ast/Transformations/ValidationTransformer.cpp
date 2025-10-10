// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <fmt/format.h>
#include <tsl/ordered_map.h>
#include <unordered_map>

namespace nzsl::Ast
{
	struct ValidationTransformer::FunctionData
	{
		std::unordered_multimap<BuiltinEntry, SourceLocation> usedBuiltins;
		std::unordered_multimap<ShaderStageType, SourceLocation> requiredShaderStage;
		Nz::HybridBitset<Nz::UInt32, 32> calledFunctions;
		Nz::HybridBitset<Nz::UInt32, 32> calledByFunctions;
		ShaderStageTypeFlags calledByStages;
		const DeclareFunctionStatement* node;
		std::optional<ShaderStageType> entryStage;
		std::size_t funcIndex;
	};

	struct ValidationTransformer::States
	{
		struct Scope
		{
			Nz::HybridVector<std::size_t, 8> aliases;
			Nz::HybridVector<std::size_t, 8> consts;
			Nz::HybridVector<std::size_t, 4> externals;
			Nz::HybridVector<std::size_t, 8> functions;
			Nz::HybridVector<std::size_t, 8> structs;
			Nz::HybridVector<std::size_t, 16> variables;
		};

		Nz::HybridBitset<Nz::UInt64, 256> registeredAliases;
		Nz::HybridBitset<Nz::UInt64, 256> registeredConsts;
		Nz::HybridBitset<Nz::UInt64, 256> registeredExternals;
		Nz::HybridBitset<Nz::UInt64, 256> registeredFuncs;
		Nz::HybridBitset<Nz::UInt64, 256> registeredStructs;
		Nz::HybridBitset<Nz::UInt64, 256> registeredVariables;

		std::unordered_map<OptionHash, std::string> declaredOptions;
		std::unordered_map<std::size_t, const StructDescription*> structs;
		std::vector<DeclareFunctionStatement*> pendingFunctions;
		std::vector<Scope> scopes;
		tsl::ordered_map<std::size_t, FunctionData> functions;
		SourceLocation pushConstantLocation;
		FunctionData* currentFunction = nullptr;
		unsigned int loopCounter = 0;
	};

	bool ValidationTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		States states;
		m_states = &states;

		PushScope();
		NAZARA_DEFER({ PopScope(); });

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	bool ValidationTransformer::TransformExpression(ExpressionPtr& expression, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		States states;
		m_states = &states;

		return Transformer::TransformExpression(expression, context, error);
	}

	bool ValidationTransformer::TransformStatement(StatementPtr& statement, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		States states;
		m_states = &states;

		return Transformer::TransformStatement(statement, context, error);
	}

	void ValidationTransformer::CheckAliasIndex(std::optional<std::size_t> aliasIndex, const SourceLocation& sourceLocation) const
	{
		assert(m_options->checkIndices);

		if (aliasIndex)
		{
			if (!m_states->registeredAliases.UnboundedTest(*aliasIndex))
				throw AstInvalidIndexError{ sourceLocation, "alias", *aliasIndex };
		}
		else
		{
			if (!m_context->partialCompilation)
				throw AstExpectedIndexError{ sourceLocation, "alias" };
		}
	}

	void ValidationTransformer::CheckConstIndex(std::optional<std::size_t> constIndex, const SourceLocation& sourceLocation) const
	{
		assert(m_options->checkIndices);

		if (constIndex)
		{
			if (!m_states->registeredConsts.UnboundedTest(*constIndex))
				throw AstInvalidIndexError{ sourceLocation, "constant", *constIndex };
		}
		else
		{
			if (!m_context->partialCompilation)
				throw AstExpectedIndexError{ sourceLocation, "constant" };
		}
	}

	void ValidationTransformer::CheckExternalIndex(std::optional<std::size_t> externalIndex, const SourceLocation& sourceLocation) const
	{
		assert(m_options->checkIndices);

		if (externalIndex)
		{
			if (!m_states->registeredExternals.UnboundedTest(*externalIndex))
				throw AstInvalidIndexError{ sourceLocation, "external", *externalIndex };
		}
		else
		{
			if (!m_context->partialCompilation)
				throw AstExpectedIndexError{ sourceLocation, "external" };
		}
	}

	void ValidationTransformer::CheckFuncIndex(std::optional<std::size_t> funcIndex, const SourceLocation& sourceLocation) const
	{
		assert(m_options->checkIndices);

		if (funcIndex)
		{
			if (!m_states->registeredFuncs.UnboundedTest(*funcIndex))
				throw AstInvalidIndexError{ sourceLocation, "function", *funcIndex };
		}
		else
		{
			if (!m_context->partialCompilation)
				throw AstExpectedIndexError{ sourceLocation, "function" };
		}
	}

	void ValidationTransformer::CheckStructIndex(std::optional<std::size_t> structIndex, const SourceLocation& sourceLocation) const
	{
		assert(m_options->checkIndices);

		if (structIndex)
		{
			if (!m_states->registeredStructs.UnboundedTest(*structIndex))
				throw AstInvalidIndexError{ sourceLocation, "struct", *structIndex };
		}
		else
		{
			if (!m_context->partialCompilation)
				throw AstExpectedIndexError{ sourceLocation, "struct" };
		}
	}

	void ValidationTransformer::CheckVariableIndex(std::optional<std::size_t> variableIndex, const SourceLocation& sourceLocation) const
	{
		assert(m_options->checkIndices);

		if (variableIndex)
		{
			if (!m_states->registeredVariables.UnboundedTest(*variableIndex))
				throw AstInvalidIndexError{ sourceLocation, "variable", *variableIndex };
		}
		else
		{
			if (!m_context->partialCompilation)
				throw AstExpectedIndexError{ sourceLocation, "variable" };
		}
	}

	const ExpressionType* ValidationTransformer::GetExpressionType(const Expression& expr) const
	{
		const ExpressionType* exprType = Ast::GetExpressionType(expr);
		if (!exprType && !m_context->partialCompilation)
			throw AstInternalError{ expr.sourceLocation, "unexpected missing expression type" };

		return exprType;
	}

	void ValidationTransformer::PopScope()
	{
		auto& scope = m_states->scopes.back();
		for (std::size_t id : scope.aliases)
			m_states->registeredAliases.Reset(id);

		for (std::size_t id : scope.consts)
			m_states->registeredConsts.Reset(id);

		for (std::size_t id : scope.externals)
			m_states->registeredExternals.Reset(id);

		for (std::size_t id : scope.functions)
			m_states->registeredFuncs.Reset(id);

		for (std::size_t id : scope.structs)
			m_states->registeredStructs.Reset(id);

		for (std::size_t id : scope.variables)
			m_states->registeredVariables.Reset(id);

		m_states->scopes.pop_back();
	}
	
	void ValidationTransformer::PropagateFunctionStages(FunctionData& calledFuncData, Nz::HybridBitset<Nz::UInt32, 32>& seen)
	{
		seen.UnboundedSet(calledFuncData.funcIndex);
		for (std::size_t calledByFuncIndex : calledFuncData.calledByFunctions.IterBits())
		{
			auto callingFuncIt = m_states->functions.find(calledByFuncIndex);
			assert(callingFuncIt != m_states->functions.end());

			auto& callingFuncData = callingFuncIt.value();
			if (!seen.UnboundedTest(calledByFuncIndex))
				PropagateFunctionStages(callingFuncData, seen);

			calledFuncData.calledByStages |= callingFuncData.calledByStages;
		}
	}

	void ValidationTransformer::PushScope()
	{
		m_states->scopes.emplace_back();
	}

	void ValidationTransformer::RegisterAlias(std::size_t aliasIndex, const SourceLocation& sourceLocation)
	{
		assert(m_options->checkIndices);

		if (m_states->registeredAliases.UnboundedTest(aliasIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, aliasIndex };

		m_states->registeredAliases.UnboundedSet(aliasIndex);
		auto& scope = m_states->scopes.back();
		scope.aliases.push_back(aliasIndex);
	}

	void ValidationTransformer::RegisterConst(std::size_t constIndex, const SourceLocation& sourceLocation)
	{
		assert(m_options->checkIndices);

		if (m_states->registeredConsts.UnboundedTest(constIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, constIndex };

		m_states->registeredConsts.UnboundedSet(constIndex);
		auto& scope = m_states->scopes.back();
		scope.consts.push_back(constIndex);
	}

	void ValidationTransformer::RegisterExternal(std::size_t externalIndex, const SourceLocation& sourceLocation)
	{
		assert(m_options->checkIndices);

		if (m_states->registeredExternals.UnboundedTest(externalIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, externalIndex };

		m_states->registeredExternals.UnboundedSet(externalIndex);
		auto& scope = m_states->scopes.back();
		scope.externals.push_back(externalIndex);
	}

	void ValidationTransformer::RegisterFunc(std::size_t funcIndex, const SourceLocation& sourceLocation)
	{
		assert(m_options->checkIndices);

		if (m_states->registeredFuncs.UnboundedTest(funcIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, funcIndex };

		m_states->registeredFuncs.UnboundedSet(funcIndex);
		auto& scope = m_states->scopes.back();
		scope.functions.push_back(funcIndex);
	}

	void ValidationTransformer::RegisterStruct(std::size_t structIndex, const SourceLocation& sourceLocation)
	{
		assert(m_options->checkIndices);

		if (m_states->registeredStructs.UnboundedTest(structIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, structIndex };

		m_states->registeredStructs.UnboundedSet(structIndex);
		auto& scope = m_states->scopes.back();
		scope.structs.push_back(structIndex);
	}

	void ValidationTransformer::RegisterVariable(std::size_t variableIndex, const SourceLocation& sourceLocation)
	{
		assert(m_options->checkIndices);

		if (m_states->registeredVariables.UnboundedTest(variableIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, variableIndex };

		m_states->registeredVariables.UnboundedSet(variableIndex);
		auto& scope = m_states->scopes.back();
		scope.variables.push_back(variableIndex);
	}

	void ValidationTransformer::ResolveFunctions()
	{
		// Once every function is known, we can evaluate function content
		for (DeclareFunctionStatement* pendingFunc : m_states->pendingFunctions)
		{
			PushScope();

			if (m_options->checkIndices)
			{
				for (auto& parameter : pendingFunc->parameters)
				{
					if (parameter.varIndex)
						RegisterVariable(*parameter.varIndex, parameter.sourceLocation);
				}
			}

			assert(pendingFunc->funcIndex);
			std::size_t funcIndex = *pendingFunc->funcIndex;

			auto it = m_states->functions.find(funcIndex);
			assert(it != m_states->functions.end());

			auto& funcData = it.value();
			if (funcData.node->entryStage.IsResultingValue())
				funcData.calledByStages = funcData.node->entryStage.GetResultingValue();

			m_states->currentFunction = &funcData;

			HandleStatementList<false>(pendingFunc->statements, [&](StatementPtr& statement)
			{
				HandleStatement(statement);
			});

			m_states->currentFunction = nullptr;

			for (std::size_t calledFunctionIndex : funcData.calledFunctions.IterBits())
			{
				auto calledFuncIt = m_states->functions.find(calledFunctionIndex);
				assert(calledFuncIt != m_states->functions.end());

				auto& targetFunc = calledFuncIt.value();
				targetFunc.calledByFunctions.UnboundedSet(*pendingFunc->funcIndex);
			}

			PopScope();
		}

		Nz::HybridBitset<Nz::UInt32, 32> seen;
		for (auto it = m_states->functions.begin(); it != m_states->functions.end(); ++it)
		{
			if (!seen.UnboundedTest(it.key()))
				PropagateFunctionStages(it.value(), seen);
		}

		for (auto it = m_states->functions.begin(); it != m_states->functions.end(); ++it)
		{
			auto& funcData = it.value();

			for (std::size_t flagIndex = 0; flagIndex <= static_cast<std::size_t>(ShaderStageType::Max); ++flagIndex)
			{
				ShaderStageType stageType = static_cast<ShaderStageType>(flagIndex);
				if (!funcData.calledByStages.Test(stageType))
					continue;

				// Check builtin usage
				for (auto&& [builtin, sourceLocation] : funcData.usedBuiltins)
				{
					auto builtinIt = LangData::s_builtinData.find(builtin);
					if (builtinIt == LangData::s_builtinData.end())
						throw AstInternalError{ sourceLocation, "missing builtin data" };

					const LangData::BuiltinData& builtinData = builtinIt->second;
					if (!builtinData.compatibleStages.Test(stageType))
						throw CompilerBuiltinUnsupportedStageError{ sourceLocation, builtin, stageType };
				}

				// Check other stage dependencies (such as discard)
				for (auto&& [requiredStageType, sourceLocation] : funcData.requiredShaderStage)
				{
					if (requiredStageType != stageType)
						throw CompilerInvalidStageDependencyError{ sourceLocation, requiredStageType, stageType };
				}
			}
		}
	}

	std::size_t ValidationTransformer::ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		std::size_t structIndex = Ast::ResolveStructIndex(exprType);
		if (structIndex == std::numeric_limits<std::size_t>::max())
			throw CompilerStructExpectedError{ sourceLocation, Ast::ToString(exprType) };

		return structIndex;
	}

	std::string ValidationTransformer::ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const
	{
		return Ast::ToString(exprType, BuildStringifier(sourceLocation));
	}

	auto ValidationTransformer::Transform(AccessFieldExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		const Ast::ExpressionType* exprType = GetExpressionType(*node.expr);
		if (!exprType)
			return DontVisitChildren{};

		std::size_t structIndex = ResolveStructIndex(*exprType, node.sourceLocation);
		const StructDescription* structDesc = Nz::Retrieve(m_states->structs, structIndex);

		const StructDescription::StructMember* structMember = nullptr;

		std::uint32_t remainingIndices = node.fieldIndex;
		for (const auto& member : structDesc->members)
		{
			if (member.cond.HasValue() && !member.cond.GetResultingValue())
				continue;

			if (remainingIndices == 0)
			{
				structMember = &member;
				break;
			}

			remainingIndices--;
		}

		if (!structMember)
			throw AstIndexOutOfBoundsError{ node.sourceLocation, "struct field", static_cast<std::int32_t>(node.fieldIndex) };

		if (structMember->builtin.HasValue() && structMember->builtin.IsResultingValue())
		{
			BuiltinEntry builtin = structMember->builtin.GetResultingValue();
			m_states->currentFunction->usedBuiltins.emplace(builtin, structMember->sourceLocation);
		}

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AccessIdentifierExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AccessIndexExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AssignExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(node.left, node.sourceLocation));
		if (!leftExprType)
			return DontVisitChildren{};

		if (GetExpressionCategory(*node.left) != ExpressionCategory::Variable)
			throw CompilerAssignTemporaryError{ node.sourceLocation };

		const ExpressionType* rightExprType = GetExpressionType(MandatoryExpr(node.right, node.sourceLocation));
		if (!rightExprType)
			return DontVisitChildren{};

		std::optional<BinaryType> binaryType;
		switch (node.op)
		{
			case AssignType::Simple:
			{
				if (!ValidateMatchingTypes(*leftExprType, UnwrapExternalType(*rightExprType)))
					throw CompilerUnmatchingTypesError{ node.sourceLocation, ToString(*leftExprType, node.sourceLocation), ToString(*rightExprType, node.sourceLocation) };

				break;
			}

			case AssignType::CompoundAdd:        binaryType = BinaryType::Add; break;
			case AssignType::CompoundDivide:     binaryType = BinaryType::Divide; break;
			case AssignType::CompoundModulo:     binaryType = BinaryType::Modulo; break;
			case AssignType::CompoundMultiply:   binaryType = BinaryType::Multiply; break;
			case AssignType::CompoundLogicalAnd: binaryType = BinaryType::LogicalAnd; break;
			case AssignType::CompoundLogicalOr:  binaryType = BinaryType::LogicalOr; break;
			case AssignType::CompoundSubtract:   binaryType = BinaryType::Subtract; break;
		}

		if (binaryType)
		{
			ExpressionType expressionType = ValidateBinaryOp(*binaryType, *leftExprType, UnwrapExternalType(*rightExprType), node.sourceLocation);
			if (!ValidateMatchingTypes(UnwrapExternalType(*leftExprType), expressionType))
				throw CompilerUnmatchingTypesError{ node.sourceLocation, ToString(*leftExprType, node.sourceLocation), ToString(expressionType, node.sourceLocation) };
		}

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(BinaryExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		// Validation already done by IdentifierTypeTransformer

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(CallFunctionExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		const ExpressionType* targetFuncType = GetExpressionType(MandatoryExpr(node.targetFunction, node.sourceLocation));
		if (!targetFuncType)
			return DontVisitChildren{};

		const ExpressionType& resolvedTargetType = ResolveAlias(*targetFuncType);

		if (!std::holds_alternative<Ast::FunctionType>(resolvedTargetType))
		{
			if (!m_context->partialCompilation)
				throw AstInternalError{ node.sourceLocation, "CallFunction target function is not a function, is the shader resolved?" };

			return DontVisitChildren{};
		}

		std::size_t functionIndex = std::get<Ast::FunctionType>(resolvedTargetType).funcIndex;
		if (m_options->checkIndices)
			CheckFuncIndex({ functionIndex }, node.sourceLocation);

		if (m_states->currentFunction)
			m_states->currentFunction->calledFunctions.UnboundedSet(functionIndex);

		auto& funcData = Nz::Retrieve(m_states->functions, functionIndex);
		const DeclareFunctionStatement* referenceDeclaration = funcData.node;

		if (referenceDeclaration->entryStage.HasValue())
			throw CompilerFunctionCallUnexpectedEntryFunctionError{ node.sourceLocation, referenceDeclaration->name };

		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			auto& expressionPtr = node.parameters[i].expr;

			const ExpressionType* parameterType = GetExpressionType(*expressionPtr);
			if (!parameterType)
				continue;

			if (node.parameters[i].semantic != FunctionParameterSemantic::In)
			{
				const Ast::ExpressionCategory category = Ast::GetExpressionCategory(*expressionPtr);
				if (category != Ast::ExpressionCategory::Variable)
					throw CompilerFunctionCallSemanticRequiresVariableError{ expressionPtr->sourceLocation };
			}

			const ExpressionType& expectedType = referenceDeclaration->parameters[i].type.GetResultingValue();

			if (!ValidateMatchingTypes(expectedType, *parameterType))
				throw CompilerFunctionCallUnmatchingParameterTypeError{ node.parameters[i].expr->sourceLocation, referenceDeclaration->name, Nz::SafeCast<std::uint32_t>(i), ToString(referenceDeclaration->parameters[i].type.GetResultingValue(), referenceDeclaration->parameters[i].sourceLocation), ToString(*parameterType, node.parameters[i].expr->sourceLocation) };

			if (node.parameters[i].semantic != referenceDeclaration->parameters[i].semantic)
				throw CompilerFunctionCallUnmatchingParameterSemanticTypeError{ node.parameters[i].expr->sourceLocation, referenceDeclaration->name, Nz::SafeCast<std::uint32_t>(i), Ast::ToString(referenceDeclaration->parameters[i].semantic), Ast::ToString(node.parameters[i].semantic) };
		}

		if (node.parameters.size() != referenceDeclaration->parameters.size())
			throw CompilerFunctionCallUnmatchingParameterCountError{ node.sourceLocation, referenceDeclaration->name, Nz::SafeCast<std::uint32_t>(referenceDeclaration->parameters.size()), Nz::SafeCast<std::uint32_t>(node.parameters.size()) };

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(CallMethodExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(CastExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (!node.targetType.HasValue())
			throw AstMissingTypeError{ node.sourceLocation };

		if (!node.targetType.IsResultingValue())
		{
			if (!m_context->partialCompilation)
				throw AstMissingTypeError{ node.sourceLocation };
		}

		ExpressionType targetType = ResolveAlias(node.targetType.GetResultingValue());

		std::size_t expressionCount = node.expressions.size();

		auto areTypeCompatibles = [&](PrimitiveType from, PrimitiveType to)
		{
			switch (to)
			{
				case PrimitiveType::Boolean:
				case PrimitiveType::String:
				case PrimitiveType::FloatLiteral:
				case PrimitiveType::IntLiteral:
					return false;

				case PrimitiveType::Float32:
				{
					switch (from)
					{
						case PrimitiveType::Boolean:
						case PrimitiveType::String:
							return false;

						case PrimitiveType::Float32:
						case PrimitiveType::Float64:
						case PrimitiveType::Int32:
						case PrimitiveType::UInt32:
						case PrimitiveType::FloatLiteral:
						case PrimitiveType::IntLiteral:
							return true;
					}

					break;
				}

				case PrimitiveType::Float64:
				{
					switch (from)
					{
						case PrimitiveType::Boolean:
						case PrimitiveType::String:
							return false;

						case PrimitiveType::Float32:
						case PrimitiveType::Float64:
						case PrimitiveType::Int32:
						case PrimitiveType::UInt32:
						case PrimitiveType::FloatLiteral:
						case PrimitiveType::IntLiteral:
							return true;
					}

					break;
				}

				case PrimitiveType::Int32:
				{
					switch (from)
					{
						case PrimitiveType::Boolean:
						case PrimitiveType::String:
							return false;

						case PrimitiveType::Float32:
						case PrimitiveType::Float64:
						case PrimitiveType::Int32:
						case PrimitiveType::UInt32:
						case PrimitiveType::FloatLiteral:
						case PrimitiveType::IntLiteral:
							return true;
					}

					break;
				}

				case PrimitiveType::UInt32:
				{
					switch (from)
					{
						case PrimitiveType::Boolean:
						case PrimitiveType::String:
							return false;

						case PrimitiveType::Float32:
						case PrimitiveType::Float64:
						case PrimitiveType::Int32:
						case PrimitiveType::UInt32:
						case PrimitiveType::FloatLiteral:
						case PrimitiveType::IntLiteral:
							return true;
					}

					break;
				}
			}

			throw AstInternalError{ node.sourceLocation, "unexpected cast from " + Ast::ToString(from) + " to " + Ast::ToString(to) };
		};

		if (IsMatrixType(targetType))
		{
			const MatrixType& targetMatrixType = std::get<MatrixType>(targetType);
			if (expressionCount == 0)
				throw CompilerCastComponentMismatchError{ node.sourceLocation, 0, Nz::SafeCast<std::uint32_t>(targetMatrixType.columnCount * targetMatrixType.rowCount) };

			auto& firstExprPtr = MandatoryExpr(node.expressions.front(), node.sourceLocation);

			const ExpressionType* firstExprType = GetExpressionType(firstExprPtr);
			if (!firstExprType)
				return DontVisitChildren{};

			const ExpressionType& resolvedFirstExprType = ResolveAlias(*firstExprType);
			if (IsMatrixType(resolvedFirstExprType))
			{
				if (expressionCount != 1)
					throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(expressionCount), 1 };

				// Matrix to matrix cast: always valid
			}
			else if (IsVectorType(resolvedFirstExprType))
			{
				// Matrix builder (from vectors)

				assert(targetMatrixType.columnCount <= 4);
				if (expressionCount != targetMatrixType.columnCount)
					throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(expressionCount), Nz::SafeCast<std::uint32_t>(targetMatrixType.columnCount) };

				for (std::size_t i = 0; i < targetMatrixType.columnCount; ++i)
				{
					auto& exprPtr = MandatoryExpr(node.expressions[i], node.sourceLocation);

					const ExpressionType* exprType = GetExpressionType(exprPtr);
					if (!exprType)
						return DontVisitChildren{};

					const ExpressionType& resolvedExprType = ResolveAlias(*exprType);
					if (!IsVectorType(resolvedExprType))
						throw CompilerCastMatrixExpectedVectorOrScalarError{ node.sourceLocation, ToString(resolvedExprType, node.expressions[i]->sourceLocation) };

					const VectorType& vecType = std::get<VectorType>(resolvedExprType);
					if (vecType.componentCount != targetMatrixType.rowCount)
						throw CompilerCastMatrixVectorComponentMismatchError{ node.expressions[i]->sourceLocation, Nz::SafeCast<std::uint32_t>(vecType.componentCount), Nz::SafeCast<std::uint32_t>(targetMatrixType.rowCount) };

					if (!ValidateMatchingTypes(targetMatrixType.type, vecType.type))
						throw CompilerCastIncompatibleBaseTypesError{ node.expressions[i]->sourceLocation, ToString(targetMatrixType.type, node.sourceLocation), ToString(vecType.type, node.sourceLocation) };
				}
			}
			else if (IsPrimitiveType(resolvedFirstExprType))
			{
				// Matrix builder (from scalars)
				std::size_t requiredComponentCount = targetMatrixType.columnCount * targetMatrixType.rowCount;
				if (expressionCount != requiredComponentCount && expressionCount != 1) //< special case where matrices can be built using one value
					throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(expressionCount), Nz::SafeCast<std::uint32_t>(requiredComponentCount) };

				for (std::size_t i = 0; i < requiredComponentCount; ++i)
				{
					std::size_t exprIndex = (expressionCount > 1) ? i : 0;

					auto& exprPtr = MandatoryExpr(node.expressions[exprIndex], node.sourceLocation);

					const ExpressionType* exprType = GetExpressionType(exprPtr);
					if (!exprType)
						return DontVisitChildren{};

					const ExpressionType& resolvedExprType = ResolveAlias(*exprType);
					if (!IsPrimitiveType(resolvedExprType))
						throw CompilerCastMatrixExpectedVectorOrScalarError{ node.sourceLocation, ToString(resolvedExprType, node.expressions[i]->sourceLocation) };

					PrimitiveType baseType = std::get<PrimitiveType>(resolvedExprType);
					if (!ValidateMatchingTypes(targetMatrixType.type, baseType))
						throw CompilerCastIncompatibleBaseTypesError{ node.expressions[exprIndex]->sourceLocation, ToString(targetMatrixType.type, node.sourceLocation), ToString(baseType, node.sourceLocation) };
				}
			}
			else
				throw CompilerCastMatrixExpectedVectorOrScalarError{ node.sourceLocation, ToString(resolvedFirstExprType, firstExprPtr.sourceLocation) };
		}
		else if (IsPrimitiveType(targetType))
		{
			// Cast between primitive types
			if (expressionCount != 1)
				throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(expressionCount), 1 };

			const ExpressionType* fromType = GetExpressionType(*node.expressions[0]);
			if (!fromType)
				return DontVisitChildren{};

			const ExpressionType& resolvedFromType = ResolveAlias(*fromType);
			if (!IsPrimitiveType(resolvedFromType))
				throw CompilerCastIncompatibleTypesError{ node.expressions[0]->sourceLocation, ToString(targetType, node.sourceLocation), ToString(resolvedFromType, node.sourceLocation) };

			PrimitiveType fromPrimitiveType = std::get<PrimitiveType>(resolvedFromType);
			PrimitiveType targetPrimitiveType = std::get<PrimitiveType>(targetType);

			if (!areTypeCompatibles(fromPrimitiveType, targetPrimitiveType))
				throw CompilerCastIncompatibleTypesError{ node.expressions[0]->sourceLocation, ToString(targetType, node.sourceLocation), ToString(resolvedFromType, node.sourceLocation) };
		}
		else if (IsVectorType(targetType))
		{
			PrimitiveType targetBaseType = std::get<VectorType>(targetType).type;

			auto GetComponentCount = [](const ExpressionType& exprType) -> std::size_t
			{
				if (IsVectorType(exprType))
					return std::get<VectorType>(exprType).componentCount;
				else
				{
					assert(IsPrimitiveType(exprType));
					return 1;
				}
			};

			std::size_t componentCount = 0;
			std::size_t requiredComponents = GetComponentCount(targetType);

			for (auto& exprPtr : node.expressions)
			{
				const ExpressionType* exprType = GetExpressionType(*exprPtr);
				if (!exprType)
					return DontVisitChildren{};

				const ExpressionType& resolvedExprType = ResolveAlias(*exprType);
				if (IsPrimitiveType(resolvedExprType))
				{
					PrimitiveType primitiveType = std::get<PrimitiveType>(resolvedExprType);
					if (!ValidateMatchingTypes(targetBaseType, primitiveType))
						throw CompilerCastIncompatibleBaseTypesError{ exprPtr->sourceLocation, ToString(targetBaseType, node.sourceLocation), ToString(primitiveType, node.sourceLocation) };
				}
				else if (IsVectorType(resolvedExprType))
				{
					const VectorType& vecType = std::get<VectorType>(resolvedExprType);
					PrimitiveType primitiveType = vecType.type;

					// Conversion between base types (vec2[i32] => vec2[u32])
					if (componentCount == 0 && requiredComponents == vecType.componentCount)
					{
						if (!areTypeCompatibles(primitiveType, targetBaseType))
							throw CompilerCastIncompatibleTypesError{ exprPtr->sourceLocation, ToString(targetType, node.sourceLocation), ToString(resolvedExprType, node.sourceLocation) };
					}
					else if (!ValidateMatchingTypes(targetBaseType, primitiveType))
						throw CompilerCastIncompatibleBaseTypesError{ exprPtr->sourceLocation, ToString(targetBaseType, node.sourceLocation), ToString(primitiveType, node.sourceLocation) };
				}
				else
					throw CompilerCastIncompatibleTypesError{ exprPtr->sourceLocation, ToString(targetType, node.sourceLocation), ToString(resolvedExprType, exprPtr->sourceLocation) };

				componentCount += GetComponentCount(resolvedExprType);
			}

			if (componentCount != requiredComponents)
				throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(componentCount), Nz::SafeCast<std::uint32_t>(requiredComponents) };
		}
		else if (IsArrayType(targetType))
		{
			ArrayType& targetArrayType = std::get<ArrayType>(targetType);
			if (targetArrayType.length > 0)
			{
				if (targetArrayType.length != node.expressions.size())
					throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(node.expressions.size()), targetArrayType.length };
			}
			else
				targetArrayType.length = Nz::SafeCast<std::uint32_t>(node.expressions.size());

			const ExpressionType& innerType = targetArrayType.InnerType();
			for (std::size_t i = 0; i < node.expressions.size(); ++i)
			{
				const auto& exprPtr = node.expressions[i];
				assert(exprPtr);

				const ExpressionType* exprType = GetExpressionType(*exprPtr);
				if (!exprType)
					return DontVisitChildren{};

				if (!ValidateMatchingTypes(innerType, *exprType))
					throw CompilerCastIncompatibleTypesError{ exprPtr->sourceLocation, ToString(innerType, node.sourceLocation), ToString(*exprType, node.sourceLocation) };
			}
		}
		else
			throw CompilerInvalidCastError{ node.sourceLocation, ToString(targetType, node.sourceLocation) };

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConditionalExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConstantArrayValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConstantValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IdentifierExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IdentifierValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
		{
			switch (node.identifierType)
			{
				case IdentifierType::Alias:
					CheckAliasIndex(node.identifierIndex, node.sourceLocation);
					break;

				case IdentifierType::Constant:
					CheckConstIndex(node.identifierIndex, node.sourceLocation);
					break;

				case IdentifierType::ExternalBlock:
					CheckExternalIndex(node.identifierIndex, node.sourceLocation);
					break;

				case IdentifierType::Function:
					CheckFuncIndex(node.identifierIndex, node.sourceLocation);
					break;

				case IdentifierType::Intrinsic:
					break;

				case IdentifierType::Module:
					if (m_context->modules.TryRetrieve(node.identifierIndex, node.sourceLocation) == nullptr)
						throw AstIndexOutOfBoundsError{ node.sourceLocation, "module", static_cast<std::int32_t>(node.identifierIndex) };

					break;

				case IdentifierType::Struct:
					CheckStructIndex(node.identifierIndex, node.sourceLocation);
					break;

				case IdentifierType::Type:
					break;

				case IdentifierType::Unresolved:
					break;

				case IdentifierType::Variable:
					CheckVariableIndex(node.identifierIndex, node.sourceLocation);
					break;
			}
		}

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IntrinsicExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		// Parameter validation
		ValidateIntrinsicParameters(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(SwizzleExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(TypeConstantExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (!IsPrimitiveType(node.type))
			throw CompilerTypeConstantUnsupportedTypeError{ node.sourceLocation, ToString(node.type, node.sourceLocation), node.typeConstant };

		PrimitiveType primitiveType = std::get<PrimitiveType>(node.type);

		switch (node.typeConstant)
		{
			case TypeConstant::Max:
			case TypeConstant::Min:
			{
				if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64 && primitiveType != PrimitiveType::Int32 && primitiveType != PrimitiveType::UInt32)
					throw CompilerTypeConstantUnsupportedTypeError{ node.sourceLocation, ToString(node.type, node.sourceLocation), node.typeConstant };

				break;
			}

			case TypeConstant::Epsilon:
			case TypeConstant::Infinity:
			case TypeConstant::MinPositive:
			case TypeConstant::NaN:
			{
				if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64)
					throw CompilerTypeConstantUnsupportedTypeError{ node.sourceLocation, ToString(node.type, node.sourceLocation), node.typeConstant };

				break;
			}
		}

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(UnaryExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		const ExpressionType* exprType = GetExpressionType(MandatoryExpr(node.expression, node.sourceLocation));
		if (!exprType)
			return DontVisitChildren{};

		ValidateUnaryOp(node.op, *exprType, node.sourceLocation, BuildStringifier(node.sourceLocation));
		return DontVisitChildren{};
	}


	auto ValidationTransformer::Transform(BranchStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(BreakStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (m_states->loopCounter == 0)
			throw CompilerLoopControlOutsideOfLoopError{ node.sourceLocation, "break" };

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConditionalStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ContinueStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (m_states->loopCounter == 0)
			throw CompilerLoopControlOutsideOfLoopError{ node.sourceLocation, "continue" };

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareAliasStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.name.empty())
			throw AstEmptyIdentifierError{ node.sourceLocation };

		const ExpressionType* exprType = GetExpressionType(*node.expression);
		if (!exprType)
			return DontVisitChildren{};

		const ExpressionType& resolvedType = ResolveAlias(*exprType);
		if (IsStructType(resolvedType))
		{
			std::size_t structIndex = ResolveStructIndex(resolvedType, node.expression->sourceLocation);
			if (m_options->checkIndices)
				CheckStructIndex(structIndex, node.expression->sourceLocation);
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;
			if (m_options->checkIndices)
				CheckFuncIndex(funcIndex, node.expression->sourceLocation);
		}
		else if (IsAliasType(resolvedType))
		{
			const AliasType& alias = std::get<AliasType>(resolvedType);
			if (m_options->checkIndices)
				CheckAliasIndex(alias.aliasIndex, node.expression->sourceLocation);
		}
		else if (IsModuleType(resolvedType))
		{
			const ModuleType& module = std::get<ModuleType>(resolvedType);

			if (m_options->checkIndices)
			{
				if (m_context->modules.TryRetrieve(module.moduleIndex, node.sourceLocation) == nullptr)
					throw AstIndexOutOfBoundsError{ node.sourceLocation, "module", static_cast<std::int32_t>(module.moduleIndex) };
			}
		}
		else
			throw CompilerAliasUnexpectedTypeError{ node.sourceLocation, Ast::ToString(*exprType) };

		if (node.aliasIndex && m_options->checkIndices)
			RegisterAlias(*node.aliasIndex, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareConstStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.constIndex && m_options->checkIndices)
			RegisterConst(*node.constIndex, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareExternalStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.externalIndex && m_options->checkIndices)
			RegisterExternal(*node.externalIndex, node.sourceLocation);

		for (auto& extVar : node.externalVars)
		{
			if (extVar.varIndex && m_options->checkIndices)
				RegisterVariable(*extVar.varIndex, extVar.sourceLocation);

			if (!extVar.type.IsResultingValue())
			{
				if (!m_context->partialCompilation)
					throw AstMissingTypeError{ extVar.sourceLocation };

				continue;
			}

			const ExpressionType& targetType = ResolveAlias(extVar.type.GetResultingValue());

			if (IsNoType(targetType))
				throw CompilerExtTypeNotAllowedError{ extVar.sourceLocation, extVar.name, ToString(extVar.type.GetResultingValue(), extVar.sourceLocation) };

			if (IsPushConstantType(targetType))
			{
				if (m_states->pushConstantLocation.IsValid())
					throw CompilerMultiplePushConstantError{ extVar.sourceLocation };

				m_states->pushConstantLocation = extVar.sourceLocation;

				if (extVar.bindingSet.HasValue())
					throw CompilerUnexpectedAttributeOnPushConstantError{ extVar.sourceLocation, Ast::AttributeType::Set };
				else if (extVar.bindingIndex.HasValue())
					throw CompilerUnexpectedAttributeOnPushConstantError{ extVar.sourceLocation, Ast::AttributeType::Binding };
			}
		}

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareFunctionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (!node.funcIndex)
			throw AstExpectedIndexError{ node.sourceLocation, "function" };

		if (m_options->checkIndices)
			RegisterFunc(*node.funcIndex, node.sourceLocation);

		FunctionData& functionData = m_states->functions[*node.funcIndex];
		functionData.funcIndex = *node.funcIndex;
		functionData.node = &node;

		if (node.entryStage.IsResultingValue())
			functionData.entryStage = node.entryStage.GetResultingValue();

		m_states->pendingFunctions.push_back(&node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareOptionStatement&& node) -> StatementTransformation
	{
		if (m_states->currentFunction)
			throw CompilerOptionDeclarationInsideFunctionError{ node.sourceLocation };

		HandleChildren(node);

		if (node.optIndex && m_options->checkIndices)
			RegisterConst(*node.optIndex, node.sourceLocation);

		if (!node.optType.HasValue())
			throw AstMissingExpressionTypeError{ node.sourceLocation };
		else if (node.optType.IsResultingValue())
		{
			const ExpressionType& optionType = node.optType.GetResultingValue();

			if (node.defaultValue)
			{
				const ExpressionType* defaultValueType = GetExpressionType(*node.defaultValue);
				if (defaultValueType)
				{
					if (!ValidateMatchingTypes(optionType, *defaultValueType))
						throw CompilerVarDeclarationTypeUnmatchingError{ node.sourceLocation, ToString(optionType, node.sourceLocation), ToString(*defaultValueType, node.sourceLocation) };
				}
				else if (!m_context->partialCompilation)
					throw AstMissingExpressionTypeError{ node.defaultValue->sourceLocation };
			}
		}
		else if (!m_context->partialCompilation)
			throw AstMissingExpressionTypeError{ node.sourceLocation };

		// Detect hash collisions
		OptionHash optionHash = HashOption(node.optName);

		if (auto it = m_states->declaredOptions.find(optionHash); it != m_states->declaredOptions.end())
		{
			if (it->second != node.optName)
				throw CompilerOptionHashCollisionError{ node.sourceLocation, node.optName, it->second };
		}
		else
			m_states->declaredOptions.emplace(optionHash, node.optName);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareStructStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.structIndex && m_options->checkIndices)
		{
			RegisterStruct(*node.structIndex, node.sourceLocation);
			m_states->structs[*node.structIndex] = &node.description;
		}

		for (auto& member : node.description.members)
		{
			// Don't validate disabled fields
			if (member.cond.IsResultingValue() && !member.cond.GetResultingValue())
				continue;

			if (member.type.HasValue() && member.type.IsExpression())
			{
				if (!m_context->partialCompilation)
					throw AstUnresolvedExpressionTypeError{ member.sourceLocation };

				continue;
			}

			const ExpressionType& memberType = member.type.GetResultingValue();

			if (member.builtin.IsResultingValue())
			{
				BuiltinEntry builtin = member.builtin.GetResultingValue();
				auto it = LangData::s_builtinData.find(builtin);
				if (it == LangData::s_builtinData.end())
					throw AstInternalError{ member.sourceLocation, "missing builtin data" };

				const LangData::BuiltinData& builtinData = it->second;
				std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if (!std::holds_alternative<T>(memberType) || std::get<T>(memberType) != arg)
						throw CompilerBuiltinUnexpectedTypeError{ member.sourceLocation, builtin, ToString(arg, member.sourceLocation), ToString(memberType, member.sourceLocation) };
				}, builtinData.type);
			}
		}

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareVariableStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.varIndex && m_options->checkIndices)
			RegisterVariable(*node.varIndex, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DiscardStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (!m_states->currentFunction)
			throw CompilerDiscardOutsideOfFunctionError{ node.sourceLocation };

		m_states->currentFunction->requiredShaderStage.emplace(ShaderStageType::Fragment, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ExpressionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ForStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		const ExpressionType* fromExprType = GetExpressionType(*node.fromExpr);
		if (fromExprType)
		{
			const ExpressionType& resolvedFromExprType = ResolveAlias(*fromExprType);
			if (!IsPrimitiveType(resolvedFromExprType))
				throw CompilerForFromTypeExpectIntegerTypeError{ node.fromExpr->sourceLocation, ToString(*fromExprType, node.fromExpr->sourceLocation) };

			PrimitiveType counterType = std::get<PrimitiveType>(resolvedFromExprType);
			if (counterType != PrimitiveType::Int32 && counterType != PrimitiveType::UInt32 && counterType != PrimitiveType::IntLiteral)
				throw CompilerForFromTypeExpectIntegerTypeError{ node.fromExpr->sourceLocation, ToString(*fromExprType, node.fromExpr->sourceLocation) };

			const ExpressionType* toExprType = GetExpressionType(*node.toExpr);
			if (toExprType)
			{
				if (!ValidateMatchingTypes(*fromExprType, *toExprType))
					throw CompilerForToUnmatchingTypeError{ node.toExpr->sourceLocation, ToString(*toExprType, node.toExpr->sourceLocation), ToString(*fromExprType, node.fromExpr->sourceLocation) };
			}

			if (node.stepExpr)
			{
				const ExpressionType* stepExprType = GetExpressionType(*node.stepExpr);
				if (stepExprType)
				{
					if (!ValidateMatchingTypes(*fromExprType, *stepExprType))
						throw CompilerForStepUnmatchingTypeError{ node.stepExpr->sourceLocation, ToString(*stepExprType, node.stepExpr->sourceLocation), ToString(*fromExprType, node.fromExpr->sourceLocation) };
				}
			}
		}

		if (node.varIndex && m_options->checkIndices)
			RegisterVariable(*node.varIndex, node.sourceLocation);

		m_states->loopCounter++;
		NAZARA_DEFER({ m_states->loopCounter--; });
		HandleStatement(node.statement);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ForEachStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.varIndex && m_options->checkIndices)
			RegisterVariable(*node.varIndex, node.sourceLocation);

		m_states->loopCounter++;
		NAZARA_DEFER({ m_states->loopCounter--; });
		HandleStatement(node.statement);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ImportStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(MultiStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(NoOpStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ReturnStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (!m_states->currentFunction)
			throw CompilerReturnOutsideOfFunctionError{ node.sourceLocation };

		auto& function = m_states->currentFunction;
		if (!function->node->returnType.IsResultingValue())
			return DontVisitChildren{};

		const ExpressionType& functionReturnType = function->node->returnType.GetResultingValue();
		bool functionHasNoReturnType = std::holds_alternative<NoType>(functionReturnType);

		if (!node.returnExpr)
		{
			if (!functionHasNoReturnType)
				throw CompilerFunctionReturnWithNoValueError{ node.sourceLocation, ToString(functionReturnType, node.sourceLocation) };

			// If node doesn't have an expression and function doesn't have return type, then we can directly validate
			return DontVisitChildren{};
		}

		if (functionHasNoReturnType)
			throw CompilerFunctionReturnWithAValueError{ node.sourceLocation };

		const ExpressionType* returnTypeOpt = GetExpressionType(MandatoryExpr(node.returnExpr, node.sourceLocation));
		if (returnTypeOpt)
		{
			if (!ValidateMatchingTypes(*returnTypeOpt, functionReturnType))
				throw CompilerFunctionReturnUnmatchingTypesError{ node.sourceLocation, ToString(*returnTypeOpt, node.sourceLocation), ToString(functionReturnType, node.sourceLocation) };
		}
		else
		{
			if (!m_context->partialCompilation)
				throw AstMissingExpressionTypeError{ node.returnExpr->sourceLocation };
		}

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ScopedStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		MandatoryStatement(node.statement, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(WhileStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		const ExpressionType* conditionType = GetExpressionType(MandatoryExpr(node.condition, node.sourceLocation));
		MandatoryStatement(node.body, node.sourceLocation);

		if (!conditionType)
			return DontVisitChildren{};

		if (ResolveAlias(*conditionType) != ExpressionType{ PrimitiveType::Boolean })
			throw CompilerConditionExpectedBoolError{ node.condition->sourceLocation, ToString(*conditionType, node.condition->sourceLocation) };

		m_states->loopCounter++;
		NAZARA_DEFER({ m_states->loopCounter--; });
		HandleStatement(node.body);

		return DontVisitChildren{};
	}

	void ValidationTransformer::Transform(ExpressionType& expressionType, const SourceLocation& sourceLocation)
	{
		Transformer::Transform(expressionType, sourceLocation);

		if (!m_options->allowUntyped && IsLiteralType(expressionType))
			throw AstUnresolvedLiteralError{ sourceLocation };
	}

	bool ValidationTransformer::TransformModule(Module& module, TransformerContext& context, std::string* error, Nz::FunctionRef<void()> postCallback)
	{
		m_states->pendingFunctions.clear();

		if (!Transformer::TransformModule(module, context, error, postCallback))
			return false;

		ResolveFunctions();
		return true;
	}

	void ValidationTransformer::ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);
			if (arrayType.length == 0)
				throw CompilerArrayLengthRequiredError{ sourceLocation };
		}
		else if (IsLiteralType(exprType))
			throw AstUnexpectedUntypedError{ sourceLocation };
	}

	void ValidationTransformer::ValidateIntrinsicParameters(IntrinsicExpression& node)
	{
		auto intrinsicIt = LangData::s_intrinsicData.find(node.intrinsic);
		if (intrinsicIt == LangData::s_intrinsicData.end())
			throw AstInternalError{ node.sourceLocation, fmt::format("missing intrinsic data for intrinsic {}", Nz::UnderlyingCast(node.intrinsic)) };

		const auto& intrinsicData = intrinsicIt->second;

		std::optional<std::size_t> unresolvedParameter;

		std::size_t paramIndex = 0;
		std::size_t lastSameComponentCountBarrierIndex = 0;
		std::size_t lastSameParamBarrierIndex = 0;
		for (std::size_t i = 0; i < intrinsicData.parameterCount; ++i)
		{
			using namespace LangData::IntrinsicHelper;

			switch (intrinsicData.parameterTypes[i])
			{
				case ParameterType::ArrayDyn:
				{
					auto Check = [](const ExpressionType& type)
					{
						return IsArrayType(type) || IsDynArrayType(type);
					};

					if (ValidateIntrinsicParameterType(node, Check, "array/dyn-array", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::BValVec:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						if (primitiveType != PrimitiveType::Boolean)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "boolean value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::BVec:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						if (primitiveType != PrimitiveType::Boolean)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "boolean vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::F32:
				{
					auto Check = [](const ExpressionType& type)
					{
						return type == ExpressionType{ PrimitiveType::Float32 } || type == ExpressionType{ PrimitiveType::FloatLiteral };
					};

					if (ValidateIntrinsicParameterType(node, Check, "f32", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::FVal:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else
							return false;

						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64 && primitiveType != PrimitiveType::FloatLiteral)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point value", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::FValVec:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						// no float16 for now
						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64 && primitiveType != PrimitiveType::FloatLiteral)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::FValVec1632:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						// no float16 for now
						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::FloatLiteral)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "16/32bits floating-point value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::FVec:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64 && primitiveType != PrimitiveType::FloatLiteral)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::FVec3:
				{
					auto Check = [](const ExpressionType& type)
					{
						if (!IsVectorType(type))
							return false;

						const VectorType& vectorType = std::get<VectorType>(type);
						if (vectorType.componentCount != 3)
							return false;

						return vectorType.type == PrimitiveType::Float32 || vectorType.type == PrimitiveType::Float64 || vectorType.type == PrimitiveType::FloatLiteral;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point vec3", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::Matrix:
				{
					if (ValidateIntrinsicParameterType(node, IsMatrixType, "matrix", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::MatrixSquare:
				{
					auto Check = [](const ExpressionType& type)
					{
						if (!IsMatrixType(type))
							return false;

						const MatrixType& matrixType = std::get<MatrixType>(type);
						return matrixType.columnCount == matrixType.rowCount;
					};

					if (ValidateIntrinsicParameterType(node, Check, "square matrix", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::Numerical:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else
							return false;

						switch (primitiveType)
						{
							case PrimitiveType::Boolean:
							case PrimitiveType::String:
								break;

							case PrimitiveType::Float32:
							case PrimitiveType::Float64:
							case PrimitiveType::Int32:
							case PrimitiveType::UInt32:
							case PrimitiveType::FloatLiteral:
							case PrimitiveType::IntLiteral:
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::NumericalVec:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						switch (primitiveType)
						{
							case PrimitiveType::Boolean:
							case PrimitiveType::String:
								break;

							case PrimitiveType::Float32:
							case PrimitiveType::Float64:
							case PrimitiveType::Int32:
							case PrimitiveType::UInt32:
							case PrimitiveType::FloatLiteral:
							case PrimitiveType::IntLiteral:
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::SampleCoordinates:
				{
					// Special check: vector dimensions must match sample type
					const ExpressionType* firstParameter = GetExpressionType(*node.parameters[0]);
					if (!firstParameter)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					const SamplerType& samplerType = std::get<SamplerType>(ResolveAlias(*firstParameter));

					std::size_t requiredComponentCount = 0;
					switch (samplerType.dim)
					{
						case ImageType::E1D:
							requiredComponentCount = 1;
							break;

						case ImageType::E1D_Array:
						case ImageType::E2D:
							requiredComponentCount = 2;
							break;

						case ImageType::E2D_Array:
						case ImageType::E3D:
						case ImageType::Cubemap:
							requiredComponentCount = 3;
							break;
					}

					if (requiredComponentCount == 0)
						throw AstInternalError{ node.parameters[0]->sourceLocation, "unhandled sampler dimensions" };

					if (requiredComponentCount > 1)
					{
						// Vector coordinates
						auto Check = [=](const ExpressionType& type)
						{
							if (!IsVectorType(type))
								return false;

							const VectorType& vectorType = std::get<VectorType>(type);
							if (vectorType.componentCount != requiredComponentCount)
								return false;

							return vectorType.type == PrimitiveType::Float32 || vectorType.type == PrimitiveType::Float64 || vectorType.type == PrimitiveType::FloatLiteral;
						};

						char errMessage[] = "floating-point vector of X components";
						assert(requiredComponentCount < 9);
						errMessage[25] = Nz::SafeCast<char>('0' + requiredComponentCount);

						if (ValidateIntrinsicParameterType(node, Check, errMessage, paramIndex) == ValidationResult::Unresolved)
						{
							if (!unresolvedParameter)
								unresolvedParameter = paramIndex;

							paramIndex++;
							continue;
						}
					}
					else
					{
						// Scalar coordinates
						auto Check = [=](const ExpressionType& type)
						{
							if (!IsPrimitiveType(type))
								return false;

							PrimitiveType primitiveType = std::get<PrimitiveType>(type);
							return primitiveType == PrimitiveType::Float32 || primitiveType == PrimitiveType::Float64;
						};

						if (ValidateIntrinsicParameterType(node, Check, "floating-point value", paramIndex) == ValidationResult::Unresolved)
						{
							if (!unresolvedParameter)
								unresolvedParameter = paramIndex;

							paramIndex++;
							continue;
						}
					}

					paramIndex++;
					break;
				}

				case ParameterType::Scalar:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else
							return false;

						switch (primitiveType)
						{
							case PrimitiveType::String:
								break;

							case PrimitiveType::Boolean:
							case PrimitiveType::Float32:
							case PrimitiveType::Float64:
							case PrimitiveType::Int32:
							case PrimitiveType::UInt32:
							case PrimitiveType::FloatLiteral:
							case PrimitiveType::IntLiteral:
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::ScalarVec:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						switch (primitiveType)
						{
							case PrimitiveType::String:
								break;

							case PrimitiveType::Boolean:
							case PrimitiveType::Float32:
							case PrimitiveType::Float64:
							case PrimitiveType::Int32:
							case PrimitiveType::UInt32:
							case PrimitiveType::FloatLiteral:
							case PrimitiveType::IntLiteral:
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::SignedNumerical:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else
							return false;

						switch (primitiveType)
						{
							case PrimitiveType::Boolean:
							case PrimitiveType::String:
							case PrimitiveType::UInt32:
								break;

							case PrimitiveType::Float32:
							case PrimitiveType::Float64:
							case PrimitiveType::Int32:
							case PrimitiveType::FloatLiteral:
							case PrimitiveType::IntLiteral:
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::SignedNumericalVec:
				{
					auto Check = [](const ExpressionType& type)
					{
						PrimitiveType primitiveType;
						if (IsPrimitiveType(type))
							primitiveType = std::get<PrimitiveType>(type);
						else if (IsVectorType(type))
							primitiveType = std::get<VectorType>(type).type;
						else
							return false;

						switch (primitiveType)
						{
							case PrimitiveType::Boolean:
							case PrimitiveType::String:
							case PrimitiveType::UInt32:
								break;

							case PrimitiveType::Float32:
							case PrimitiveType::Float64:
							case PrimitiveType::Int32:
							case PrimitiveType::FloatLiteral:
							case PrimitiveType::IntLiteral:
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::Sampler:
				{
					if (ValidateIntrinsicParameterType(node, IsSamplerType, "sampler type", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::SameType:
				{
					if (ValidateIntrinsicParamMatchingType(node, lastSameParamBarrierIndex, paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex - 1;

						continue;
					}

					break;
				}

				case ParameterType::SameTypeBarrier:
				{
					lastSameParamBarrierIndex = paramIndex;
					break; //< Handled by SameType
				}

				case ParameterType::SameVecComponentCount:
				{
					if (ValidateIntrinsicParamMatchingVecComponent(node, lastSameComponentCountBarrierIndex, paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex - 1;

						continue;
					}

					break;
				}

				case ParameterType::SameVecComponentCountBarrier:
				{
					lastSameComponentCountBarrierIndex = paramIndex;
					break; //< Handled by SameType
				}

				case ParameterType::Texture:
				{
					if (ValidateIntrinsicParameterType(node, IsTextureType, "texture type", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						paramIndex++;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::TextureCoordinates:
				{
					// Special check: vector dimensions must match sample type
					const ExpressionType* firstParameter = GetExpressionType(*node.parameters[0]);
					if (!firstParameter)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						continue;
					}

					const TextureType& textureType = std::get<TextureType>(ResolveAlias(*firstParameter));
					std::size_t requiredComponentCount = 0;
					switch (textureType.dim)
					{
						case ImageType::E1D:
							requiredComponentCount = 1;
							break;

						case ImageType::E1D_Array:
						case ImageType::E2D:
							requiredComponentCount = 2;
							break;

						case ImageType::E2D_Array:
						case ImageType::E3D:
						case ImageType::Cubemap:
							requiredComponentCount = 3;
							break;
					}

					if (requiredComponentCount == 0)
						throw AstInternalError{ node.parameters[0]->sourceLocation, "unhandled texture dimensions" };

					if (requiredComponentCount > 1)
					{
						// Vector coordinates
						auto Check = [=](const ExpressionType& type)
						{
							if (!IsVectorType(type))
								return false;

							const VectorType& vectorType = std::get<VectorType>(type);
							if (vectorType.componentCount != requiredComponentCount)
								return false;

							return vectorType.type == PrimitiveType::Int32 || vectorType.type == PrimitiveType::IntLiteral;
						};

						char errMessage[] = "integer vector of X components";
						assert(requiredComponentCount < 9);
						errMessage[18] = Nz::SafeCast<char>('0' + requiredComponentCount);

						if (ValidateIntrinsicParameterType(node, Check, errMessage, paramIndex) == ValidationResult::Unresolved)
						{
							if (!unresolvedParameter)
								unresolvedParameter = paramIndex;

							continue;
						}
					}
					else
					{
						// Scalar coordinates
						auto Check = [=](const ExpressionType& type)
						{
							if (!IsPrimitiveType(type))
								return false;

							PrimitiveType primitiveType = std::get<PrimitiveType>(type);
							return primitiveType == PrimitiveType::Int32 || primitiveType == PrimitiveType::IntLiteral;
						};

						if (ValidateIntrinsicParameterType(node, Check, "integer value", paramIndex) == ValidationResult::Unresolved)
						{
							if (!unresolvedParameter)
								unresolvedParameter = paramIndex;

							continue;
						}
					}

					paramIndex++;
					break;
				}

				case ParameterType::TextureData:
				{
					// Special check: vector data
					const ExpressionType* firstParameter = GetExpressionType(*node.parameters[0]);
					if (!firstParameter)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						continue;
					}

					const TextureType& textureType = std::get<TextureType>(ResolveAlias(*firstParameter));

					// Vector data
					auto Check = [=](const ExpressionType& type)
					{
						if (!IsVectorType(type))
							return false;

						const VectorType& vectorType = std::get<VectorType>(type);
						if (vectorType.componentCount != 4)
							return false;

						return vectorType.type == textureType.baseType;
					};

					if (ValidateIntrinsicParameterType(node, Check, "texture-type vector of 4 components", paramIndex) == ValidationResult::Unresolved)
					{
						if (!unresolvedParameter)
							unresolvedParameter = paramIndex;

						continue;
					}

					paramIndex++;
					break;
				}
			}
		}

		if (node.parameters.size() != paramIndex)
			throw CompilerIntrinsicExpectedParameterCountError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(paramIndex) };

		if (unresolvedParameter && !m_context->partialCompilation)
			throw CompilerIntrinsicUnresolvedParameterError{ node.parameters[*unresolvedParameter]->sourceLocation, *unresolvedParameter };
	}

	auto ValidationTransformer::ValidateIntrinsicParamMatchingType(IntrinsicExpression& node, std::size_t from, std::size_t to) -> ValidationResult
	{
		// Check if all types prior to this one matches their type
		const ExpressionType* firstParameterType = GetExpressionType(*node.parameters[from]);
		if (!firstParameterType)
			return ValidationResult::Unresolved;

		const ExpressionType& matchingType = ResolveAlias(*firstParameterType);

		for (std::size_t i = from + 1; i < to; ++i)
		{
			const ExpressionType* parameterType = GetExpressionType(*node.parameters[i]);
			if (!parameterType)
				return ValidationResult::Unresolved;

			if (!ValidateMatchingTypes(matchingType, *parameterType))
				throw CompilerIntrinsicUnmatchingParameterTypeError{ node.parameters[i]->sourceLocation, Nz::SafeCast<std::uint32_t>(from), Nz::SafeCast<std::uint32_t>(to) };
		}

		return ValidationResult::Validated;
	}

	auto ValidationTransformer::ValidateIntrinsicParamMatchingVecComponent(IntrinsicExpression& node, std::size_t from, std::size_t to) -> ValidationResult
	{
		// Check if all vector types prior to this one matches their component count
		std::size_t componentCount = 0;
		for (; from < to; ++from)
		{
			const ExpressionType* firstParameterType = GetExpressionType(*node.parameters[from]);
			if (!firstParameterType)
				return ValidationResult::Unresolved;

			const ExpressionType& exprType = ResolveAlias(*firstParameterType);
			if (!IsVectorType(exprType))
				continue;

			componentCount = std::get<VectorType>(exprType).componentCount;
			break;
		}

		for (std::size_t i = from + 1; i < to; ++i)
		{
			const ExpressionType* parameterType = GetExpressionType(*node.parameters[i]);
			if (!parameterType)
				return ValidationResult::Unresolved;

			const ExpressionType& exprType = ResolveAlias(*parameterType);
			if (!IsVectorType(exprType))
				continue;

			if (componentCount != std::get<VectorType>(exprType).componentCount)
				throw CompilerIntrinsicUnmatchingVecComponentError{ node.parameters[i]->sourceLocation, Nz::SafeCast<std::uint32_t>(from), Nz::SafeCast<std::uint32_t>(to) };
		}

		return ValidationResult::Validated;
	}

	template<typename F>
	auto ValidationTransformer::ValidateIntrinsicParameter(IntrinsicExpression& node, F&& func, std::size_t index) -> ValidationResult
	{
		assert(index < node.parameters.size());
		auto& parameter = MandatoryExpr(node.parameters[index], node.sourceLocation);
		const ExpressionType* type = GetExpressionType(parameter);
		if (!type)
			return ValidationResult::Unresolved;

		const ExpressionType& resolvedType = ResolveAlias(*type);
		func(parameter, resolvedType);

		return ValidationResult::Validated;
	}

	template<typename F>
	auto ValidationTransformer::ValidateIntrinsicParameterType(IntrinsicExpression& node, F&& func, const char* typeStr, std::size_t index) -> ValidationResult
	{
		assert(index < node.parameters.size());
		auto& parameter = MandatoryExpr(node.parameters[index], node.sourceLocation);

		const ExpressionType* type = GetExpressionType(parameter);
		if (!type)
			return ValidationResult::Unresolved;

		const ExpressionType& resolvedType = ResolveAlias(*type);
		if (!func(resolvedType))
			throw CompilerIntrinsicExpectedTypeError{ parameter.sourceLocation, Nz::SafeCast<std::uint32_t>(index), typeStr, ToString(*type, parameter.sourceLocation)};

		return ValidationResult::Validated;
	}
}
