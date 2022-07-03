// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVBLOCK_HPP
#define NZSL_SPIRVBLOCK_HPP

#include <NZSL/Config.hpp>
#include <NZSL/SpirV/SpirvSectionBase.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <string>
#include <vector>

namespace nzsl
{
	class NZSL_API SpirvBlock : public SpirvSectionBase
	{
		public:
			inline SpirvBlock(SpirvWriter& writer);
			SpirvBlock(const SpirvBlock&) = default;
			SpirvBlock(SpirvBlock&&) = default;
			~SpirvBlock() = default;

			inline std::size_t Append(SpirvOp opcode, const OpSize& wordCount);
			template<typename... Args> std::size_t Append(SpirvOp opcode, Args&&... args);
			template<typename F> std::size_t AppendVariadic(SpirvOp opcode, F&& callback);

			inline std::uint32_t GetLabelId() const;

			inline bool IsTerminated() const;

			SpirvBlock& operator=(const SpirvBlock&) = delete;
			SpirvBlock& operator=(SpirvBlock&&) = default;

			static inline bool IsTerminationInstruction(SpirvOp op);

		private:
			inline void HandleSpirvOp(SpirvOp op);

			std::uint32_t m_labelId;
			bool m_isTerminated;
	};
}

#include <NZSL/SpirV/SpirvBlock.inl>

#endif // NZSL_SPIRVBLOCK_HPP
