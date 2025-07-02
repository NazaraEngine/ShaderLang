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
		return std::find_if(m_identifierInScope.rbegin(), m_identifierInScope.rend(), [&](const std::string& identifier) { return identifier == identifierName; }) != m_identifierInScope.rend();
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
			return false;

		return m_options->identifierSanitizer(identifier, scope);
	}

	auto IdentifierTransformer::Transform(DeclareAliasStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.name, IdentifierType::Alias);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareConstStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.name, IdentifierType::Const);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareExternalStatement&& statement) -> StatementTransformation
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

		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareFunctionStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.name, IdentifierType::Function);
		for (auto& param : statement.parameters)
			SanitizeIdentifier(param.name, IdentifierType::Parameter);

		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareOptionStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.optName, IdentifierType::Parameter);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareStructStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.description.name, IdentifierType::Struct);
		for (auto& member : statement.description.members)
		{
			// FIXME: Why is this necessary?
			if (member.originalName.empty())
				member.originalName = member.name;

			SanitizeIdentifier(member.name, IdentifierType::Field);
		}

		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(DeclareVariableStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.varName, IdentifierType::Variable);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(ForEachStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.varName, IdentifierType::Variable);
		return VisitChildren{};
	}

	auto IdentifierTransformer::Transform(ForStatement&& statement) -> StatementTransformation
	{
		SanitizeIdentifier(statement.varName, IdentifierType::Variable);
		return VisitChildren{};
	}
}
