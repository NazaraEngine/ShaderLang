// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/Endianness.hpp>

namespace nzsl
{
	inline std::size_t SpirvSectionBase::Append(SpirvOp opcode, const OpSize& wordCount)
	{
		return AppendRaw(BuildOpcode(opcode, wordCount.wc));
	}

	template<typename... Args>
	std::size_t SpirvSectionBase::Append(SpirvOp opcode, const Args&... args)
	{
		unsigned int wordCount = 1 + (CountWord(args) + ... + 0);
		std::size_t offset = Append(opcode, OpSize{ wordCount });
		if constexpr (sizeof...(args) > 0)
			(AppendRaw(args), ...);

		return offset;
	}

	template<typename F> std::size_t SpirvSectionBase::AppendVariadic(SpirvOp opcode, F&& callback)
	{
		std::size_t offset = AppendRaw(0); //< Will be filled later

		unsigned int wordCount = 1;
		auto appendFunctor = [&](const auto& value)
		{
			wordCount += CountWord(value);
			AppendRaw(value);
		};
		callback(appendFunctor);

		std::uint32_t bytecode = BuildOpcode(opcode, wordCount);

#ifdef NAZARA_BIG_ENDIAN
		// SPIR-V is little endian
		bytecode = Nz::SwapBytes(bytecode);
#endif

		m_bytecode[offset] = bytecode;

		return offset;
	}

	inline std::size_t SpirvSectionBase::AppendRaw(const char* str)
	{
		return AppendRaw(std::string_view(str));
	}

	inline std::size_t SpirvSectionBase::AppendRaw(std::string_view str)
	{
		std::size_t offset = GetOutputOffset();

		std::size_t wordCount = CountWord(str);
		for (std::size_t i = 0; i < wordCount; ++i)
		{
			std::uint32_t codepoint = 0;
			for (std::size_t j = 0; j < 4; ++j)
			{
				std::size_t pos = i * 4 + j;
				if (pos >= str.size())
					break;

				codepoint |= std::uint32_t(str[pos]) << (j * 8);
			}

			AppendRaw(codepoint);
		}

		return offset;
	}

	inline std::size_t SpirvSectionBase::AppendRaw(const std::string& str)
	{
		return AppendRaw(std::string_view(str));
	}

	inline std::size_t SpirvSectionBase::AppendRaw(std::uint32_t value)
	{
#ifdef NAZARA_BIG_ENDIAN
		// SPIR-V is little endian
		value = Nz::SwapBytes(value);
#endif

		std::size_t offset = GetOutputOffset();
		m_bytecode.push_back(value);

		return offset;
	}

	inline std::size_t SpirvSectionBase::AppendRaw(std::initializer_list<std::uint32_t> codepoints)
	{
		std::size_t offset = GetOutputOffset();

		for (std::uint32_t cp : codepoints)
			AppendRaw(cp);

		return offset;
	}

	inline std::size_t SpirvSectionBase::AppendSection(const SpirvSectionBase& section)
	{
		const std::vector<std::uint32_t>& bytecode = section.GetBytecode();

		std::size_t offset = GetOutputOffset();
		m_bytecode.resize(offset + bytecode.size());
		std::copy(bytecode.begin(), bytecode.end(), m_bytecode.begin() + offset);

		return offset;
	}

	template<typename T, typename>
	std::size_t SpirvSectionBase::AppendRaw(T value)
	{
		return AppendRaw(static_cast<std::uint32_t>(value));
	}

	template<typename T, typename>
	unsigned int SpirvSectionBase::CountWord(const T& /*value*/)
	{
		return 1;
	}

	template<typename T1, typename T2, typename ...Args>
	unsigned int SpirvSectionBase::CountWord(const T1& value, const T2& value2, const Args&... rest)
	{
		return CountWord(value) + CountWord(value2) + (CountWord(rest) + ...);
	}

	inline unsigned int SpirvSectionBase::CountWord(const char* str)
	{
		return CountWord(std::string_view(str));
	}

	inline unsigned int SpirvSectionBase::CountWord(const std::string& str)
	{
		return CountWord(std::string_view(str));
	}

	inline unsigned int SpirvSectionBase::CountWord(const Raw& raw)
	{
		return static_cast<unsigned int>((raw.size + sizeof(std::uint32_t) - 1) / sizeof(std::uint32_t));
	}

	inline unsigned int SpirvSectionBase::CountWord(std::string_view str)
	{
		return (static_cast<unsigned int>(str.size() + 1) + sizeof(std::uint32_t) - 1) / sizeof(std::uint32_t); //< + 1 for null character
	}

	inline const std::vector<std::uint32_t>& SpirvSectionBase::GetBytecode() const
	{
		return m_bytecode;
	}

	inline std::size_t SpirvSectionBase::GetOutputOffset() const
	{
		return m_bytecode.size();
	}

	inline std::uint32_t SpirvSectionBase::BuildOpcode(SpirvOp opcode, unsigned int wordCount)
	{
		return std::uint32_t(opcode) | std::uint32_t(wordCount) << 16;
	}
}
