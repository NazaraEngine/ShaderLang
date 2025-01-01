// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl
{
	inline const std::uint32_t* SpirvDecoder::GetCurrentPtr() const
	{
		return m_currentCodepoint;
	}

	inline void SpirvDecoder::ResetPtr(const std::uint32_t* codepoint)
	{
		m_currentCodepoint = codepoint;
	}
}
