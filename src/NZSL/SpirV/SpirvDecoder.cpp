// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirV/SpirvDecoder.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackArray.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace nzsl
{
	void SpirvDecoder::Decode(const std::uint32_t* codepoints, std::size_t count)
	{
		m_currentCodepoint = codepoints;
		m_codepointEnd = codepoints + count;

		std::uint32_t magicNumber = ReadWord();
		if (magicNumber != SpirvMagicNumber)
			throw std::runtime_error("invalid SPIR-V: magic number didn't match");

		std::uint32_t versionNumber = ReadWord();
		if (versionNumber > SpirvVersion)
			throw std::runtime_error("SPIR-V is more recent than decoder, dismissing");

		SpirvHeader header;
		header.generatorId = ReadWord();
		header.bound = ReadWord();
		header.schema = ReadWord();
		header.versionNumber = versionNumber;

		if (!HandleHeader(header))
			return;

		while (m_currentCodepoint < m_codepointEnd)
		{
			const std::uint32_t* instructionBegin = m_currentCodepoint;

			std::uint32_t firstWord = ReadWord();

			std::uint16_t wordCount = static_cast<std::uint16_t>((firstWord >> 16) & 0xFFFF);
			std::uint16_t opcode = static_cast<std::uint16_t>(firstWord & 0xFFFF);

			const SpirvInstruction* inst = GetSpirvInstruction(opcode);
			if (!inst)
				throw std::runtime_error("invalid instruction");

			if (!HandleOpcode(*inst, wordCount))
				break;

			m_currentCodepoint = instructionBegin + wordCount;
		}
	}

	bool SpirvDecoder::HandleHeader(const SpirvHeader& /*header*/)
	{
		return true;
	}

	std::string SpirvDecoder::ReadString()
	{
		std::string str;

		for (;;)
		{
			std::uint32_t value = ReadWord();
			for (std::size_t j = 0; j < 4; ++j)
			{
				char c = static_cast<char>((value >> (j * 8)) & 0xFF);
				if (c == '\0')
					return str;

				str.push_back(c);
			}
		}
	}

	std::uint32_t SpirvDecoder::ReadWord()
	{
		if (m_currentCodepoint >= m_codepointEnd)
			throw std::runtime_error("unexpected end of stream");

		return *m_currentCodepoint++;
	}
}
