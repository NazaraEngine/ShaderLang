// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMEREXECUTOR_HPP
#define NZSL_AST_TRANSFORMEREXECUTOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Transformations/Transformer.hpp>
#include <memory>
#include <string>
#include <vector>

namespace nzsl::Ast
{
	class Module;

	class TransformerExecutor
	{
		public:
			TransformerExecutor() = default;
			TransformerExecutor(const TransformerExecutor&) = delete;
			TransformerExecutor(TransformerExecutor&&) noexcept = default;
			~TransformerExecutor() = default;

			template<typename T> void AddPass(const typename T::Options& options = {});

			inline bool Transform(Module& module, std::string* error = nullptr) const;
			inline bool Transform(Module& module, Transformer::Context& context, std::string* error = nullptr) const;

			TransformerExecutor& operator=(const TransformerExecutor&) = delete;
			TransformerExecutor& operator=(TransformerExecutor&&) noexcept = default;

		private:
			struct PassInterface
			{
				virtual ~PassInterface() = default;

				virtual bool Transform(Module& module, Transformer::Context& context, std::string* error = nullptr) = 0;
			};

			template<typename T>
			struct Pass : PassInterface
			{
				bool Transform(Module& module, Transformer::Context& context, std::string* error) override;

				T transformer;
				typename T::Options options;
			};

			std::vector<std::unique_ptr<PassInterface>> m_passes;
	};
}

#include <NZSL/Ast/TransformerExecutor.inl>

#endif // NZSL_AST_TRANSFORMEREXECUTOR_HPP
