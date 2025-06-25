// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/IdentifierTransformer.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool IdentifierTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		return TransformModule(module, context, error);
	}

	bool IdentifierTransformer::HasIdentifier(std::string_view identifierName) const
	{
		return std::find_if(m_identifierInScope.rbegin(), m_identifierInScope.rend(), [&](const std::string& identifier) { identifier == identifierName; }) != m_identifierInScope.rend();
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

	bool IdentifierTransformer::SanitizeIdentifier(std::string& identifier, IdentifierType scope)
	{
		// Don't sanitize identifiers when performing partial sanitization (as it could break future compilation)
		if (!m_options->identifierSanitizer || m_context->partialSanitization)
			return;

		return m_options->identifierSanitizer(identifier, scope);
	}

	StatementPtr IdentifierTransformer::Transform(DeclareAliasStatement&& statement)
	{
		SanitizeIdentifier(statement.name, IdentifierType::Alias);
		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(DeclareConstStatement&& statement)
	{
		SanitizeIdentifier(statement.name, IdentifierType::Const);
		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(DeclareExternalStatement&& statement)
	{
		if (!statement.name.empty())
		{
			SanitizeIdentifier(statement.name, IdentifierType::ExternalBlock);
			PushScope();
		}

		for (auto& externalVar : statement.externalVars)
			SanitizeIdentifier(externalVar.name, IdentifierType::ExternalVariable);

		if (!statement.name.empty())
			PopScope();

		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(DeclareFunctionStatement&& statement)
	{
		SanitizeIdentifier(statement.name, IdentifierType::Function);
		for (auto& param : statement.parameters)
			SanitizeIdentifier(param.name, IdentifierType::Parameter);

		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(DeclareOptionStatement&& statement)
	{
		SanitizeIdentifier(statement.optName, IdentifierType::Parameter);
		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(DeclareStructStatement&& statement)
	{
		SanitizeIdentifier(statement.description.name, IdentifierType::Struct);
		for (auto& member : statement.description.members)
		{
			// FIXME: Why is this necessary?
			if (member.originalName.empty())
				member.originalName = member.name;

			SanitizeIdentifier(member.name, IdentifierType::Field);
		}

		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(DeclareVariableStatement&& statement)
	{
		SanitizeIdentifier(statement.varName, IdentifierType::Variable);
		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(ForEachStatement&& statement)
	{
		SanitizeIdentifier(statement.varName, IdentifierType::Variable);
		return nullptr;
	}

	StatementPtr IdentifierTransformer::Transform(ForStatement&& statement)
	{
		SanitizeIdentifier(statement.varName, IdentifierType::Variable);
		return nullptr;
	}
}
