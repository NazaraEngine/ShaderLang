// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_UNIFORMSTRUCTTOSTD140_HPP
#define NZSL_AST_TRANSFORMATIONS_UNIFORMSTRUCTTOSTD140_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API UniformStructToStd140Transformer final : public Transformer
	{
		public:
			struct Options;

			UniformStructToStd140Transformer() = default;

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
			};

		private:
			using Transformer::Transform;

			StatementTransformation Transform(DeclareExternalStatement&& statement) override;
			StatementTransformation Transform(DeclareStructStatement&& declStruct) override;

			std::unordered_map<std::size_t /*structIndex*/, StructDescription*> m_structDescs;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/UniformStructToStd140.inl>

#endif // NZSL_AST_TRANSFORMATIONS_UNIFORMSTRUCTTOSTD140_HPP
