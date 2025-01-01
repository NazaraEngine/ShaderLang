// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirV/SpirvSectionBase.hpp>
#include <NazaraUtils/Endianness.hpp>

namespace nzsl
{
	std::size_t SpirvSectionBase::AppendRaw(const Raw& raw)
	{
		std::size_t offset = GetOutputOffset();

		const std::uint8_t* ptr = static_cast<const std::uint8_t*>(raw.ptr);

		std::size_t size4 = CountWord(raw);
		for (std::size_t i = 0; i < size4; ++i)
		{
			std::uint32_t codepoint = 0;
			for (std::size_t j = 0; j < 4; ++j)
			{
#ifdef NZSL_BIG_ENDIAN
				std::size_t pos = i * 4 + (3 - j);
#else
				std::size_t pos = i * 4 + j;
#endif

				if (pos < raw.size)
					codepoint |= std::uint32_t(ptr[pos]) << (j * 8);
			}

			AppendRaw(codepoint);
		}

		return offset;
	}
}
