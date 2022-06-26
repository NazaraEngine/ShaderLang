// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirvPrinter.hpp>
#include <Nazara/Utils/CallOnExit.hpp>
#include <Nazara/Utils/StackArray.hpp>
#include <NZSL/SpirvData.hpp>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace nzsl
{
	struct SpirvPrinter::State
	{
		State(const Settings& s) :
		settings(s)
		{
		}

		std::size_t resultOffset;
		std::ostringstream stream;
		const Settings& settings;
	};

	std::string SpirvPrinter::Print(const std::uint32_t* codepoints, std::size_t count, const Settings& settings)
	{
		State state(settings);

		m_currentState = &state;
		Nz::CallOnExit resetOnExit([&] { m_currentState = nullptr; });

		Decode(codepoints, count);

		return m_currentState->stream.str();
	}

	bool SpirvPrinter::HandleHeader(const SpirvHeader& header)
	{
		std::uint8_t majorVersion = ((header.versionNumber) >> 16) & 0xFF;
		std::uint8_t minorVersion = ((header.versionNumber) >> 8) & 0xFF;

		m_currentState->resultOffset = std::snprintf(nullptr, 0, "%%%u = ", header.bound);

		if (m_currentState->settings.printHeader)
		{
			m_currentState->stream << "Version " + std::to_string(+majorVersion) << "." << std::to_string(+minorVersion) << "\n";
			m_currentState->stream << "Generator: " << std::to_string(header.generatorId) << "\n";
			m_currentState->stream << "Bound: " << std::to_string(header.bound) << "\n";
			m_currentState->stream << "Schema: " << std::to_string(header.schema) << "\n";
		}

		return true;
	}

	bool SpirvPrinter::HandleOpcode(const SpirvInstruction& instruction, std::uint32_t wordCount)
	{
		const std::uint32_t* startPtr = GetCurrentPtr();

		if (m_currentState->settings.printParameters)
		{
			std::ostringstream instructionStream;
			instructionStream << instruction.name;

			std::uint32_t resultId = 0;

			std::size_t currentOperand = 0;
			const std::uint32_t* endPtr = startPtr + wordCount - 1;
			while (GetCurrentPtr() < endPtr)
			{
				const SpirvInstruction::Operand* operand = &instruction.operands[currentOperand];

				if (operand->kind != SpirvOperandKind::IdResult)
				{
					PrintOperand(instructionStream, operand);
				}
				else
					resultId = ReadWord();

				if (currentOperand < instruction.minOperandCount - 1)
					currentOperand++;
			}

			if (resultId != 0)
			{
				std::string resultInfo = "%" + std::to_string(resultId) + " = ";
				m_currentState->stream << std::setw(m_currentState->resultOffset) << resultInfo;
			}
			else
				m_currentState->stream << std::string(m_currentState->resultOffset, ' ');

			m_currentState->stream << instructionStream.str();

			assert(GetCurrentPtr() == startPtr + wordCount - 1);
		}
		else
			m_currentState->stream << instruction.name;

		m_currentState->stream << "\n";

		return true;
	}
	
	void SpirvPrinter::PrintOperand(std::ostream& instructionStream, const SpirvOperand* operand)
	{
		switch (operand->kind)
		{
			case SpirvOperandKind::IdRef:
			case SpirvOperandKind::IdResultType:
			case SpirvOperandKind::IdMemorySemantics:
			case SpirvOperandKind::IdScope:
			{
				std::uint32_t value = ReadWord();
				instructionStream << " %" << value;
				break;
			}

#define NZSL_HandleOperandKind(Kind) \
			case SpirvOperandKind:: Kind : \
			{ \
				Spirv##Kind value = static_cast<Spirv##Kind>(ReadWord()); \
				instructionStream << " " #Kind "(" << ToString(value) << ")"; \
\
				/* handle extra operands */ \
				auto [operandPtr, operandCount] = GetSpirvExtraOperands(value); \
				for (std::size_t i = 0; i < operandCount; ++i) \
					PrintOperand(instructionStream, operandPtr + i); \
\
				break; \
			} \

			NZSL_HandleOperandKind(AccessQualifier)
			NZSL_HandleOperandKind(AddressingModel)
			NZSL_HandleOperandKind(BuiltIn)
			NZSL_HandleOperandKind(Capability)
			NZSL_HandleOperandKind(Decoration)
			NZSL_HandleOperandKind(Dim)
			NZSL_HandleOperandKind(ExecutionMode)
			NZSL_HandleOperandKind(ExecutionModel)
			NZSL_HandleOperandKind(FPDenormMode)
			NZSL_HandleOperandKind(FPOperationMode)
			NZSL_HandleOperandKind(FPRoundingMode)
			NZSL_HandleOperandKind(FunctionParameterAttribute)
			NZSL_HandleOperandKind(GroupOperation)
			NZSL_HandleOperandKind(ImageChannelDataType)
			NZSL_HandleOperandKind(ImageChannelOrder)
			NZSL_HandleOperandKind(ImageFormat)
			NZSL_HandleOperandKind(KernelEnqueueFlags)
			NZSL_HandleOperandKind(LinkageType)
			NZSL_HandleOperandKind(MemoryModel)
			NZSL_HandleOperandKind(OverflowModes)
			NZSL_HandleOperandKind(PackedVectorFormat)
			NZSL_HandleOperandKind(QuantizationModes)
			NZSL_HandleOperandKind(RayQueryCandidateIntersectionType)
			NZSL_HandleOperandKind(RayQueryCommittedIntersectionType)
			NZSL_HandleOperandKind(RayQueryIntersection)
			NZSL_HandleOperandKind(SamplerAddressingMode)
			NZSL_HandleOperandKind(SamplerFilterMode)
			NZSL_HandleOperandKind(Scope)
			NZSL_HandleOperandKind(SourceLanguage)
			NZSL_HandleOperandKind(StorageClass)
#undef NZSL_HandleOperandKind

			case SpirvOperandKind::ImageOperands:
			case SpirvOperandKind::FPFastMathMode:
			case SpirvOperandKind::SelectionControl:
			case SpirvOperandKind::LoopControl:
			case SpirvOperandKind::FunctionControl:
			case SpirvOperandKind::MemorySemantics:
			case SpirvOperandKind::MemoryAccess:
			case SpirvOperandKind::KernelProfilingInfo:
			case SpirvOperandKind::RayFlags:
			case SpirvOperandKind::LiteralExtInstInteger:
			case SpirvOperandKind::LiteralSpecConstantOpInteger:
			case SpirvOperandKind::LiteralContextDependentNumber: //< FIXME
			{
				std::uint32_t value = ReadWord();
				instructionStream << " " << operand->name << "(" << value << ")";
				break;
			}

			case SpirvOperandKind::LiteralInteger:
			{
				std::uint32_t value = ReadWord();
				instructionStream << " " << value;
				break;
			}

			case SpirvOperandKind::LiteralString:
			{
				std::string str = ReadString();
				instructionStream << " \"" << str << "\"";

				/*
				std::size_t offset = GetOutputOffset();

				std::size_t size4 = CountWord(str);
				for (std::size_t i = 0; i < size4; ++i)
				{
					std::uint32_t codepoint = 0;
					for (std::size_t j = 0; j < 4; ++j)
					{
						std::size_t pos = i * 4 + j;
						if (pos < str.size())
							codepoint |= std::uint32_t(str[pos]) << (j * 8);
					}

					Append(codepoint);
				}
				*/
				break;
			}


			case SpirvOperandKind::PairLiteralIntegerIdRef:
			{
				ReadWord();
				ReadWord();
				break;
			}

			case SpirvOperandKind::PairIdRefLiteralInteger:
			{
				ReadWord();
				ReadWord();
				break;
			}

			case SpirvOperandKind::PairIdRefIdRef:
			{
				ReadWord();
				ReadWord();
				break;
			}

			/*case SpirvOperandKind::LiteralContextDependentNumber:
			{
				throw std::runtime_error("not yet implemented");
			}*/

			default:
				break;
		}
	}
}
