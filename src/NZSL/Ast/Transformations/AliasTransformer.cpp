// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/AliasTransformer.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl::Ast
{
	bool AliasTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	void AliasTransformer::Transform(ExpressionType& expressionType, const SourceLocation& /*sourceLocation*/)
	{
		if (IsAliasType(expressionType))
			expressionType = ResolveAlias(expressionType);
	}

	auto AliasTransformer::Transform(IdentifierValueExpression&& identifierValue) -> ExpressionTransformation
	{
		if (identifierValue.identifierType != IdentifierType::Alias)
			return VisitChildren{};

		const auto& aliasData = m_context->aliases.Retrieve(identifierValue.identifierIndex, identifierValue.sourceLocation);
		identifierValue.identifierType = aliasData.identifier.target.type;
		identifierValue.identifierIndex = aliasData.identifier.target.index;

		return VisitChildren{};
	}

	auto AliasTransformer::Transform(DeclareAliasStatement&& /*declareAlias*/) -> StatementTransformation
	{
		return RemoveStatement{};
	}
}
