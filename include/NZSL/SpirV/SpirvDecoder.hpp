// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVDECODER_HPP
#define NZSL_SPIRVDECODER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <functional>
#include <string>
#include <vector>

namespace nzsl
{
	class NZSL_API SpirvDecoder
	{
		public:
			SpirvDecoder() = default;
			SpirvDecoder(const SpirvDecoder&) = default;
			SpirvDecoder(SpirvDecoder&&) = default;
			~SpirvDecoder() = default;

			void Decode(const std::uint32_t* codepoints, std::size_t count);

			SpirvDecoder& operator=(const SpirvDecoder&) = default;
			SpirvDecoder& operator=(SpirvDecoder&&) = default;

		protected:
			struct SpirvHeader;

			inline const std::uint32_t* GetCurrentPtr() const;

			virtual bool HandleHeader(const SpirvHeader& header);
			virtual bool HandleOpcode(const SpirvInstruction& instruction, std::uint32_t wordCount) = 0;

			std::string ReadString();
			std::uint32_t ReadWord();

			inline void ResetPtr(const std::uint32_t* codepoint);

			struct SpirvHeader
			{
				std::uint32_t generatorId;
				std::uint32_t bound;
				std::uint32_t schema;
				std::uint32_t versionNumber;
			};

		private:
			const std::uint32_t* m_currentCodepoint;
			const std::uint32_t* m_codepointEnd;
	};
}

#include <NZSL/SpirV/SpirvDecoder.inl>

#endif // NZSL_SPIRVDECODER_HPP
