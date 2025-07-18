// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	template<typename T>
	void TransformerExecutor::AddPass(const typename T::Options& options)
	{
		return AddPass<T>(m_passes.size(), options);
	}

	template<typename T>
	void TransformerExecutor::AddPass(std::size_t index, const typename T::Options& options)
	{
		auto passPtr = std::make_unique<Pass<T>>();
		passPtr->options = options;

		m_passes.emplace(m_passes.begin() + index, std::move(passPtr));
	}

	inline bool TransformerExecutor::Transform(Module& module, std::string* error) const
	{
		Transformer::Context context;
		return Transform(module, context, error);
	}

	inline bool TransformerExecutor::Transform(Module& module, Transformer::Context& context, std::string* error) const
	{
		for (auto& passPtr : m_passes)
		{
			if (!passPtr->Transform(module, context, error))
				return false;
		}

		return true;
	}

	template<typename T>
	bool TransformerExecutor::Pass<T>::Transform(Module& module, Transformer::Context& context, std::string* error)
	{
		return transformer.Transform(module, context, options, error);
	}
}
