// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/IdentifierTransformer.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <fmt/format.h>

namespace nzsl::Ast
{
	bool IdentifierTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		return TransformModule(module, context, error);
	}

	bool IdentifierTransformer::HasIdentifier(std::string_view identifierName) const
	{
		return std::find_if(m_identifierInScope.rbegin(), m_identifierInScope.rend(), [&](const std::string& identifier) { return identifier == identifierName; }) != m_identifierInScope.rend();
	}

	void IdentifierTransformer::RegisterIdentifier(std::string identifier)
	{
		m_identifierInScope.emplace_back(std::move(identifier));
	}

	void IdentifierTransformer::PopScope()
	{
		assert(!m_scopeIndices.empty());
		m_identifierInScope.resize(m_scopeIndices.back());
		m_scopeIndices.pop_back();
	}

	void IdentifierTransformer::PushScope()
	{
		m_scopeIndices.push_back(m_identifierInScope.size());
	}

	bool IdentifierTransformer::HandleIdentifier(std::string& identifier, IdentifierType scope)
	{
		// Don't rename identifiers when performing partial sanitization (as it could break future compilation)
		if (m_context->partialCompilation)
			return false;

		bool nameChanged = false;
		if (m_options->identifierSanitizer)
			nameChanged = m_options->identifierSanitizer(identifier, scope);
		
		if (m_options->makeVariableNameUnique)
		{
			if (HasIdentifier(identifier))
			{
				// Try to make identifier unique by appending _X to its name (incrementing X until it's unique) until it's unique
				unsigned int cloneIndex = 2;
				std::string candidateName;
				do
				{
					candidateName = fmt::format("{}_{}", identifier, cloneIndex++);
				}
				while (HasIdentifier(candidateName));

				identifier = std::move(candidateName);
				nameChanged = true;
			}

			RegisterIdentifier(identifier);
		}

		return nameChanged;
	}

	auto IdentifierTransformer::Transform(DeclareAliasStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.name, IdentifierType::Alias);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareConstStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.name, IdentifierType::Const);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareExternalStatement&& statement) -> StatementTransformation
	{
		if (!statement.name.empty())
		{
			HandleIdentifier(statement.name, IdentifierType::ExternalBlock);
			PushScope();
		}

		for (auto& externalVar : statement.externalVars)
			HandleIdentifier(externalVar.name, IdentifierType::ExternalVariable);

		if (!statement.name.empty())
			PopScope();

		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareFunctionStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.name, IdentifierType::Function);
		for (auto& param : statement.parameters)
			HandleIdentifier(param.name, IdentifierType::Parameter);

		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareOptionStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.optName, IdentifierType::Parameter);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareStructStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.description.name, IdentifierType::Struct);
		
		PushScope();
		for (auto& member : statement.description.members)
		{
			// FIXME: Why is this necessary?
			if (member.originalName.empty())
				member.originalName = member.name;

			HandleIdentifier(member.name, IdentifierType::Field);
		}
		PopScope();

		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareVariableStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.varName, IdentifierType::Variable);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(ForEachStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.varName, IdentifierType::Variable);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(ForStatement&& statement) -> StatementTransformation
	{
		HandleIdentifier(statement.varName, IdentifierType::Variable);
		return VisitChildren{};
	}
}
