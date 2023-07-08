// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRV_SPIRVSECTIONBASE_HPP
#define NZSL_SPIRV_SPIRVSECTIONBASE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <string>
#include <vector>

namespace nzsl
{
	class NZSL_API SpirvSectionBase
	{
		public:
			struct OpSize;
			struct Raw;

			SpirvSectionBase() = default;
			SpirvSectionBase(const SpirvSectionBase&) = default;
			SpirvSectionBase(SpirvSectionBase&&) = default;
			~SpirvSectionBase() = default;

			inline const std::vector<std::uint32_t>& GetBytecode() const;
			inline std::size_t GetOutputOffset() const;

			SpirvSectionBase& operator=(const SpirvSectionBase&) = delete;
			SpirvSectionBase& operator=(SpirvSectionBase&&) = default;

			struct OpSize
			{
				unsigned int wc;
			};

			struct Raw
			{
				const void* ptr;
				std::size_t size;
			};

			static inline std::uint32_t BuildOpcode(SpirvOp opcode, unsigned int wordCount);

			static constexpr std::size_t MaxWordCount = 0xFFFF;

		protected:
			inline std::size_t Append(SpirvOp opcode, const OpSize& wordCount);
			template<typename... Args> std::size_t Append(SpirvOp opcode, const Args&... args);
			template<typename F> std::size_t AppendVariadic(SpirvOp opcode, F&& callback);
			inline std::size_t AppendRaw(const char* str);
			inline std::size_t AppendRaw(std::string_view str);
			inline std::size_t AppendRaw(const std::string& str);
			inline std::size_t AppendRaw(std::uint32_t value);
			std::size_t AppendRaw(const Raw& raw);
			inline std::size_t AppendRaw(std::initializer_list<std::uint32_t> codepoints);
			inline std::size_t AppendSection(const SpirvSectionBase& section);
			template<typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_enum_v<T>>> std::size_t AppendRaw(T value);

			inline unsigned int CountWord(const char* str);
			inline unsigned int CountWord(std::string_view str);
			inline unsigned int CountWord(const std::string& str);
			inline unsigned int CountWord(const Raw& raw);
			template<typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_enum_v<T>>> unsigned int CountWord(const T& value);
			template<typename T1, typename T2, typename... Args> unsigned int CountWord(const T1& value, const T2& value2, const Args&... rest);

		private:
			std::vector<std::uint32_t> m_bytecode;
	};
}

#include <NZSL/SpirV/SpirvSectionBase.inl>

#endif // NZSL_SPIRV_SPIRVSECTIONBASE_HPP
