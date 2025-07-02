// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_TYPETRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_TYPETRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	struct PartialType;

	class NZSL_API TypeTransformer : public Transformer
	{
		protected:
			virtual bool IsFeatureEnabled(ModuleFeature feature) const = 0;

			void RegisterBuiltin();

			virtual std::size_t RegisterConstant(std::string name, ConstantValue&& value, std::optional<std::size_t> index, const SourceLocation& sourceLocation) = 0;
			virtual std::size_t RegisterIntrinsic(std::string name, IntrinsicType type) = 0;
			virtual std::size_t RegisterType(std::string name, ExpressionType&& expressionType, std::optional<std::size_t> index, const SourceLocation& sourceLocation) = 0;
			virtual std::size_t RegisterType(std::string name, PartialType&& partialType, std::optional<std::size_t> index, const SourceLocation& sourceLocation) = 0;
	};
}

#include <NZSL/Ast/Transformations/TypeTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_TYPETRANSFORMER_HPP
