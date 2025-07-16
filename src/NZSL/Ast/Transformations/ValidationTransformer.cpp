// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <fmt/format.h>
#include <unordered_map>
#include <NazaraUtils/CallOnExit.hpp>

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

		std::unordered_map<std::size_t, FunctionData> functions;
		std::unordered_map<std::size_t, const StructDescription*> structs;
		std::vector<DeclareFunctionStatement*> pendingFunctions;
		std::vector<Scope> scopes;
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
		if (m_states->registeredAliases.UnboundedTest(aliasIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, aliasIndex };

		m_states->registeredAliases.UnboundedSet(aliasIndex);
		auto& scope = m_states->scopes.back();
		scope.aliases.push_back(aliasIndex);
	}

	void ValidationTransformer::RegisterConst(std::size_t constIndex, const SourceLocation& sourceLocation)
	{
		if (m_states->registeredConsts.UnboundedTest(constIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, constIndex };

		m_states->registeredConsts.UnboundedSet(constIndex);
		auto& scope = m_states->scopes.back();
		scope.consts.push_back(constIndex);
	}

	void ValidationTransformer::RegisterExternal(std::size_t externalIndex, const SourceLocation& sourceLocation)
	{
		if (m_states->registeredExternals.UnboundedTest(externalIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, externalIndex };

		m_states->registeredExternals.UnboundedSet(externalIndex);
		auto& scope = m_states->scopes.back();
		scope.externals.push_back(externalIndex);
	}

	void ValidationTransformer::RegisterFunc(std::size_t funcIndex, const SourceLocation& sourceLocation)
	{
		if (m_states->registeredFuncs.UnboundedTest(funcIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, funcIndex };

		m_states->registeredFuncs.UnboundedSet(funcIndex);
		auto& scope = m_states->scopes.back();
		scope.functions.push_back(funcIndex);
	}

	void ValidationTransformer::RegisterStruct(std::size_t structIndex, const SourceLocation& sourceLocation)
	{
		if (m_states->registeredStructs.UnboundedTest(structIndex))
			throw AstAlreadyUsedIndexError{ sourceLocation, structIndex };

		m_states->registeredStructs.UnboundedSet(structIndex);
		auto& scope = m_states->scopes.back();
		scope.structs.push_back(structIndex);
	}

	void ValidationTransformer::RegisterVariable(std::size_t variableIndex, const SourceLocation& sourceLocation)
	{
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

			for (auto& parameter : pendingFunc->parameters)
			{
				if (parameter.varIndex)
					RegisterVariable(*parameter.varIndex, parameter.sourceLocation);
			}
			
			assert(pendingFunc->funcIndex);
			std::size_t funcIndex = *pendingFunc->funcIndex;

			auto& funcData = Nz::Retrieve(m_states->functions, funcIndex);
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
				auto& targetFunc = Nz::Retrieve(m_states->functions, calledFunctionIndex);
				targetFunc.calledByFunctions.UnboundedSet(*pendingFunc->funcIndex);
			}

			PopScope();
		}

		Nz::HybridBitset<Nz::UInt32, 32> calledByFunctions;
		Nz::HybridBitset<Nz::UInt32, 32> seen;
		Nz::HybridBitset<Nz::UInt32, 32> temp;
		for (std::size_t funcIndex : m_states->registeredFuncs.IterBits())
		{
			seen.Clear();
			seen.UnboundedSet(funcIndex);

			auto& funcData = m_states->functions[funcIndex];

			calledByFunctions.Clear();
			calledByFunctions |= funcData.calledByFunctions;
			for (std::size_t i : calledByFunctions.IterBits())
			{
				seen.UnboundedSet(funcIndex);

				auto& callingFunctionData = Nz::Retrieve(m_states->functions, funcIndex);
				funcData.calledByStages |= funcData.calledByStages;

				calledByFunctions |= callingFunctionData.calledByFunctions;
				
				// Prevent infinite recursion
				// calledByFunctions &= ~seen;
				temp.PerformsNOT(seen);
				calledByFunctions &= temp;
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

		std::uint32_t remainingIndices = node.fieldIndex;
		for (const auto& member : structDesc->members)
		{
			if (member.cond.HasValue() && !member.cond.GetResultingValue())
				continue;

			if (remainingIndices == 0)
				break;

			remainingIndices--;
		}

		if (remainingIndices > 0)
			throw AstIndexOutOfBoundsError{ node.sourceLocation, "struct field", static_cast<std::int32_t>(node.fieldIndex) };

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

		if (node.moduleId >= m_states->rootModule->importedModules.size())
			throw AstIndexOutOfBoundsError{ node.sourceLocation, "module", static_cast<std::int32_t>(node.moduleId) };

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(NamedExternalBlockExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		CheckExternalIndex(node.externalBlockId, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(StructTypeExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

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
			CheckStructIndex(structIndex, node.expression->sourceLocation);
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;
			CheckFuncIndex(funcIndex, node.expression->sourceLocation);
		}
		else if (IsAliasType(resolvedType))
		{
			const AliasType& alias = std::get<AliasType>(resolvedType);
			CheckAliasIndex(alias.aliasIndex, node.expression->sourceLocation);
		}
		else if (IsModuleType(resolvedType))
		{
			const ModuleType& module = std::get<ModuleType>(resolvedType);

			if (module.moduleIndex >= m_states->rootModule->importedModules.size())
				throw AstIndexOutOfBoundsError{ node.sourceLocation, "module", static_cast<std::int32_t>(module.moduleIndex) };
		}
		else
			throw CompilerAliasUnexpectedTypeError{ node.sourceLocation, Ast::ToString(*exprType) };

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareConstStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.constIndex)
			RegisterConst(*node.constIndex, node.sourceLocation);
		
		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareExternalStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.externalIndex)
			RegisterExternal(*node.externalIndex, node.sourceLocation);

		if (!node.name.empty())
			PushScope();

		for (auto& extVar : node.externalVars)
		{
			if (extVar.varIndex)
				RegisterVariable(*extVar.varIndex, extVar.sourceLocation);
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

		States::FunctionData& functionData = m_states->functions[*node.funcIndex];
		functionData.node = &node;

		if (node.entryStage.IsResultingValue())
			functionData.entryStage = node.entryStage.GetResultingValue();

		m_states->pendingFunctions.push_back(&node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareOptionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.optIndex)
			RegisterConst(*node.optIndex, node.sourceLocation);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareStructStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.structIndex)
		{
			RegisterStruct(*node.structIndex, node.sourceLocation);
			m_states->structs[*node.structIndex] = &node.description;
		}
		
		for (auto& member : node.description.members)
		{
			if (member.type.HasValue() && member.type.IsExpression())
			{
				assert(m_context->partialCompilation);
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

		if (node.varIndex)
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

		if (node.varIndex)
			RegisterVariable(*node.varIndex, node.sourceLocation);

		m_states->loopCounter++;
		NAZARA_DEFER({ m_states->loopCounter--; });
		HandleStatement(node.statement);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ForEachStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.varIndex)
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

	void ValidationTransformer::TypeMustMatch(const ExpressionPtr& left, const ExpressionPtr& right, const SourceLocation& sourceLocation) const
	{
		const ExpressionType* leftType = GetExpressionType(*left);
		const ExpressionType* rightType = GetExpressionType(*right);
		if (!leftType || !rightType)
			return;

		return TypeMustMatch(*leftType, *rightType, sourceLocation);
	}
}
