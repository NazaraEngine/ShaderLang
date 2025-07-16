// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <fmt/format.h>
#include <tsl/ordered_map.h>
#include <unordered_map>

namespace nzsl::Ast
{
	struct ValidationTransformer::States
	{
		struct FunctionData
		{
			std::unordered_multimap<BuiltinEntry, SourceLocation> usedBuiltins;
			std::unordered_multimap<ShaderStageType, SourceLocation> requiredShaderStage;
			Nz::HybridBitset<Nz::UInt32, 32> calledFunctions;
			Nz::HybridBitset<Nz::UInt32, 32> calledByFunctions;
			ShaderStageTypeFlags calledByStages;
			const DeclareFunctionStatement* node;
			std::optional<ShaderStageType> entryStage;
		};

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
		Module* rootModule;
		unsigned int loopCounter = 0;
	};

	bool ValidationTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		States states;
		states.rootModule = &module;
		m_states = &states;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	bool ValidationTransformer::TransformExpression(Module& module, ExpressionPtr& expression, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		States states;
		states.rootModule = &module;
		m_states = &states;

		return Transformer::TransformExpression(expression, context, error);
	}

	bool ValidationTransformer::TransformStatement(Module& module, StatementPtr& statement, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		States states;
		states.rootModule = &module;
		m_states = &states;

		return Transformer::TransformStatement(statement, context, error);
	}

	Stringifier ValidationTransformer::BuildStringifier(const SourceLocation& sourceLocation) const
	{
		Stringifier stringifier;
		/*stringifier.aliasStringifier = [&](std::size_t aliasIndex)
		{
			return m_states->aliases.Retrieve(aliasIndex, sourceLocation).name;
		};*/

		stringifier.moduleStringifier = [&](std::size_t moduleIndex)
		{
			const std::string& moduleName = m_states->rootModule->importedModules[moduleIndex].identifier;
			return (!moduleName.empty()) ? moduleName : fmt::format("<anonymous module #{}>", moduleIndex);
		};
		
		/*stringifier.namedExternalBlockStringifier = [&](std::size_t namedExternalBlockIndex)
		{
			return m_states->namedExternalBlocks.Retrieve(namedExternalBlockIndex, sourceLocation).name;
		};*/

		stringifier.structStringifier = [&](std::size_t structIndex)
		{
			const StructDescription* structDesc = Nz::Retrieve(m_states->structs, structIndex);
			return structDesc->name;
		};

		/*stringifier.typeStringifier = [&](std::size_t typeIndex)
		{
			return ToString(m_states->types.Retrieve(typeIndex, sourceLocation), sourceLocation);
		};*/

		return stringifier;
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

		Nz::HybridBitset<Nz::UInt32, 32> calledByFunctions;
		Nz::HybridBitset<Nz::UInt32, 32> seen;
		Nz::HybridBitset<Nz::UInt32, 32> temp;
		for (auto it = m_states->functions.begin(); it != m_states->functions.end(); ++it)
		{
			std::size_t funcIndex = it.key();
			auto& funcData = it.value();

			seen.Clear();
			seen.UnboundedSet(funcIndex);

			calledByFunctions.Clear();
			calledByFunctions |= funcData.calledByFunctions;

			std::size_t callingFuncIndex;
			while ((callingFuncIndex = calledByFunctions.FindFirst()) != calledByFunctions.npos)
			{
				auto callingFuncIt = m_states->functions.find(callingFuncIndex);
				assert(callingFuncIt != m_states->functions.end());

				auto& callingFunctionData = callingFuncIt.value();
				funcData.calledByStages |= callingFunctionData.calledByStages;

				calledByFunctions |= callingFunctionData.calledByFunctions;
				
				// Prevent infinite recursion
				// calledByFunctions &= ~seen;
				temp.PerformsNOT(seen);
				calledByFunctions &= temp;

				seen.UnboundedSet(callingFuncIndex);
			}

			for (std::size_t flagIndex = 0; flagIndex <= static_cast<std::size_t>(ShaderStageType::Max); ++flagIndex)
			{
				ShaderStageType stageType = static_cast<ShaderStageType>(flagIndex);
				if (!funcData.calledByStages.Test(stageType))
					continue;

				// Check builtin usage
				for (auto&& [builtin, sourceLocation] : funcData.usedBuiltins)
				{
					auto it = LangData::s_builtinData.find(builtin);
					if (it == LangData::s_builtinData.end())
						throw AstInternalError{ sourceLocation, "missing builtin data" };

					const LangData::BuiltinData& builtinData = it->second;
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
		return Ast::ToString(exprType, (m_options->stringifier) ? *m_options->stringifier : BuildStringifier(sourceLocation));
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
				remainingIndices--;
				break;
			}
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

	auto ValidationTransformer::Transform(AliasValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
			CheckAliasIndex(node.aliasId, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AssignExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(node.left, node.sourceLocation));
		if (!leftExprType)
			return DontVisitChildren{};

		if (GetExpressionCategory(*node.left) != ExpressionCategory::LValue)
			throw CompilerAssignTemporaryError{ node.sourceLocation };

		const ExpressionType* rightExprType = GetExpressionType(MandatoryExpr(node.right, node.sourceLocation));
		if (!rightExprType)
			return DontVisitChildren{};

		std::optional<BinaryType> binaryType;
		switch (node.op)
		{
			case AssignType::Simple:
				TypeMustMatch(*leftExprType, UnwrapExternalType(ResolveAlias(*rightExprType)), node.sourceLocation);
				break;

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
			ExpressionType expressionType = ValidateBinaryOp(*binaryType, ResolveAlias(*leftExprType), UnwrapExternalType(ResolveAlias(*rightExprType)), node.sourceLocation);
			TypeMustMatch(UnwrapExternalType(*leftExprType), expressionType, node.sourceLocation);
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

		std::size_t functionIndex = std::get<Ast::FunctionType>(*GetExpressionType(*node.targetFunction)).funcIndex;
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
			const ExpressionType* parameterType = GetExpressionType(*node.parameters[i].expr);
			if (!parameterType)
				continue;

			if (ResolveAlias(*parameterType) != ResolveAlias(referenceDeclaration->parameters[i].type.GetResultingValue()))
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

					if (vecType.type != targetMatrixType.type)
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

					const PrimitiveType& baseType = std::get<PrimitiveType>(resolvedExprType);
					if (baseType != targetMatrixType.type)
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
					if (primitiveType != targetBaseType)
						throw CompilerCastIncompatibleBaseTypesError{ exprPtr->sourceLocation, ToString(targetBaseType, node.sourceLocation), ToString(primitiveType, exprPtr->sourceLocation) };
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
					else
					{
						if (primitiveType != targetBaseType)
							throw CompilerCastIncompatibleBaseTypesError{ exprPtr->sourceLocation, ToString(targetBaseType, node.sourceLocation), ToString(primitiveType, exprPtr->sourceLocation) };
					}
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

			const ExpressionType& innerType = targetArrayType.containedType->type;
			for (std::size_t i = 0; i < node.expressions.size(); ++i)
			{
				const auto& exprPtr = node.expressions[i];
				assert(exprPtr);

				const ExpressionType* exprType = GetExpressionType(*exprPtr);
				if (!exprType)
					return DontVisitChildren{};

				if (innerType != *exprType)
					throw CompilerCastIncompatibleTypesError{ exprPtr->sourceLocation, ToString(innerType, node.sourceLocation), ToString(*exprType, exprPtr->sourceLocation) };
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

	auto ValidationTransformer::Transform(ConstantExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
			CheckConstIndex(node.constantId, node.sourceLocation);

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

	auto ValidationTransformer::Transform(FunctionExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
			CheckFuncIndex(node.funcId, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IdentifierExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IntrinsicExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		// Parameter validation
		ValidateIntrinsicParameters(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IntrinsicFunctionExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ModuleExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
		{
			if (node.moduleId >= m_states->rootModule->importedModules.size())
				throw AstIndexOutOfBoundsError{ node.sourceLocation, "module", static_cast<std::int32_t>(node.moduleId) };
		}
		
		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(NamedExternalBlockExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
			CheckExternalIndex(node.externalBlockId, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(StructTypeExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
			CheckStructIndex(node.structTypeId, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(SwizzleExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(TypeExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(UnaryExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(VariableValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		if (m_options->checkIndices)
			CheckVariableIndex(node.variableId, node.sourceLocation);

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
				if (module.moduleIndex >= m_states->rootModule->importedModules.size())
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

		if (!node.name.empty())
			PushScope();

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

		if (!node.name.empty())
			PopScope();

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareFunctionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (!node.funcIndex)
			throw AstExpectedIndexError{ node.sourceLocation, "function" };

		if (m_options->checkIndices)
			RegisterFunc(*node.funcIndex, node.sourceLocation);

		States::FunctionData& functionData = m_states->functions[*node.funcIndex];
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
					if (ResolveAlias(optionType) != ResolveAlias(*defaultValueType))
						throw CompilerVarDeclarationTypeUnmatchingError{ node.sourceLocation, ToString(optionType, node.sourceLocation), ToString(*defaultValueType, node.defaultValue->sourceLocation) };
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
			if (member.type.HasValue() && member.type.IsExpression())
			{
				if (!m_context->partialCompilation)
					throw AstMissingExpressionTypeError{ member.sourceLocation };

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
			if (ResolveAlias(*returnTypeOpt) != ResolveAlias(functionReturnType))
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

	void ValidationTransformer::TypeMustMatch(const ExpressionType& left, const ExpressionType& right, const SourceLocation& sourceLocation) const
	{
		if (ResolveAlias(left) != ResolveAlias(right))
			throw CompilerUnmatchingTypesError{ sourceLocation, ToString(left, sourceLocation), ToString(right, sourceLocation) };
	}

	bool ValidationTransformer::TransformModule(Module& module, Context& context, std::string* error, Nz::FunctionRef<void()> postCallback)
	{
		m_states->pendingFunctions.clear();

		PushScope();
		NAZARA_DEFER({ PopScope(); });

		if (!Transformer::TransformModule(module, context, error, postCallback))
			return false;

		ResolveFunctions();
		return true;
	}

	void ValidationTransformer::TypeMustMatch(const ExpressionPtr& left, const ExpressionPtr& right, const SourceLocation& sourceLocation) const
	{
		const ExpressionType* leftType = GetExpressionType(*left);
		const ExpressionType* rightType = GetExpressionType(*right);
		if (!leftType || !rightType)
			return;

		return TypeMustMatch(*leftType, *rightType, sourceLocation);
	}

	void ValidationTransformer::ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);
			if (arrayType.length == 0)
				throw CompilerArrayLengthRequiredError{ sourceLocation };
		}
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
						unresolvedParameter = paramIndex;
						continue;
					}

					paramIndex++;
					break;
				}

				case ParameterType::F32:
				{
					auto Check = [](const ExpressionType& type)
					{
						return type == ExpressionType{ PrimitiveType::Float32 };
					};

					if (ValidateIntrinsicParameterType(node, Check, "f32", paramIndex) == ValidationResult::Unresolved)
					{
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

						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point value", paramIndex) == ValidationResult::Unresolved)
					{
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
						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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
						if (primitiveType != PrimitiveType::Float32)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "16/32bits floating-point value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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

						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64)
							return false;

						return true;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point vector", paramIndex) == ValidationResult::Unresolved)
					{
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

						return vectorType.type == PrimitiveType::Float32 || vectorType.type == PrimitiveType::Float64;
					};

					if (ValidateIntrinsicParameterType(node, Check, "floating-point vec3", paramIndex) == ValidationResult::Unresolved)
					{
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
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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

							return vectorType.type == PrimitiveType::Float32 || vectorType.type == PrimitiveType::Float64;
						};

						char errMessage[] = "floating-point vector of X components";
						assert(requiredComponentCount < 9);
						errMessage[25] = Nz::SafeCast<char>('0' + requiredComponentCount);

						if (ValidateIntrinsicParameterType(node, Check, errMessage, paramIndex) == ValidationResult::Unresolved)
						{
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
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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
								return true;
						}

						return false;
					};

					if (ValidateIntrinsicParameterType(node, Check, "scalar value or vector", paramIndex) == ValidationResult::Unresolved)
					{
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
						unresolvedParameter = paramIndex;
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
						unresolvedParameter = paramIndex;
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

							return vectorType.type == PrimitiveType::Int32;
						};

						char errMessage[] = "integer vector of X components";
						assert(requiredComponentCount < 9);
						errMessage[18] = Nz::SafeCast<char>('0' + requiredComponentCount);

						if (ValidateIntrinsicParameterType(node, Check, errMessage, paramIndex) == ValidationResult::Unresolved)
						{
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
							return primitiveType == PrimitiveType::Int32;
						};

						if (ValidateIntrinsicParameterType(node, Check, "integer value", paramIndex) == ValidationResult::Unresolved)
						{
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

		if (unresolvedParameter)
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

			if (matchingType != ResolveAlias(*parameterType))
				throw CompilerIntrinsicUnmatchingParameterTypeError{ node.parameters[i]->sourceLocation, Nz::SafeCast<std::uint32_t>(from), Nz::SafeCast<std::uint32_t>(to) };
		}

		return ValidationResult::Validated;
	}

	auto ValidationTransformer::ValidateIntrinsicParamMatchingVecComponent(IntrinsicExpression& node, std::size_t from, std::size_t to) -> ValidationResult
	{
		// Check if all types prior to this one matches their type
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
