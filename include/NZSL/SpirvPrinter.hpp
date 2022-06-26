// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVPRINTER_HPP
#define NZSL_SPIRVPRINTER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/SpirvDecoder.hpp>
#include <iosfwd>
#include <string>
#include <vector>

namespace nzsl
{
	class NZSL_API SpirvPrinter : SpirvDecoder
	{
		public:
			struct Settings;

			inline SpirvPrinter();
			SpirvPrinter(const SpirvPrinter&) = default;
			SpirvPrinter(SpirvPrinter&&) = default;
			~SpirvPrinter() = default;

			inline std::string Print(const std::vector<std::uint32_t>& codepoints);
			inline std::string Print(const std::uint32_t* codepoints, std::size_t count);
			inline std::string Print(const std::vector<std::uint32_t>& codepoints, const Settings& settings);
			std::string Print(const std::uint32_t* codepoints, std::size_t count, const Settings& settings);

			SpirvPrinter& operator=(const SpirvPrinter&) = default;
			SpirvPrinter& operator=(SpirvPrinter&&) = default;

			struct Settings
			{
				bool printHeader = true;
				bool printParameters = true;
			};

		private:
			bool HandleHeader(const SpirvHeader& header) override;
			bool HandleOpcode(const SpirvInstruction& instruction, std::uint32_t wordCount) override;
			void PrintOperand(std::ostream& instructionStream, const SpirvOperand* operand);

			enum class ExtensionSet
			{
				GLSLstd450
			};

			struct State;
			State* m_currentState;
	};
}

#include <NZSL/SpirvPrinter.inl>

#endif // NZSL_SPIRVPRINTER_HPP
