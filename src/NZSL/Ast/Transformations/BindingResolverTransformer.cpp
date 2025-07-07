// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/BindingResolverTransformer.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackVector.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool BindingResolverTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_usedBindingIndexes.clear();
		m_currentConditionalIndex = 0;
		m_nextConditionalIndex = 1;
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto BindingResolverTransformer::Transform(ConditionalStatement&& condStatement) -> StatementTransformation
	{
		unsigned int prevCondStatementIndex = m_currentConditionalIndex;

		m_currentConditionalIndex = m_nextConditionalIndex++;
		HandleChildren(condStatement);
		m_currentConditionalIndex = prevCondStatementIndex;

		return DontVisitChildren{};
	}

	auto BindingResolverTransformer::Transform(DeclareExternalStatement&& externalStatement) -> StatementTransformation
	{
		std::optional<std::uint32_t> defaultBlockSet = 0;
		if (externalStatement.bindingSet.HasValue())
		{
			if (externalStatement.bindingSet.IsResultingValue())
				defaultBlockSet = externalStatement.bindingSet.GetResultingValue();
			else
			{
				if (!m_context->partialCompilation)
					throw CompilerConstantExpressionRequiredError{ externalStatement.sourceLocation };

				defaultBlockSet.reset(); //< Unresolved value
			}
		}

		std::optional<bool> hasAutoBinding = false;
		if (externalStatement.autoBinding.HasValue())
		{
			if (externalStatement.autoBinding.IsResultingValue())
				hasAutoBinding = externalStatement.autoBinding.GetResultingValue();
			else
			{
				if (!m_context->partialCompilation)
					throw CompilerConstantExpressionRequiredError{ externalStatement.sourceLocation };

				hasAutoBinding.reset(); //< Unresolved value
			}
		}
		
		auto BuildBindingKey = [](std::uint32_t bindingSet, std::uint32_t bindingIndex)
		{
			return std::uint64_t(bindingSet) << 32 | bindingIndex;
		};

		auto RegisterBinding = [&](std::uint32_t count, std::uint32_t bindingSet, std::uint32_t bindingIndex, const SourceLocation& sourceLoc)
		{
			for (std::uint32_t i = 0; i < count; ++i)
			{
				std::uint64_t bindingKey = BuildBindingKey(bindingSet, bindingIndex + i);
				if (auto it = m_usedBindingIndexes.find(bindingKey); it != m_usedBindingIndexes.end())
				{
					if ((it->second == 0 || it->second == m_currentConditionalIndex))
						throw CompilerExtBindingAlreadyUsedError{ sourceLoc, bindingSet, bindingIndex };
				}

				m_usedBindingIndexes.emplace(bindingKey, m_currentConditionalIndex);
			}
		};

		bool hasUnresolved = false;
		Nz::StackVector<std::size_t> autoBindingEntries = NazaraStackVector(std::size_t, externalStatement.externalVars.size());
		for (std::size_t i = 0; i < externalStatement.externalVars.size(); ++i)
		{
			auto& extVar = externalStatement.externalVars[i];

			if (!extVar.type.IsResultingValue())
			{
				if (!m_context->partialCompilation)
					throw AstMissingTypeError{ extVar.sourceLocation };

				continue;
			}

			const ExpressionType& targetType = ResolveAlias(extVar.type.GetResultingValue());
			if (IsPushConstantType(targetType))
				continue;

			if (!extVar.bindingSet.HasValue() && defaultBlockSet)
				extVar.bindingSet = *defaultBlockSet;

			if (!extVar.bindingIndex.HasValue())
			{
				if (hasAutoBinding == false)
					throw CompilerExtMissingBindingIndexError{ extVar.sourceLocation };
				else if (hasAutoBinding == true && extVar.bindingSet.IsResultingValue())
				{
					// Don't resolve binding indices (?) when performing a partial compilation
					if (!m_context->partialCompilation || m_options->forceAutoBindingResolve)
						autoBindingEntries.push_back(i);
				}
			}

			if (extVar.bindingSet.IsResultingValue() && extVar.bindingIndex.IsResultingValue())
			{
				std::uint32_t bindingSet = extVar.bindingSet.GetResultingValue();
				std::uint32_t bindingIndex = extVar.bindingIndex.GetResultingValue();

				std::uint32_t arraySize = (IsArrayType(targetType)) ? std::get<ArrayType>(targetType).length : 1;
				RegisterBinding(arraySize, bindingSet, bindingIndex, extVar.sourceLocation);
			}
		}

		if (!hasUnresolved)
		{
			for (std::size_t extVarIndex : autoBindingEntries)
			{
				auto& extVar = externalStatement.externalVars[extVarIndex];

				// Since we're not in a partial compilation at this point, binding set has a known value
				assert(extVar.bindingSet.IsResultingValue());

				// Find first binding range
				std::uint32_t bindingSet = extVar.bindingSet.GetResultingValue();
				std::uint32_t bindingIndex = 0;

				// Type cannot be unresolved here
				const ExpressionType& targetType = ResolveAlias(extVar.type.GetResultingValue());
				std::uint32_t arraySize = (IsArrayType(targetType)) ? std::get<ArrayType>(targetType).length : 1;

				auto SearchFreeBindingRange = [&](std::uint32_t& binding)
				{
					for (std::uint32_t i = 0; i < arraySize; ++i)
					{
						if (m_usedBindingIndexes.find(BuildBindingKey(bindingSet, binding + i)) != m_usedBindingIndexes.end())
						{
							binding += i;
							return false;
						}
					}

					return true;
				};

				while (!SearchFreeBindingRange(bindingIndex))
					bindingIndex++;

				extVar.bindingIndex = bindingIndex;
				RegisterBinding(arraySize, bindingSet, bindingIndex, extVar.sourceLocation);
			}
		}

		return DontVisitChildren{};
	}
}
