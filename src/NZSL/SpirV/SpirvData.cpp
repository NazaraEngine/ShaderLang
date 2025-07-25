// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

// this file was automatically generated and should not be edited

#include <NZSL/SpirV/SpirvData.hpp>
#include <algorithm>
#include <array>
#include <cassert>

namespace nzsl
{
	static constexpr std::array<SpirvOperand, 1420> s_operands = {
		{
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Continued Source')"
			},
			{
				SpirvOperandKind::SourceLanguage,
				R"(SourceLanguage)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Version')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('File')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Source')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Extension')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Name')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Member')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Name')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('String')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('File')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Line')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Column')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Name')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Set')"
			},
			{
				SpirvOperandKind::LiteralExtInstInteger,
				R"('Instruction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Operand 1', +
'Operand 2', +
...)"
			},
			{
				SpirvOperandKind::AddressingModel,
				R"(AddressingModel)"
			},
			{
				SpirvOperandKind::MemoryModel,
				R"(MemoryModel)"
			},
			{
				SpirvOperandKind::ExecutionModel,
				R"(ExecutionModel)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Entry Point')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Name')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Interface')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Entry Point')"
			},
			{
				SpirvOperandKind::ExecutionMode,
				R"('Mode')"
			},
			{
				SpirvOperandKind::Capability,
				R"('Capability')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Width')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Signedness')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Width')"
			},
			{
				SpirvOperandKind::FPEncoding,
				R"('Floating Point Encoding')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Component Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Component Count')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Column Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Column Count')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sampled Type')"
			},
			{
				SpirvOperandKind::Dim,
				R"(Dim)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Depth')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Arrayed')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('MS')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Sampled')"
			},
			{
				SpirvOperandKind::ImageFormat,
				R"(ImageFormat)"
			},
			{
				SpirvOperandKind::AccessQualifier,
				R"(AccessQualifier)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image Type')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Element Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Length')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Member 0 type', +
'member 1 type', +
...)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralString,
				R"(The name of the opaque type.)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::StorageClass,
				R"(StorageClass)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Type')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Return Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Parameter 0 Type', +
'Parameter 1 Type', +
...)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::AccessQualifier,
				R"('Qualifier')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer Type')"
			},
			{
				SpirvOperandKind::StorageClass,
				R"(StorageClass)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralContextDependentNumber,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Constituents')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::SamplerAddressingMode,
				R"(SamplerAddressingMode)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Param')"
			},
			{
				SpirvOperandKind::SamplerFilterMode,
				R"(SamplerFilterMode)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralSpecConstantOpInteger,
				R"('Opcode')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::FunctionControl,
				R"(FunctionControl)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Function Type')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Function')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Argument 0', +
'Argument 1', +
...)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::StorageClass,
				R"(StorageClass)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Initializer')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sample')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Object')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Source')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Source')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Size')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Indexes')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Element')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Indexes')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Structure')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Array member')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target')"
			},
			{
				SpirvOperandKind::Decoration,
				R"(Decoration)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Structure Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Member')"
			},
			{
				SpirvOperandKind::Decoration,
				R"(Decoration)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Decoration Group')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Targets')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Decoration Group')"
			},
			{
				SpirvOperandKind::PairIdRefLiteralInteger,
				R"('Targets')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Index')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Component')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Index')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector 1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector 2')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Components')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Composite')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Indexes')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Composite')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Indexes')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Operand')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Matrix')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sampler')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sampled Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::ImageOperands,
				R"(ImageOperands)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sampled Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('D~ref~')"
			},
			{
				SpirvOperandKind::ImageOperands,
				R"(ImageOperands)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::ImageOperands,
				R"(ImageOperands)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sampled Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Component')"
			},
			{
				SpirvOperandKind::ImageOperands,
				R"(ImageOperands)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Texel')"
			},
			{
				SpirvOperandKind::ImageOperands,
				R"(ImageOperands)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Level of Detail')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Float Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Signed Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Unsigned Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Integer Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::StorageClass,
				R"('Storage')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Operand 1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Operand 2')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Scalar')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Matrix')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Scalar')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Matrix')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Matrix')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('LeftMatrix')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RightMatrix')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('y')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Condition')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Object 1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Object 2')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Shift')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Insert')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Count')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Count')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('P')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stream')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Memory')"
			},
			{
				SpirvOperandKind::IdMemorySemantics,
				R"('Semantics')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Memory')"
			},
			{
				SpirvOperandKind::IdMemorySemantics,
				R"('Semantics')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Memory')"
			},
			{
				SpirvOperandKind::IdMemorySemantics,
				R"('Semantics')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Memory')"
			},
			{
				SpirvOperandKind::IdMemorySemantics,
				R"('Semantics')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Memory')"
			},
			{
				SpirvOperandKind::IdMemorySemantics,
				R"('Equal')"
			},
			{
				SpirvOperandKind::IdMemorySemantics,
				R"('Unequal')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Comparator')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::PairIdRefIdRef,
				R"('Variable, Parent, ...')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Merge Block')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Continue Target')"
			},
			{
				SpirvOperandKind::LoopControl,
				R"(LoopControl)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Merge Block')"
			},
			{
				SpirvOperandKind::SelectionControl,
				R"(SelectionControl)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target Label')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Condition')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('True Label')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('False Label')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Branch weights')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Selector')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Default')"
			},
			{
				SpirvOperandKind::PairLiteralIntegerIdRef,
				R"('Target')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Size')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Destination')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Source')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Num Elements')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Event')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Num Events')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Events List')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Predicate')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('LocalId')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::GroupOperation,
				R"('Operation')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('X')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reserve Id')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Num Packets')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reserve Id')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reserve Id')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Num Packets')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reserve Id')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Queue')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Num Events')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Wait Events')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ret Event')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Queue')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Flags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ND Range')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Num Events')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Wait Events')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ret Event')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Invoke')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Align')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Local Size')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ND Range')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Invoke')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Align')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Invoke')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Align')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Event')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Event')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Status')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Event')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Profiling Info')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('GlobalWorkSize')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('LocalWorkSize')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('GlobalWorkOffset')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Resident Code')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Capacity')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pipe Storage')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Subgroup Count')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Invoke')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Param Align')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Named Barrier')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Memory')"
			},
			{
				SpirvOperandKind::IdMemorySemantics,
				R"('Semantics')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Process')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Id')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Index')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::GroupOperation,
				R"('Operation')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Mask')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Delta')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::GroupOperation,
				R"('Operation')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ClusterSize')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Attachment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sample')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sample')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::StorageClass,
				R"(StorageClass)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Data Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Initializer')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Indexes')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Predicate')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Element')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Indexes')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Structure')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Array member')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Num Bytes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RW')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Locality')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cache Type')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Delta')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ClusterSize')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Flags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cull Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Miss Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Callable Data')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accel')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector 1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector 2')"
			},
			{
				SpirvOperandKind::PackedVectorFormat,
				R"('Packed Vector Format')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector 1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vector 2')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accumulator')"
			},
			{
				SpirvOperandKind::PackedVectorFormat,
				R"('Packed Vector Format')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Component Type')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Scope')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Rows')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Columns')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Use')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('MemoryLayout')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stride')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"('Memory Operand')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('MemoryLayout')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stride')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"('Memory Operand')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('A')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('B')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('C')"
			},
			{
				SpirvOperandKind::CooperativeMatrixOperands,
				R"('Cooperative Matrix Operands')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayQuery')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayFlags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('CullMask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayOrigin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayTMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayDirection')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayTMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayQuery')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('HitT')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayQuery')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayQuery')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Intersection')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Texture')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinates')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Weights')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Texture')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinates')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Box Size')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target Coordinates')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reference')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reference Coordinates')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Block Size')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target Sampled Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target Coordinates')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reference Sampled Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reference Coordinates')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Block Size')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Fragment Index')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Scope')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Visibility')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload Count')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Node Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload Array')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload Type')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload Array')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Node Index')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Literal String')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Acceleration Structure')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('InstanceId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PrimitiveId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('GeometryIndex')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Kind')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Current Time')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('HitObject Attributes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Acceleration Structure')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('InstanceId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PrimitiveId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('GeometryIndex')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Kind')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Current Time')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('HitObject Attributes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Current Time')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Acceleration Structure')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayFlags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cullmask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Miss Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Time')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Acceleration Structure')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('RayFlags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cullmask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Miss Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Acceleration Structure')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('InstanceId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PrimitiveId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('GeometryIndex')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Kind')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('HitObject Attributes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Acceleration Structure')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('InstanceId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PrimitiveId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('GeometryIndex')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Kind')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Record Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TMax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('HitObject Attributes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object Attribute')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hint')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Bits')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sampled Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Granularity')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coarse')"
			},
			{
				SpirvOperandKind::ImageOperands,
				R"(ImageOperands)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Group Count X')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Group Count Y')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Group Count Z')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Vertex Count')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Primitive Count')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Index Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Indices')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Instance Id')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Geometry Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Primitive Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Barycentric')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Hit')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('HitKind')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Flags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cull Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Miss Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PayloadId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Flags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cull Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Miss Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Time')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PayloadId')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Accel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Flags')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cull Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Miss Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Origin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmin')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ray Tmax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Time')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SBT Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Callable DataId')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Component Type')"
			},
			{
				SpirvOperandKind::IdScope,
				R"('Execution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Rows')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Columns')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Column Major')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Column Major')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Matrix')"
			},
			{
				SpirvOperandKind::CooperativeMatrixReduce,
				R"('Reduce')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('CombineFunc')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pointer')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Object')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorLayout')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"('Memory Operand')"
			},
			{
				SpirvOperandKind::TensorAddressingOperands,
				R"('Tensor Addressing Operands')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Matrix')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Func')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Operands')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Dim')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ClampMode')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Dim')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('HasDimensions')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('p')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorLayout')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Dim')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorLayout')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stride')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorLayout')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Operands')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorLayout')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorView')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Dim')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorView')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Stride')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorView')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ClipRowOffset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ClipRowSpan')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ClipColOffset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ClipColSpan')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('TensorLayout')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('BlockSize')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Bit Width')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Byte stride')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Element index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Byte offset')"
			},
			{
				SpirvOperandKind::RawAccessChainOperands,
				R"(RawAccessChainOperands)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Data')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('InvocationId')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Current')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Next')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Delta')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Previous')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Current')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Delta')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Data')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ptr')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ptr')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Data')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Data')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Width')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Height')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Coordinate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Width')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Height')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Data')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Asm target')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Asm type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Asm instructions')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Constraints')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Asm')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Argument 0')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Value')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('ExpectedValue')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Struct Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Member')"
			},
			{
				SpirvOperandKind::Decoration,
				R"(Decoration)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sampler')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Slice Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Qp')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reference Base Penalty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Shape Penalty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction Cost')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Cost Center Delta')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Cost Table')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Cost Precision')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Source Field Polarity')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Reference Field Polarity')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Forward Reference Field Polarity')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Backward Reference Field Polarity')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Reference Ids')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Reference Parameter Field Polarities')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Coord')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Partition Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('SAD Adjustment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ref Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Search Window Config')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Fwd Ref Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Bwd Ref Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('id> Search Window Config')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Search Window Config')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Dual Ref')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ref Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Coord')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ref Window Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image Size')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Max Motion Vector Count')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Threshold')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Sad Weights')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ref Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Fwd Ref Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Bwd Ref Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ref Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Streamin Components')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Fwd Ref Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Bwd Ref Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Streamin Components')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Major Shape')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Major Shape')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Image Select')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Coord')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Motion Vectors')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Major Shapes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Minor Shapes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pixel Resolution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sad Adjustment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Coord')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Motion Vectors')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Major Shapes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Minor Shapes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Pixel Resolution')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Bidirectional Weight')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sad Adjustment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Reference Ids')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Reference Ids')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Reference Field Polarities')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Skip Block Partition Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Skip Motion Vector Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Motion Vectors')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Bidirectional Weight')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sad Adjustment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Luma Intra Partition Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Intra Neighbour Availabilty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Left Edge Luma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Left Corner Luma Pixel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Edge Luma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Right Edge Luma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sad Adjustment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Luma Intra Partition Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Intra Neighbour Availabilty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Left Edge Luma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Left Corner Luma Pixel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Edge Luma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Right Edge Luma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Left Edge Chroma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Left Corner Chroma Pixel')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Upper Edge Chroma Pixels')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Sad Adjustment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Skip Block Partition Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Direction')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Luma Mode Penalty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Luma Packed Neighbor Modes')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Luma Packed Non Dc Penalty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Chroma Mode Base Penalty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packed Sad Coefficients')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Block Based Skip Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Src Image')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Payload')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Lenght')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('A')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('M1')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Mout')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('FromSign')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('EnableSubnormals')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingMode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingAccuracy')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('A')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('M1')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Mout')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('EnableSubnormals')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingMode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingAccuracy')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('A')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Mout')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('FromSign')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('EnableSubnormals')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingMode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingAccuracy')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('A')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('M1')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('EnableSubnormals')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingMode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingAccuracy')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('A')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('M1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('B')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('M2')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Mout')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('EnableSubnormals')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingMode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingAccuracy')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('A')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('M1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('B')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Mout')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('EnableSubnormals')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingMode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('RoundingAccuracy')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Loop Control Parameters')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Name')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Alias Domain')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Name')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('AliasScope1, AliasScope2, ...')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Input Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Input')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('S')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('I')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('rI')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Q')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('O')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Packet Alignment')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Result')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Input')"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::AccessQualifier,
				R"('AccessQualifier')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('BFloat16 Value')"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target ')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Ptr')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('NumBytes')"
			},
			{
				SpirvOperandKind::MemoryAccess,
				R"(MemoryAccess)"
			},
			{
				SpirvOperandKind::IdResultType,
				R"(IdResultType)"
			},
			{
				SpirvOperandKind::IdResult,
				R"(IdResult)"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PtrVector')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Alignment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('FillEmpty')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('InputVector')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('PtrVector')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Alignment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Mask')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('degrees')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('radians')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('y_over_x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('y')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('i')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('minVal')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('maxVal')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('y')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('a')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('edge')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('edge0')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('edge1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('a')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('b')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('c')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('exp')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('v')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('p0')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('p1')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('N')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('I')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Nref')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('I')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('N')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('I')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('N')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('eta')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('interpolant')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('interpolant')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('sample')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('interpolant')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('offset')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Number of <<Invocation,invocations>>')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('x size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('y size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('z size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Vertex count')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Vector type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Subgroup Size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Subgroups Per Workgroup')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Subgroups Per Workgroup')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('y size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('z size')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('x size hint')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('y size hint')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('z size hint')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Target Width')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Is Entry')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Number of recursions')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Shader Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Node Name')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Shader Index')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Primitive count')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('max_x_size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('max_y_size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('max_z_size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('max_dimensions')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('vector_width')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('target_fmax')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Target Type')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Fast-Math Mode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('StallFreeReturn')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('WaitForDoneWrite')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Barrier Count')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Number of Registers')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Number of Registers')"
			},
			{
				SpirvOperandKind::NamedMaximumNumberOfRegisters,
				R"('Named Maximum Number of Registers')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Specialization Constant ID')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Array Stride')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Matrix Stride')"
			},
			{
				SpirvOperandKind::BuiltIn,
				R"(BuiltIn)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Stream Number')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Location')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Component')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Index')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Binding Point')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Descriptor Set')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Byte Offset')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('XFB Buffer Number')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('XFB Stride')"
			},
			{
				SpirvOperandKind::FunctionParameterAttribute,
				R"('Function Parameter Attribute')"
			},
			{
				SpirvOperandKind::FPRoundingMode,
				R"('Floating-Point Rounding Mode')"
			},
			{
				SpirvOperandKind::FPFastMathMode,
				R"('Fast-Math Mode')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Name')"
			},
			{
				SpirvOperandKind::LinkageType,
				R"('Linkage Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Attachment Index')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Max Byte Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Alignment')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Max Byte Offset')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Max number of payloads')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Base Index')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Array Size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Offset')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('N')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Register')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Kind')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Counter Buffer')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Semantic')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('User Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Target Width')"
			},
			{
				SpirvOperandKind::FPRoundingMode,
				R"('FP Rounding Mode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Target Width')"
			},
			{
				SpirvOperandKind::FPDenormMode,
				R"('FP Denorm Mode')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Memory Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Banks')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Bank Width')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Maximum Copies')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Maximum Replicates')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Merge Key')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Merge Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Bank Bits')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Force Key')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Stride Size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Word Size')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Cache Size in bytes')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Prefetcher Size in bytes')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Mode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Propagate')"
			},
			{
				SpirvOperandKind::IdRef,
				R"('Aliasing Scopes List')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Cycles')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Invocations')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Enable')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Buffer Location ID')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('IO Pipe ID')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Target Width')"
			},
			{
				SpirvOperandKind::FPOperationMode,
				R"('FP Operation Mode')"
			},
			{
				SpirvOperandKind::LiteralFloat,
				R"('Max Error')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Latency Label')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Relative To')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Control Type')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Relative Cycle')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('AddressWidth')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('DataWidth')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Latency')"
			},
			{
				SpirvOperandKind::AccessQualifier,
				R"('ReadWriteMode')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('MaxBurstCount')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Waitrequest')"
			},
			{
				SpirvOperandKind::HostAccessQualifier,
				R"('Access')"
			},
			{
				SpirvOperandKind::LiteralString,
				R"('Name')"
			},
			{
				SpirvOperandKind::InitializationModeQualifier,
				R"('Trigger')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"(Value)"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Cache Level')"
			},
			{
				SpirvOperandKind::LoadCacheControl,
				R"('Cache Control')"
			},
			{
				SpirvOperandKind::LiteralInteger,
				R"('Cache Level')"
			},
			{
				SpirvOperandKind::StoreCacheControl,
				R"('Cache Control')"
			},
		}
	};

	static std::array<SpirvInstruction, 759> s_instructions = {
		{
			{
				SpirvOp::OpNop,
				R"(OpNop)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpUndef,
				R"(OpUndef)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSourceContinued,
				R"(OpSourceContinued)",
				&s_operands[2],
				nullptr,
				1,
			},
			{
				SpirvOp::OpSource,
				R"(OpSource)",
				&s_operands[3],
				nullptr,
				4,
			},
			{
				SpirvOp::OpSourceExtension,
				R"(OpSourceExtension)",
				&s_operands[7],
				nullptr,
				1,
			},
			{
				SpirvOp::OpName,
				R"(OpName)",
				&s_operands[8],
				nullptr,
				2,
			},
			{
				SpirvOp::OpMemberName,
				R"(OpMemberName)",
				&s_operands[10],
				nullptr,
				3,
			},
			{
				SpirvOp::OpString,
				R"(OpString)",
				&s_operands[13],
				&s_operands[13],
				2,
			},
			{
				SpirvOp::OpLine,
				R"(OpLine)",
				&s_operands[15],
				nullptr,
				3,
			},
			{
				SpirvOp::OpExtension,
				R"(OpExtension)",
				&s_operands[9],
				nullptr,
				1,
			},
			{
				SpirvOp::OpExtInstImport,
				R"(OpExtInstImport)",
				&s_operands[18],
				&s_operands[18],
				2,
			},
			{
				SpirvOp::OpExtInst,
				R"(OpExtInst)",
				&s_operands[20],
				&s_operands[21],
				5,
			},
			{
				SpirvOp::OpMemoryModel,
				R"(OpMemoryModel)",
				&s_operands[25],
				nullptr,
				2,
			},
			{
				SpirvOp::OpEntryPoint,
				R"(OpEntryPoint)",
				&s_operands[27],
				nullptr,
				4,
			},
			{
				SpirvOp::OpExecutionMode,
				R"(OpExecutionMode)",
				&s_operands[31],
				nullptr,
				2,
			},
			{
				SpirvOp::OpCapability,
				R"(OpCapability)",
				&s_operands[33],
				nullptr,
				1,
			},
			{
				SpirvOp::OpTypeVoid,
				R"(OpTypeVoid)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeBool,
				R"(OpTypeBool)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeInt,
				R"(OpTypeInt)",
				&s_operands[34],
				&s_operands[34],
				3,
			},
			{
				SpirvOp::OpTypeFloat,
				R"(OpTypeFloat)",
				&s_operands[37],
				&s_operands[37],
				3,
			},
			{
				SpirvOp::OpTypeVector,
				R"(OpTypeVector)",
				&s_operands[40],
				&s_operands[40],
				3,
			},
			{
				SpirvOp::OpTypeMatrix,
				R"(OpTypeMatrix)",
				&s_operands[43],
				&s_operands[43],
				3,
			},
			{
				SpirvOp::OpTypeImage,
				R"(OpTypeImage)",
				&s_operands[46],
				&s_operands[46],
				9,
			},
			{
				SpirvOp::OpTypeSampler,
				R"(OpTypeSampler)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeSampledImage,
				R"(OpTypeSampledImage)",
				&s_operands[55],
				&s_operands[55],
				2,
			},
			{
				SpirvOp::OpTypeArray,
				R"(OpTypeArray)",
				&s_operands[57],
				&s_operands[57],
				3,
			},
			{
				SpirvOp::OpTypeRuntimeArray,
				R"(OpTypeRuntimeArray)",
				&s_operands[57],
				&s_operands[57],
				2,
			},
			{
				SpirvOp::OpTypeStruct,
				R"(OpTypeStruct)",
				&s_operands[60],
				&s_operands[60],
				2,
			},
			{
				SpirvOp::OpTypeOpaque,
				R"(OpTypeOpaque)",
				&s_operands[62],
				&s_operands[62],
				2,
			},
			{
				SpirvOp::OpTypePointer,
				R"(OpTypePointer)",
				&s_operands[64],
				&s_operands[64],
				3,
			},
			{
				SpirvOp::OpTypeFunction,
				R"(OpTypeFunction)",
				&s_operands[67],
				&s_operands[67],
				3,
			},
			{
				SpirvOp::OpTypeEvent,
				R"(OpTypeEvent)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeDeviceEvent,
				R"(OpTypeDeviceEvent)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeReserveId,
				R"(OpTypeReserveId)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeQueue,
				R"(OpTypeQueue)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypePipe,
				R"(OpTypePipe)",
				&s_operands[70],
				&s_operands[70],
				2,
			},
			{
				SpirvOp::OpTypeForwardPointer,
				R"(OpTypeForwardPointer)",
				&s_operands[72],
				nullptr,
				2,
			},
			{
				SpirvOp::OpConstantTrue,
				R"(OpConstantTrue)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpConstantFalse,
				R"(OpConstantFalse)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpConstant,
				R"(OpConstant)",
				&s_operands[74],
				&s_operands[75],
				3,
			},
			{
				SpirvOp::OpConstantComposite,
				R"(OpConstantComposite)",
				&s_operands[77],
				&s_operands[78],
				3,
			},
			{
				SpirvOp::OpConstantSampler,
				R"(OpConstantSampler)",
				&s_operands[80],
				&s_operands[81],
				5,
			},
			{
				SpirvOp::OpConstantNull,
				R"(OpConstantNull)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSpecConstantTrue,
				R"(OpSpecConstantTrue)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSpecConstantFalse,
				R"(OpSpecConstantFalse)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSpecConstant,
				R"(OpSpecConstant)",
				&s_operands[74],
				&s_operands[75],
				3,
			},
			{
				SpirvOp::OpSpecConstantComposite,
				R"(OpSpecConstantComposite)",
				&s_operands[77],
				&s_operands[78],
				3,
			},
			{
				SpirvOp::OpSpecConstantOp,
				R"(OpSpecConstantOp)",
				&s_operands[85],
				&s_operands[86],
				3,
			},
			{
				SpirvOp::OpFunction,
				R"(OpFunction)",
				&s_operands[88],
				&s_operands[89],
				4,
			},
			{
				SpirvOp::OpFunctionParameter,
				R"(OpFunctionParameter)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpFunctionEnd,
				R"(OpFunctionEnd)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpFunctionCall,
				R"(OpFunctionCall)",
				&s_operands[92],
				&s_operands[93],
				4,
			},
			{
				SpirvOp::OpVariable,
				R"(OpVariable)",
				&s_operands[96],
				&s_operands[97],
				4,
			},
			{
				SpirvOp::OpImageTexelPointer,
				R"(OpImageTexelPointer)",
				&s_operands[100],
				&s_operands[101],
				5,
			},
			{
				SpirvOp::OpLoad,
				R"(OpLoad)",
				&s_operands[105],
				&s_operands[106],
				4,
			},
			{
				SpirvOp::OpStore,
				R"(OpStore)",
				&s_operands[109],
				nullptr,
				3,
			},
			{
				SpirvOp::OpCopyMemory,
				R"(OpCopyMemory)",
				&s_operands[112],
				nullptr,
				4,
			},
			{
				SpirvOp::OpCopyMemorySized,
				R"(OpCopyMemorySized)",
				&s_operands[116],
				nullptr,
				5,
			},
			{
				SpirvOp::OpAccessChain,
				R"(OpAccessChain)",
				&s_operands[121],
				&s_operands[122],
				4,
			},
			{
				SpirvOp::OpInBoundsAccessChain,
				R"(OpInBoundsAccessChain)",
				&s_operands[121],
				&s_operands[122],
				4,
			},
			{
				SpirvOp::OpPtrAccessChain,
				R"(OpPtrAccessChain)",
				&s_operands[125],
				&s_operands[126],
				5,
			},
			{
				SpirvOp::OpArrayLength,
				R"(OpArrayLength)",
				&s_operands[130],
				&s_operands[131],
				4,
			},
			{
				SpirvOp::OpGenericPtrMemSemantics,
				R"(OpGenericPtrMemSemantics)",
				&s_operands[105],
				&s_operands[106],
				3,
			},
			{
				SpirvOp::OpInBoundsPtrAccessChain,
				R"(OpInBoundsPtrAccessChain)",
				&s_operands[125],
				&s_operands[126],
				5,
			},
			{
				SpirvOp::OpDecorate,
				R"(OpDecorate)",
				&s_operands[134],
				nullptr,
				2,
			},
			{
				SpirvOp::OpMemberDecorate,
				R"(OpMemberDecorate)",
				&s_operands[136],
				nullptr,
				3,
			},
			{
				SpirvOp::OpDecorationGroup,
				R"(OpDecorationGroup)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpGroupDecorate,
				R"(OpGroupDecorate)",
				&s_operands[139],
				nullptr,
				2,
			},
			{
				SpirvOp::OpGroupMemberDecorate,
				R"(OpGroupMemberDecorate)",
				&s_operands[141],
				nullptr,
				2,
			},
			{
				SpirvOp::OpVectorExtractDynamic,
				R"(OpVectorExtractDynamic)",
				&s_operands[143],
				&s_operands[144],
				4,
			},
			{
				SpirvOp::OpVectorInsertDynamic,
				R"(OpVectorInsertDynamic)",
				&s_operands[147],
				&s_operands[148],
				5,
			},
			{
				SpirvOp::OpVectorShuffle,
				R"(OpVectorShuffle)",
				&s_operands[152],
				&s_operands[153],
				5,
			},
			{
				SpirvOp::OpCompositeConstruct,
				R"(OpCompositeConstruct)",
				&s_operands[77],
				&s_operands[78],
				3,
			},
			{
				SpirvOp::OpCompositeExtract,
				R"(OpCompositeExtract)",
				&s_operands[157],
				&s_operands[158],
				4,
			},
			{
				SpirvOp::OpCompositeInsert,
				R"(OpCompositeInsert)",
				&s_operands[161],
				&s_operands[162],
				5,
			},
			{
				SpirvOp::OpCopyObject,
				R"(OpCopyObject)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpTranspose,
				R"(OpTranspose)",
				&s_operands[169],
				&s_operands[170],
				3,
			},
			{
				SpirvOp::OpSampledImage,
				R"(OpSampledImage)",
				&s_operands[172],
				&s_operands[173],
				4,
			},
			{
				SpirvOp::OpImageSampleImplicitLod,
				R"(OpImageSampleImplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSampleExplicitLod,
				R"(OpImageSampleExplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSampleDrefImplicitLod,
				R"(OpImageSampleDrefImplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSampleDrefExplicitLod,
				R"(OpImageSampleDrefExplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSampleProjImplicitLod,
				R"(OpImageSampleProjImplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSampleProjExplicitLod,
				R"(OpImageSampleProjExplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSampleProjDrefImplicitLod,
				R"(OpImageSampleProjDrefImplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSampleProjDrefExplicitLod,
				R"(OpImageSampleProjDrefExplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageFetch,
				R"(OpImageFetch)",
				&s_operands[187],
				&s_operands[188],
				5,
			},
			{
				SpirvOp::OpImageGather,
				R"(OpImageGather)",
				&s_operands[192],
				&s_operands[193],
				6,
			},
			{
				SpirvOp::OpImageDrefGather,
				R"(OpImageDrefGather)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageRead,
				R"(OpImageRead)",
				&s_operands[187],
				&s_operands[188],
				5,
			},
			{
				SpirvOp::OpImageWrite,
				R"(OpImageWrite)",
				&s_operands[198],
				nullptr,
				4,
			},
			{
				SpirvOp::OpImage,
				R"(OpImage)",
				&s_operands[176],
				&s_operands[177],
				3,
			},
			{
				SpirvOp::OpImageQueryFormat,
				R"(OpImageQueryFormat)",
				&s_operands[100],
				&s_operands[101],
				3,
			},
			{
				SpirvOp::OpImageQueryOrder,
				R"(OpImageQueryOrder)",
				&s_operands[100],
				&s_operands[101],
				3,
			},
			{
				SpirvOp::OpImageQuerySizeLod,
				R"(OpImageQuerySizeLod)",
				&s_operands[202],
				&s_operands[203],
				4,
			},
			{
				SpirvOp::OpImageQuerySize,
				R"(OpImageQuerySize)",
				&s_operands[100],
				&s_operands[101],
				3,
			},
			{
				SpirvOp::OpImageQueryLod,
				R"(OpImageQueryLod)",
				&s_operands[176],
				&s_operands[177],
				4,
			},
			{
				SpirvOp::OpImageQueryLevels,
				R"(OpImageQueryLevels)",
				&s_operands[100],
				&s_operands[101],
				3,
			},
			{
				SpirvOp::OpImageQuerySamples,
				R"(OpImageQuerySamples)",
				&s_operands[100],
				&s_operands[101],
				3,
			},
			{
				SpirvOp::OpConvertFToU,
				R"(OpConvertFToU)",
				&s_operands[206],
				&s_operands[207],
				3,
			},
			{
				SpirvOp::OpConvertFToS,
				R"(OpConvertFToS)",
				&s_operands[206],
				&s_operands[207],
				3,
			},
			{
				SpirvOp::OpConvertSToF,
				R"(OpConvertSToF)",
				&s_operands[209],
				&s_operands[210],
				3,
			},
			{
				SpirvOp::OpConvertUToF,
				R"(OpConvertUToF)",
				&s_operands[212],
				&s_operands[213],
				3,
			},
			{
				SpirvOp::OpUConvert,
				R"(OpUConvert)",
				&s_operands[212],
				&s_operands[213],
				3,
			},
			{
				SpirvOp::OpSConvert,
				R"(OpSConvert)",
				&s_operands[209],
				&s_operands[210],
				3,
			},
			{
				SpirvOp::OpFConvert,
				R"(OpFConvert)",
				&s_operands[206],
				&s_operands[207],
				3,
			},
			{
				SpirvOp::OpQuantizeToF16,
				R"(OpQuantizeToF16)",
				&s_operands[215],
				&s_operands[216],
				3,
			},
			{
				SpirvOp::OpConvertPtrToU,
				R"(OpConvertPtrToU)",
				&s_operands[105],
				&s_operands[106],
				3,
			},
			{
				SpirvOp::OpSatConvertSToU,
				R"(OpSatConvertSToU)",
				&s_operands[209],
				&s_operands[210],
				3,
			},
			{
				SpirvOp::OpSatConvertUToS,
				R"(OpSatConvertUToS)",
				&s_operands[212],
				&s_operands[213],
				3,
			},
			{
				SpirvOp::OpConvertUToPtr,
				R"(OpConvertUToPtr)",
				&s_operands[218],
				&s_operands[219],
				3,
			},
			{
				SpirvOp::OpPtrCastToGeneric,
				R"(OpPtrCastToGeneric)",
				&s_operands[105],
				&s_operands[106],
				3,
			},
			{
				SpirvOp::OpGenericCastToPtr,
				R"(OpGenericCastToPtr)",
				&s_operands[105],
				&s_operands[106],
				3,
			},
			{
				SpirvOp::OpGenericCastToPtrExplicit,
				R"(OpGenericCastToPtrExplicit)",
				&s_operands[221],
				&s_operands[222],
				4,
			},
			{
				SpirvOp::OpBitcast,
				R"(OpBitcast)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpSNegate,
				R"(OpSNegate)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpFNegate,
				R"(OpFNegate)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpIAdd,
				R"(OpIAdd)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFAdd,
				R"(OpFAdd)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpISub,
				R"(OpISub)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFSub,
				R"(OpFSub)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpIMul,
				R"(OpIMul)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFMul,
				R"(OpFMul)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUDiv,
				R"(OpUDiv)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSDiv,
				R"(OpSDiv)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFDiv,
				R"(OpFDiv)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUMod,
				R"(OpUMod)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSRem,
				R"(OpSRem)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSMod,
				R"(OpSMod)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFRem,
				R"(OpFRem)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFMod,
				R"(OpFMod)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpVectorTimesScalar,
				R"(OpVectorTimesScalar)",
				&s_operands[229],
				&s_operands[230],
				4,
			},
			{
				SpirvOp::OpMatrixTimesScalar,
				R"(OpMatrixTimesScalar)",
				&s_operands[233],
				&s_operands[234],
				4,
			},
			{
				SpirvOp::OpVectorTimesMatrix,
				R"(OpVectorTimesMatrix)",
				&s_operands[237],
				&s_operands[238],
				4,
			},
			{
				SpirvOp::OpMatrixTimesVector,
				R"(OpMatrixTimesVector)",
				&s_operands[241],
				&s_operands[242],
				4,
			},
			{
				SpirvOp::OpMatrixTimesMatrix,
				R"(OpMatrixTimesMatrix)",
				&s_operands[245],
				&s_operands[246],
				4,
			},
			{
				SpirvOp::OpOuterProduct,
				R"(OpOuterProduct)",
				&s_operands[152],
				&s_operands[153],
				4,
			},
			{
				SpirvOp::OpDot,
				R"(OpDot)",
				&s_operands[152],
				&s_operands[153],
				4,
			},
			{
				SpirvOp::OpIAddCarry,
				R"(OpIAddCarry)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpISubBorrow,
				R"(OpISubBorrow)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUMulExtended,
				R"(OpUMulExtended)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSMulExtended,
				R"(OpSMulExtended)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpAny,
				R"(OpAny)",
				&s_operands[143],
				&s_operands[144],
				3,
			},
			{
				SpirvOp::OpAll,
				R"(OpAll)",
				&s_operands[143],
				&s_operands[144],
				3,
			},
			{
				SpirvOp::OpIsNan,
				R"(OpIsNan)",
				&s_operands[249],
				&s_operands[250],
				3,
			},
			{
				SpirvOp::OpIsInf,
				R"(OpIsInf)",
				&s_operands[249],
				&s_operands[250],
				3,
			},
			{
				SpirvOp::OpIsFinite,
				R"(OpIsFinite)",
				&s_operands[249],
				&s_operands[250],
				3,
			},
			{
				SpirvOp::OpIsNormal,
				R"(OpIsNormal)",
				&s_operands[249],
				&s_operands[250],
				3,
			},
			{
				SpirvOp::OpSignBitSet,
				R"(OpSignBitSet)",
				&s_operands[249],
				&s_operands[250],
				3,
			},
			{
				SpirvOp::OpLessOrGreater,
				R"(OpLessOrGreater)",
				&s_operands[252],
				&s_operands[253],
				4,
			},
			{
				SpirvOp::OpOrdered,
				R"(OpOrdered)",
				&s_operands[252],
				&s_operands[253],
				4,
			},
			{
				SpirvOp::OpUnordered,
				R"(OpUnordered)",
				&s_operands[252],
				&s_operands[253],
				4,
			},
			{
				SpirvOp::OpLogicalEqual,
				R"(OpLogicalEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpLogicalNotEqual,
				R"(OpLogicalNotEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpLogicalOr,
				R"(OpLogicalOr)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpLogicalAnd,
				R"(OpLogicalAnd)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpLogicalNot,
				R"(OpLogicalNot)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpSelect,
				R"(OpSelect)",
				&s_operands[256],
				&s_operands[257],
				5,
			},
			{
				SpirvOp::OpIEqual,
				R"(OpIEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpINotEqual,
				R"(OpINotEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUGreaterThan,
				R"(OpUGreaterThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSGreaterThan,
				R"(OpSGreaterThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUGreaterThanEqual,
				R"(OpUGreaterThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSGreaterThanEqual,
				R"(OpSGreaterThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpULessThan,
				R"(OpULessThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSLessThan,
				R"(OpSLessThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpULessThanEqual,
				R"(OpULessThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpSLessThanEqual,
				R"(OpSLessThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFOrdEqual,
				R"(OpFOrdEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFUnordEqual,
				R"(OpFUnordEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFOrdNotEqual,
				R"(OpFOrdNotEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFUnordNotEqual,
				R"(OpFUnordNotEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFOrdLessThan,
				R"(OpFOrdLessThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFUnordLessThan,
				R"(OpFUnordLessThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFOrdGreaterThan,
				R"(OpFOrdGreaterThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFUnordGreaterThan,
				R"(OpFUnordGreaterThan)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFOrdLessThanEqual,
				R"(OpFOrdLessThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFUnordLessThanEqual,
				R"(OpFUnordLessThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFOrdGreaterThanEqual,
				R"(OpFOrdGreaterThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpFUnordGreaterThanEqual,
				R"(OpFUnordGreaterThanEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpShiftRightLogical,
				R"(OpShiftRightLogical)",
				&s_operands[261],
				&s_operands[262],
				4,
			},
			{
				SpirvOp::OpShiftRightArithmetic,
				R"(OpShiftRightArithmetic)",
				&s_operands[261],
				&s_operands[262],
				4,
			},
			{
				SpirvOp::OpShiftLeftLogical,
				R"(OpShiftLeftLogical)",
				&s_operands[261],
				&s_operands[262],
				4,
			},
			{
				SpirvOp::OpBitwiseOr,
				R"(OpBitwiseOr)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpBitwiseXor,
				R"(OpBitwiseXor)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpBitwiseAnd,
				R"(OpBitwiseAnd)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpNot,
				R"(OpNot)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpBitFieldInsert,
				R"(OpBitFieldInsert)",
				&s_operands[265],
				&s_operands[266],
				6,
			},
			{
				SpirvOp::OpBitFieldSExtract,
				R"(OpBitFieldSExtract)",
				&s_operands[271],
				&s_operands[272],
				5,
			},
			{
				SpirvOp::OpBitFieldUExtract,
				R"(OpBitFieldUExtract)",
				&s_operands[271],
				&s_operands[272],
				5,
			},
			{
				SpirvOp::OpBitReverse,
				R"(OpBitReverse)",
				&s_operands[121],
				&s_operands[122],
				3,
			},
			{
				SpirvOp::OpBitCount,
				R"(OpBitCount)",
				&s_operands[121],
				&s_operands[122],
				3,
			},
			{
				SpirvOp::OpDPdx,
				R"(OpDPdx)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpDPdy,
				R"(OpDPdy)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpFwidth,
				R"(OpFwidth)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpDPdxFine,
				R"(OpDPdxFine)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpDPdyFine,
				R"(OpDPdyFine)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpFwidthFine,
				R"(OpFwidthFine)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpDPdxCoarse,
				R"(OpDPdxCoarse)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpDPdyCoarse,
				R"(OpDPdyCoarse)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpFwidthCoarse,
				R"(OpFwidthCoarse)",
				&s_operands[276],
				&s_operands[277],
				3,
			},
			{
				SpirvOp::OpEmitVertex,
				R"(OpEmitVertex)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpEndPrimitive,
				R"(OpEndPrimitive)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpEmitStreamVertex,
				R"(OpEmitStreamVertex)",
				&s_operands[279],
				nullptr,
				1,
			},
			{
				SpirvOp::OpEndStreamPrimitive,
				R"(OpEndStreamPrimitive)",
				&s_operands[279],
				nullptr,
				1,
			},
			{
				SpirvOp::OpControlBarrier,
				R"(OpControlBarrier)",
				&s_operands[280],
				nullptr,
				3,
			},
			{
				SpirvOp::OpMemoryBarrier,
				R"(OpMemoryBarrier)",
				&s_operands[281],
				nullptr,
				2,
			},
			{
				SpirvOp::OpAtomicLoad,
				R"(OpAtomicLoad)",
				&s_operands[283],
				&s_operands[284],
				5,
			},
			{
				SpirvOp::OpAtomicStore,
				R"(OpAtomicStore)",
				&s_operands[288],
				nullptr,
				4,
			},
			{
				SpirvOp::OpAtomicExchange,
				R"(OpAtomicExchange)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicCompareExchange,
				R"(OpAtomicCompareExchange)",
				&s_operands[298],
				&s_operands[299],
				8,
			},
			{
				SpirvOp::OpAtomicCompareExchangeWeak,
				R"(OpAtomicCompareExchangeWeak)",
				&s_operands[298],
				&s_operands[299],
				8,
			},
			{
				SpirvOp::OpAtomicIIncrement,
				R"(OpAtomicIIncrement)",
				&s_operands[283],
				&s_operands[284],
				5,
			},
			{
				SpirvOp::OpAtomicIDecrement,
				R"(OpAtomicIDecrement)",
				&s_operands[283],
				&s_operands[284],
				5,
			},
			{
				SpirvOp::OpAtomicIAdd,
				R"(OpAtomicIAdd)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicISub,
				R"(OpAtomicISub)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicSMin,
				R"(OpAtomicSMin)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicUMin,
				R"(OpAtomicUMin)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicSMax,
				R"(OpAtomicSMax)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicUMax,
				R"(OpAtomicUMax)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicAnd,
				R"(OpAtomicAnd)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicOr,
				R"(OpAtomicOr)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicXor,
				R"(OpAtomicXor)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpPhi,
				R"(OpPhi)",
				&s_operands[306],
				&s_operands[307],
				3,
			},
			{
				SpirvOp::OpLoopMerge,
				R"(OpLoopMerge)",
				&s_operands[309],
				nullptr,
				3,
			},
			{
				SpirvOp::OpSelectionMerge,
				R"(OpSelectionMerge)",
				&s_operands[312],
				nullptr,
				2,
			},
			{
				SpirvOp::OpLabel,
				R"(OpLabel)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpBranch,
				R"(OpBranch)",
				&s_operands[314],
				nullptr,
				1,
			},
			{
				SpirvOp::OpBranchConditional,
				R"(OpBranchConditional)",
				&s_operands[315],
				nullptr,
				4,
			},
			{
				SpirvOp::OpSwitch,
				R"(OpSwitch)",
				&s_operands[319],
				nullptr,
				3,
			},
			{
				SpirvOp::OpKill,
				R"(OpKill)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpReturn,
				R"(OpReturn)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpReturnValue,
				R"(OpReturnValue)",
				&s_operands[217],
				nullptr,
				1,
			},
			{
				SpirvOp::OpUnreachable,
				R"(OpUnreachable)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpLifetimeStart,
				R"(OpLifetimeStart)",
				&s_operands[322],
				nullptr,
				2,
			},
			{
				SpirvOp::OpLifetimeStop,
				R"(OpLifetimeStop)",
				&s_operands[322],
				nullptr,
				2,
			},
			{
				SpirvOp::OpGroupAsyncCopy,
				R"(OpGroupAsyncCopy)",
				&s_operands[324],
				&s_operands[325],
				8,
			},
			{
				SpirvOp::OpGroupWaitEvents,
				R"(OpGroupWaitEvents)",
				&s_operands[332],
				nullptr,
				3,
			},
			{
				SpirvOp::OpGroupAll,
				R"(OpGroupAll)",
				&s_operands[335],
				&s_operands[336],
				4,
			},
			{
				SpirvOp::OpGroupAny,
				R"(OpGroupAny)",
				&s_operands[335],
				&s_operands[336],
				4,
			},
			{
				SpirvOp::OpGroupBroadcast,
				R"(OpGroupBroadcast)",
				&s_operands[339],
				&s_operands[340],
				5,
			},
			{
				SpirvOp::OpGroupIAdd,
				R"(OpGroupIAdd)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupFAdd,
				R"(OpGroupFAdd)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupFMin,
				R"(OpGroupFMin)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupUMin,
				R"(OpGroupUMin)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupSMin,
				R"(OpGroupSMin)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupFMax,
				R"(OpGroupFMax)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupUMax,
				R"(OpGroupUMax)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupSMax,
				R"(OpGroupSMax)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpReadPipe,
				R"(OpReadPipe)",
				&s_operands[349],
				&s_operands[350],
				6,
			},
			{
				SpirvOp::OpWritePipe,
				R"(OpWritePipe)",
				&s_operands[349],
				&s_operands[350],
				6,
			},
			{
				SpirvOp::OpReservedReadPipe,
				R"(OpReservedReadPipe)",
				&s_operands[355],
				&s_operands[356],
				8,
			},
			{
				SpirvOp::OpReservedWritePipe,
				R"(OpReservedWritePipe)",
				&s_operands[355],
				&s_operands[356],
				8,
			},
			{
				SpirvOp::OpReserveReadPipePackets,
				R"(OpReserveReadPipePackets)",
				&s_operands[363],
				&s_operands[364],
				6,
			},
			{
				SpirvOp::OpReserveWritePipePackets,
				R"(OpReserveWritePipePackets)",
				&s_operands[363],
				&s_operands[364],
				6,
			},
			{
				SpirvOp::OpCommitReadPipe,
				R"(OpCommitReadPipe)",
				&s_operands[369],
				nullptr,
				4,
			},
			{
				SpirvOp::OpCommitWritePipe,
				R"(OpCommitWritePipe)",
				&s_operands[369],
				nullptr,
				4,
			},
			{
				SpirvOp::OpIsValidReserveId,
				R"(OpIsValidReserveId)",
				&s_operands[373],
				&s_operands[374],
				3,
			},
			{
				SpirvOp::OpGetNumPipePackets,
				R"(OpGetNumPipePackets)",
				&s_operands[376],
				&s_operands[377],
				5,
			},
			{
				SpirvOp::OpGetMaxPipePackets,
				R"(OpGetMaxPipePackets)",
				&s_operands[376],
				&s_operands[377],
				5,
			},
			{
				SpirvOp::OpGroupReserveReadPipePackets,
				R"(OpGroupReserveReadPipePackets)",
				&s_operands[381],
				&s_operands[382],
				7,
			},
			{
				SpirvOp::OpGroupReserveWritePipePackets,
				R"(OpGroupReserveWritePipePackets)",
				&s_operands[381],
				&s_operands[382],
				7,
			},
			{
				SpirvOp::OpGroupCommitReadPipe,
				R"(OpGroupCommitReadPipe)",
				&s_operands[388],
				nullptr,
				5,
			},
			{
				SpirvOp::OpGroupCommitWritePipe,
				R"(OpGroupCommitWritePipe)",
				&s_operands[388],
				nullptr,
				5,
			},
			{
				SpirvOp::OpEnqueueMarker,
				R"(OpEnqueueMarker)",
				&s_operands[393],
				&s_operands[394],
				6,
			},
			{
				SpirvOp::OpEnqueueKernel,
				R"(OpEnqueueKernel)",
				&s_operands[399],
				&s_operands[400],
				13,
			},
			{
				SpirvOp::OpGetKernelNDrangeSubGroupCount,
				R"(OpGetKernelNDrangeSubGroupCount)",
				&s_operands[412],
				&s_operands[413],
				7,
			},
			{
				SpirvOp::OpGetKernelNDrangeMaxSubGroupSize,
				R"(OpGetKernelNDrangeMaxSubGroupSize)",
				&s_operands[412],
				&s_operands[413],
				7,
			},
			{
				SpirvOp::OpGetKernelWorkGroupSize,
				R"(OpGetKernelWorkGroupSize)",
				&s_operands[419],
				&s_operands[420],
				6,
			},
			{
				SpirvOp::OpGetKernelPreferredWorkGroupSizeMultiple,
				R"(OpGetKernelPreferredWorkGroupSizeMultiple)",
				&s_operands[419],
				&s_operands[420],
				6,
			},
			{
				SpirvOp::OpRetainEvent,
				R"(OpRetainEvent)",
				&s_operands[331],
				nullptr,
				1,
			},
			{
				SpirvOp::OpReleaseEvent,
				R"(OpReleaseEvent)",
				&s_operands[331],
				nullptr,
				1,
			},
			{
				SpirvOp::OpCreateUserEvent,
				R"(OpCreateUserEvent)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpIsValidEvent,
				R"(OpIsValidEvent)",
				&s_operands[425],
				&s_operands[426],
				3,
			},
			{
				SpirvOp::OpSetUserEventStatus,
				R"(OpSetUserEventStatus)",
				&s_operands[428],
				nullptr,
				2,
			},
			{
				SpirvOp::OpCaptureEventProfilingInfo,
				R"(OpCaptureEventProfilingInfo)",
				&s_operands[430],
				nullptr,
				3,
			},
			{
				SpirvOp::OpGetDefaultQueue,
				R"(OpGetDefaultQueue)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpBuildNDRange,
				R"(OpBuildNDRange)",
				&s_operands[433],
				&s_operands[434],
				5,
			},
			{
				SpirvOp::OpImageSparseSampleImplicitLod,
				R"(OpImageSparseSampleImplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSparseSampleExplicitLod,
				R"(OpImageSparseSampleExplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSparseSampleDrefImplicitLod,
				R"(OpImageSparseSampleDrefImplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSparseSampleDrefExplicitLod,
				R"(OpImageSparseSampleDrefExplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSparseSampleProjImplicitLod,
				R"(OpImageSparseSampleProjImplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSparseSampleProjExplicitLod,
				R"(OpImageSparseSampleProjExplicitLod)",
				&s_operands[176],
				&s_operands[177],
				5,
			},
			{
				SpirvOp::OpImageSparseSampleProjDrefImplicitLod,
				R"(OpImageSparseSampleProjDrefImplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSparseSampleProjDrefExplicitLod,
				R"(OpImageSparseSampleProjDrefExplicitLod)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSparseFetch,
				R"(OpImageSparseFetch)",
				&s_operands[187],
				&s_operands[188],
				5,
			},
			{
				SpirvOp::OpImageSparseGather,
				R"(OpImageSparseGather)",
				&s_operands[192],
				&s_operands[193],
				6,
			},
			{
				SpirvOp::OpImageSparseDrefGather,
				R"(OpImageSparseDrefGather)",
				&s_operands[181],
				&s_operands[182],
				6,
			},
			{
				SpirvOp::OpImageSparseTexelsResident,
				R"(OpImageSparseTexelsResident)",
				&s_operands[438],
				&s_operands[439],
				3,
			},
			{
				SpirvOp::OpNoLine,
				R"(OpNoLine)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpAtomicFlagTestAndSet,
				R"(OpAtomicFlagTestAndSet)",
				&s_operands[283],
				&s_operands[284],
				5,
			},
			{
				SpirvOp::OpAtomicFlagClear,
				R"(OpAtomicFlagClear)",
				&s_operands[285],
				nullptr,
				3,
			},
			{
				SpirvOp::OpImageSparseRead,
				R"(OpImageSparseRead)",
				&s_operands[187],
				&s_operands[188],
				5,
			},
			{
				SpirvOp::OpSizeOf,
				R"(OpSizeOf)",
				&s_operands[105],
				&s_operands[106],
				3,
			},
			{
				SpirvOp::OpTypePipeStorage,
				R"(OpTypePipeStorage)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpConstantPipeStorage,
				R"(OpConstantPipeStorage)",
				&s_operands[441],
				&s_operands[442],
				5,
			},
			{
				SpirvOp::OpCreatePipeFromPipeStorage,
				R"(OpCreatePipeFromPipeStorage)",
				&s_operands[446],
				&s_operands[447],
				3,
			},
			{
				SpirvOp::OpGetKernelLocalSizeForSubgroupCount,
				R"(OpGetKernelLocalSizeForSubgroupCount)",
				&s_operands[449],
				&s_operands[450],
				7,
			},
			{
				SpirvOp::OpGetKernelMaxNumSubgroups,
				R"(OpGetKernelMaxNumSubgroups)",
				&s_operands[419],
				&s_operands[420],
				6,
			},
			{
				SpirvOp::OpTypeNamedBarrier,
				R"(OpTypeNamedBarrier)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpNamedBarrierInitialize,
				R"(OpNamedBarrierInitialize)",
				&s_operands[449],
				&s_operands[450],
				3,
			},
			{
				SpirvOp::OpMemoryNamedBarrier,
				R"(OpMemoryNamedBarrier)",
				&s_operands[456],
				nullptr,
				3,
			},
			{
				SpirvOp::OpModuleProcessed,
				R"(OpModuleProcessed)",
				&s_operands[459],
				nullptr,
				1,
			},
			{
				SpirvOp::OpExecutionModeId,
				R"(OpExecutionModeId)",
				&s_operands[31],
				nullptr,
				2,
			},
			{
				SpirvOp::OpDecorateId,
				R"(OpDecorateId)",
				&s_operands[134],
				nullptr,
				2,
			},
			{
				SpirvOp::OpGroupNonUniformElect,
				R"(OpGroupNonUniformElect)",
				&s_operands[324],
				&s_operands[325],
				3,
			},
			{
				SpirvOp::OpGroupNonUniformAll,
				R"(OpGroupNonUniformAll)",
				&s_operands[335],
				&s_operands[336],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformAny,
				R"(OpGroupNonUniformAny)",
				&s_operands[335],
				&s_operands[336],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformAllEqual,
				R"(OpGroupNonUniformAllEqual)",
				&s_operands[339],
				&s_operands[340],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformBroadcast,
				R"(OpGroupNonUniformBroadcast)",
				&s_operands[460],
				&s_operands[461],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformBroadcastFirst,
				R"(OpGroupNonUniformBroadcastFirst)",
				&s_operands[339],
				&s_operands[340],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformBallot,
				R"(OpGroupNonUniformBallot)",
				&s_operands[335],
				&s_operands[336],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformInverseBallot,
				R"(OpGroupNonUniformInverseBallot)",
				&s_operands[339],
				&s_operands[340],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformBallotBitExtract,
				R"(OpGroupNonUniformBallotBitExtract)",
				&s_operands[465],
				&s_operands[466],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformBallotBitCount,
				R"(OpGroupNonUniformBallotBitCount)",
				&s_operands[470],
				&s_operands[471],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformBallotFindLSB,
				R"(OpGroupNonUniformBallotFindLSB)",
				&s_operands[339],
				&s_operands[340],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformBallotFindMSB,
				R"(OpGroupNonUniformBallotFindMSB)",
				&s_operands[339],
				&s_operands[340],
				4,
			},
			{
				SpirvOp::OpGroupNonUniformShuffle,
				R"(OpGroupNonUniformShuffle)",
				&s_operands[460],
				&s_operands[461],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformShuffleXor,
				R"(OpGroupNonUniformShuffleXor)",
				&s_operands[475],
				&s_operands[476],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformShuffleUp,
				R"(OpGroupNonUniformShuffleUp)",
				&s_operands[480],
				&s_operands[481],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformShuffleDown,
				R"(OpGroupNonUniformShuffleDown)",
				&s_operands[480],
				&s_operands[481],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformIAdd,
				R"(OpGroupNonUniformIAdd)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformFAdd,
				R"(OpGroupNonUniformFAdd)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformIMul,
				R"(OpGroupNonUniformIMul)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformFMul,
				R"(OpGroupNonUniformFMul)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformSMin,
				R"(OpGroupNonUniformSMin)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformUMin,
				R"(OpGroupNonUniformUMin)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformFMin,
				R"(OpGroupNonUniformFMin)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformSMax,
				R"(OpGroupNonUniformSMax)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformUMax,
				R"(OpGroupNonUniformUMax)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformFMax,
				R"(OpGroupNonUniformFMax)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformBitwiseAnd,
				R"(OpGroupNonUniformBitwiseAnd)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformBitwiseOr,
				R"(OpGroupNonUniformBitwiseOr)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformBitwiseXor,
				R"(OpGroupNonUniformBitwiseXor)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformLogicalAnd,
				R"(OpGroupNonUniformLogicalAnd)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformLogicalOr,
				R"(OpGroupNonUniformLogicalOr)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformLogicalXor,
				R"(OpGroupNonUniformLogicalXor)",
				&s_operands[485],
				&s_operands[486],
				6,
			},
			{
				SpirvOp::OpGroupNonUniformQuadBroadcast,
				R"(OpGroupNonUniformQuadBroadcast)",
				&s_operands[465],
				&s_operands[466],
				5,
			},
			{
				SpirvOp::OpGroupNonUniformQuadSwap,
				R"(OpGroupNonUniformQuadSwap)",
				&s_operands[491],
				&s_operands[492],
				5,
			},
			{
				SpirvOp::OpCopyLogical,
				R"(OpCopyLogical)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpPtrEqual,
				R"(OpPtrEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpPtrNotEqual,
				R"(OpPtrNotEqual)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpPtrDiff,
				R"(OpPtrDiff)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpColorAttachmentReadEXT,
				R"(OpColorAttachmentReadEXT)",
				&s_operands[496],
				&s_operands[497],
				4,
			},
			{
				SpirvOp::OpDepthAttachmentReadEXT,
				R"(OpDepthAttachmentReadEXT)",
				&s_operands[500],
				&s_operands[501],
				3,
			},
			{
				SpirvOp::OpStencilAttachmentReadEXT,
				R"(OpStencilAttachmentReadEXT)",
				&s_operands[500],
				&s_operands[501],
				3,
			},
			{
				SpirvOp::OpTerminateInvocation,
				R"(OpTerminateInvocation)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpTypeUntypedPointerKHR,
				R"(OpTypeUntypedPointerKHR)",
				&s_operands[64],
				&s_operands[64],
				2,
			},
			{
				SpirvOp::OpUntypedVariableKHR,
				R"(OpUntypedVariableKHR)",
				&s_operands[503],
				&s_operands[504],
				5,
			},
			{
				SpirvOp::OpUntypedAccessChainKHR,
				R"(OpUntypedAccessChainKHR)",
				&s_operands[508],
				&s_operands[509],
				5,
			},
			{
				SpirvOp::OpUntypedInBoundsAccessChainKHR,
				R"(OpUntypedInBoundsAccessChainKHR)",
				&s_operands[508],
				&s_operands[509],
				5,
			},
			{
				SpirvOp::OpSubgroupBallotKHR,
				R"(OpSubgroupBallotKHR)",
				&s_operands[513],
				&s_operands[514],
				3,
			},
			{
				SpirvOp::OpSubgroupFirstInvocationKHR,
				R"(OpSubgroupFirstInvocationKHR)",
				&s_operands[215],
				&s_operands[216],
				3,
			},
			{
				SpirvOp::OpUntypedPtrAccessChainKHR,
				R"(OpUntypedPtrAccessChainKHR)",
				&s_operands[516],
				&s_operands[517],
				6,
			},
			{
				SpirvOp::OpUntypedInBoundsPtrAccessChainKHR,
				R"(OpUntypedInBoundsPtrAccessChainKHR)",
				&s_operands[516],
				&s_operands[517],
				6,
			},
			{
				SpirvOp::OpUntypedArrayLengthKHR,
				R"(OpUntypedArrayLengthKHR)",
				&s_operands[522],
				&s_operands[523],
				5,
			},
			{
				SpirvOp::OpUntypedPrefetchKHR,
				R"(OpUntypedPrefetchKHR)",
				&s_operands[527],
				nullptr,
				5,
			},
			{
				SpirvOp::OpSubgroupAllKHR,
				R"(OpSubgroupAllKHR)",
				&s_operands[513],
				&s_operands[514],
				3,
			},
			{
				SpirvOp::OpSubgroupAnyKHR,
				R"(OpSubgroupAnyKHR)",
				&s_operands[513],
				&s_operands[514],
				3,
			},
			{
				SpirvOp::OpSubgroupAllEqualKHR,
				R"(OpSubgroupAllEqualKHR)",
				&s_operands[513],
				&s_operands[514],
				3,
			},
			{
				SpirvOp::OpGroupNonUniformRotateKHR,
				R"(OpGroupNonUniformRotateKHR)",
				&s_operands[532],
				&s_operands[533],
				6,
			},
			{
				SpirvOp::OpSubgroupReadInvocationKHR,
				R"(OpSubgroupReadInvocationKHR)",
				&s_operands[538],
				&s_operands[539],
				4,
			},
			{
				SpirvOp::OpExtInstWithForwardRefsKHR,
				R"(OpExtInstWithForwardRefsKHR)",
				&s_operands[20],
				&s_operands[21],
				5,
			},
			{
				SpirvOp::OpTraceRayKHR,
				R"(OpTraceRayKHR)",
				&s_operands[542],
				nullptr,
				11,
			},
			{
				SpirvOp::OpExecuteCallableKHR,
				R"(OpExecuteCallableKHR)",
				&s_operands[553],
				nullptr,
				2,
			},
			{
				SpirvOp::OpConvertUToAccelerationStructureKHR,
				R"(OpConvertUToAccelerationStructureKHR)",
				&s_operands[555],
				&s_operands[556],
				3,
			},
			{
				SpirvOp::OpIgnoreIntersectionKHR,
				R"(OpIgnoreIntersectionKHR)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpTerminateRayKHR,
				R"(OpTerminateRayKHR)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpSDot,
				R"(OpSDot)",
				&s_operands[558],
				&s_operands[559],
				5,
			},
			{
				SpirvOp::OpUDot,
				R"(OpUDot)",
				&s_operands[558],
				&s_operands[559],
				5,
			},
			{
				SpirvOp::OpSUDot,
				R"(OpSUDot)",
				&s_operands[558],
				&s_operands[559],
				5,
			},
			{
				SpirvOp::OpSDotAccSat,
				R"(OpSDotAccSat)",
				&s_operands[563],
				&s_operands[564],
				6,
			},
			{
				SpirvOp::OpUDotAccSat,
				R"(OpUDotAccSat)",
				&s_operands[563],
				&s_operands[564],
				6,
			},
			{
				SpirvOp::OpSUDotAccSat,
				R"(OpSUDotAccSat)",
				&s_operands[563],
				&s_operands[564],
				6,
			},
			{
				SpirvOp::OpTypeCooperativeMatrixKHR,
				R"(OpTypeCooperativeMatrixKHR)",
				&s_operands[569],
				&s_operands[569],
				6,
			},
			{
				SpirvOp::OpCooperativeMatrixLoadKHR,
				R"(OpCooperativeMatrixLoadKHR)",
				&s_operands[575],
				&s_operands[576],
				6,
			},
			{
				SpirvOp::OpCooperativeMatrixStoreKHR,
				R"(OpCooperativeMatrixStoreKHR)",
				&s_operands[581],
				nullptr,
				5,
			},
			{
				SpirvOp::OpCooperativeMatrixMulAddKHR,
				R"(OpCooperativeMatrixMulAddKHR)",
				&s_operands[586],
				&s_operands[587],
				6,
			},
			{
				SpirvOp::OpCooperativeMatrixLengthKHR,
				R"(OpCooperativeMatrixLengthKHR)",
				&s_operands[592],
				&s_operands[593],
				3,
			},
			{
				SpirvOp::OpConstantCompositeReplicateEXT,
				R"(OpConstantCompositeReplicateEXT)",
				&s_operands[215],
				&s_operands[216],
				3,
			},
			{
				SpirvOp::OpSpecConstantCompositeReplicateEXT,
				R"(OpSpecConstantCompositeReplicateEXT)",
				&s_operands[215],
				&s_operands[216],
				3,
			},
			{
				SpirvOp::OpCompositeConstructReplicateEXT,
				R"(OpCompositeConstructReplicateEXT)",
				&s_operands[215],
				&s_operands[216],
				3,
			},
			{
				SpirvOp::OpTypeRayQueryKHR,
				R"(OpTypeRayQueryKHR)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpRayQueryInitializeKHR,
				R"(OpRayQueryInitializeKHR)",
				&s_operands[595],
				nullptr,
				8,
			},
			{
				SpirvOp::OpRayQueryTerminateKHR,
				R"(OpRayQueryTerminateKHR)",
				&s_operands[595],
				nullptr,
				1,
			},
			{
				SpirvOp::OpRayQueryGenerateIntersectionKHR,
				R"(OpRayQueryGenerateIntersectionKHR)",
				&s_operands[603],
				nullptr,
				2,
			},
			{
				SpirvOp::OpRayQueryConfirmIntersectionKHR,
				R"(OpRayQueryConfirmIntersectionKHR)",
				&s_operands[595],
				nullptr,
				1,
			},
			{
				SpirvOp::OpRayQueryProceedKHR,
				R"(OpRayQueryProceedKHR)",
				&s_operands[605],
				&s_operands[606],
				3,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionTypeKHR,
				R"(OpRayQueryGetIntersectionTypeKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpImageSampleWeightedQCOM,
				R"(OpImageSampleWeightedQCOM)",
				&s_operands[612],
				&s_operands[613],
				5,
			},
			{
				SpirvOp::OpImageBoxFilterQCOM,
				R"(OpImageBoxFilterQCOM)",
				&s_operands[617],
				&s_operands[618],
				5,
			},
			{
				SpirvOp::OpImageBlockMatchSSDQCOM,
				R"(OpImageBlockMatchSSDQCOM)",
				&s_operands[622],
				&s_operands[623],
				7,
			},
			{
				SpirvOp::OpImageBlockMatchSADQCOM,
				R"(OpImageBlockMatchSADQCOM)",
				&s_operands[622],
				&s_operands[623],
				7,
			},
			{
				SpirvOp::OpImageBlockMatchWindowSSDQCOM,
				R"(OpImageBlockMatchWindowSSDQCOM)",
				&s_operands[629],
				&s_operands[630],
				7,
			},
			{
				SpirvOp::OpImageBlockMatchWindowSADQCOM,
				R"(OpImageBlockMatchWindowSADQCOM)",
				&s_operands[629],
				&s_operands[630],
				7,
			},
			{
				SpirvOp::OpImageBlockMatchGatherSSDQCOM,
				R"(OpImageBlockMatchGatherSSDQCOM)",
				&s_operands[629],
				&s_operands[630],
				7,
			},
			{
				SpirvOp::OpImageBlockMatchGatherSADQCOM,
				R"(OpImageBlockMatchGatherSADQCOM)",
				&s_operands[629],
				&s_operands[630],
				7,
			},
			{
				SpirvOp::OpGroupIAddNonUniformAMD,
				R"(OpGroupIAddNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupFAddNonUniformAMD,
				R"(OpGroupFAddNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupFMinNonUniformAMD,
				R"(OpGroupFMinNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupUMinNonUniformAMD,
				R"(OpGroupUMinNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupSMinNonUniformAMD,
				R"(OpGroupSMinNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupFMaxNonUniformAMD,
				R"(OpGroupFMaxNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupUMaxNonUniformAMD,
				R"(OpGroupUMaxNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupSMaxNonUniformAMD,
				R"(OpGroupSMaxNonUniformAMD)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpFragmentMaskFetchAMD,
				R"(OpFragmentMaskFetchAMD)",
				&s_operands[100],
				&s_operands[101],
				4,
			},
			{
				SpirvOp::OpFragmentFetchAMD,
				R"(OpFragmentFetchAMD)",
				&s_operands[636],
				&s_operands[637],
				5,
			},
			{
				SpirvOp::OpReadClockKHR,
				R"(OpReadClockKHR)",
				&s_operands[641],
				&s_operands[642],
				3,
			},
			{
				SpirvOp::OpAllocateNodePayloadsAMDX,
				R"(OpAllocateNodePayloadsAMDX)",
				&s_operands[644],
				&s_operands[645],
				5,
			},
			{
				SpirvOp::OpEnqueueNodePayloadsAMDX,
				R"(OpEnqueueNodePayloadsAMDX)",
				&s_operands[649],
				nullptr,
				1,
			},
			{
				SpirvOp::OpTypeNodePayloadArrayAMDX,
				R"(OpTypeNodePayloadArrayAMDX)",
				&s_operands[650],
				&s_operands[650],
				2,
			},
			{
				SpirvOp::OpFinishWritingNodePayloadAMDX,
				R"(OpFinishWritingNodePayloadAMDX)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpNodePayloadArrayLengthAMDX,
				R"(OpNodePayloadArrayLengthAMDX)",
				&s_operands[655],
				&s_operands[656],
				3,
			},
			{
				SpirvOp::OpIsNodePayloadValidAMDX,
				R"(OpIsNodePayloadValidAMDX)",
				&s_operands[658],
				&s_operands[659],
				4,
			},
			{
				SpirvOp::OpConstantStringAMDX,
				R"(OpConstantStringAMDX)",
				&s_operands[662],
				&s_operands[662],
				2,
			},
			{
				SpirvOp::OpSpecConstantStringAMDX,
				R"(OpSpecConstantStringAMDX)",
				&s_operands[662],
				&s_operands[662],
				2,
			},
			{
				SpirvOp::OpGroupNonUniformQuadAllKHR,
				R"(OpGroupNonUniformQuadAllKHR)",
				&s_operands[513],
				&s_operands[514],
				3,
			},
			{
				SpirvOp::OpGroupNonUniformQuadAnyKHR,
				R"(OpGroupNonUniformQuadAnyKHR)",
				&s_operands[513],
				&s_operands[514],
				3,
			},
			{
				SpirvOp::OpHitObjectRecordHitMotionNV,
				R"(OpHitObjectRecordHitMotionNV)",
				&s_operands[664],
				nullptr,
				14,
			},
			{
				SpirvOp::OpHitObjectRecordHitWithIndexMotionNV,
				R"(OpHitObjectRecordHitWithIndexMotionNV)",
				&s_operands[678],
				nullptr,
				13,
			},
			{
				SpirvOp::OpHitObjectRecordMissMotionNV,
				R"(OpHitObjectRecordMissMotionNV)",
				&s_operands[691],
				nullptr,
				7,
			},
			{
				SpirvOp::OpHitObjectGetWorldToObjectNV,
				R"(OpHitObjectGetWorldToObjectNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetObjectToWorldNV,
				R"(OpHitObjectGetObjectToWorldNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetObjectRayDirectionNV,
				R"(OpHitObjectGetObjectRayDirectionNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetObjectRayOriginNV,
				R"(OpHitObjectGetObjectRayOriginNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectTraceRayMotionNV,
				R"(OpHitObjectTraceRayMotionNV)",
				&s_operands[701],
				nullptr,
				13,
			},
			{
				SpirvOp::OpHitObjectGetShaderRecordBufferHandleNV,
				R"(OpHitObjectGetShaderRecordBufferHandleNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetShaderBindingTableRecordIndexNV,
				R"(OpHitObjectGetShaderBindingTableRecordIndexNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectRecordEmptyNV,
				R"(OpHitObjectRecordEmptyNV)",
				&s_operands[664],
				nullptr,
				1,
			},
			{
				SpirvOp::OpHitObjectTraceRayNV,
				R"(OpHitObjectTraceRayNV)",
				&s_operands[714],
				nullptr,
				12,
			},
			{
				SpirvOp::OpHitObjectRecordHitNV,
				R"(OpHitObjectRecordHitNV)",
				&s_operands[726],
				nullptr,
				13,
			},
			{
				SpirvOp::OpHitObjectRecordHitWithIndexNV,
				R"(OpHitObjectRecordHitWithIndexNV)",
				&s_operands[739],
				nullptr,
				12,
			},
			{
				SpirvOp::OpHitObjectRecordMissNV,
				R"(OpHitObjectRecordMissNV)",
				&s_operands[691],
				nullptr,
				6,
			},
			{
				SpirvOp::OpHitObjectExecuteShaderNV,
				R"(OpHitObjectExecuteShaderNV)",
				&s_operands[751],
				nullptr,
				2,
			},
			{
				SpirvOp::OpHitObjectGetCurrentTimeNV,
				R"(OpHitObjectGetCurrentTimeNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetAttributesNV,
				R"(OpHitObjectGetAttributesNV)",
				&s_operands[753],
				nullptr,
				2,
			},
			{
				SpirvOp::OpHitObjectGetHitKindNV,
				R"(OpHitObjectGetHitKindNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetPrimitiveIndexNV,
				R"(OpHitObjectGetPrimitiveIndexNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetGeometryIndexNV,
				R"(OpHitObjectGetGeometryIndexNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetInstanceIdNV,
				R"(OpHitObjectGetInstanceIdNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetInstanceCustomIndexNV,
				R"(OpHitObjectGetInstanceCustomIndexNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetWorldRayDirectionNV,
				R"(OpHitObjectGetWorldRayDirectionNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetWorldRayOriginNV,
				R"(OpHitObjectGetWorldRayOriginNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetRayTMaxNV,
				R"(OpHitObjectGetRayTMaxNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectGetRayTMinNV,
				R"(OpHitObjectGetRayTMinNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectIsEmptyNV,
				R"(OpHitObjectIsEmptyNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectIsHitNV,
				R"(OpHitObjectIsHitNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpHitObjectIsMissNV,
				R"(OpHitObjectIsMissNV)",
				&s_operands[698],
				&s_operands[699],
				3,
			},
			{
				SpirvOp::OpReorderThreadWithHitObjectNV,
				R"(OpReorderThreadWithHitObjectNV)",
				&s_operands[755],
				nullptr,
				3,
			},
			{
				SpirvOp::OpReorderThreadWithHintNV,
				R"(OpReorderThreadWithHintNV)",
				&s_operands[756],
				nullptr,
				2,
			},
			{
				SpirvOp::OpTypeHitObjectNV,
				R"(OpTypeHitObjectNV)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpImageSampleFootprintNV,
				R"(OpImageSampleFootprintNV)",
				&s_operands[758],
				&s_operands[759],
				7,
			},
			{
				SpirvOp::OpCooperativeMatrixConvertNV,
				R"(OpCooperativeMatrixConvertNV)",
				&s_operands[169],
				&s_operands[170],
				3,
			},
			{
				SpirvOp::OpEmitMeshTasksEXT,
				R"(OpEmitMeshTasksEXT)",
				&s_operands[765],
				nullptr,
				4,
			},
			{
				SpirvOp::OpSetMeshOutputsEXT,
				R"(OpSetMeshOutputsEXT)",
				&s_operands[769],
				nullptr,
				2,
			},
			{
				SpirvOp::OpGroupNonUniformPartitionNV,
				R"(OpGroupNonUniformPartitionNV)",
				&s_operands[215],
				&s_operands[216],
				3,
			},
			{
				SpirvOp::OpWritePackedPrimitiveIndices4x8NV,
				R"(OpWritePackedPrimitiveIndices4x8NV)",
				&s_operands[771],
				nullptr,
				2,
			},
			{
				SpirvOp::OpFetchMicroTriangleVertexPositionNV,
				R"(OpFetchMicroTriangleVertexPositionNV)",
				&s_operands[773],
				&s_operands[774],
				7,
			},
			{
				SpirvOp::OpFetchMicroTriangleVertexBarycentricNV,
				R"(OpFetchMicroTriangleVertexBarycentricNV)",
				&s_operands[773],
				&s_operands[774],
				7,
			},
			{
				SpirvOp::OpReportIntersectionKHR,
				R"(OpReportIntersectionKHR)",
				&s_operands[780],
				&s_operands[781],
				4,
			},
			{
				SpirvOp::OpIgnoreIntersectionNV,
				R"(OpIgnoreIntersectionNV)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpTerminateRayNV,
				R"(OpTerminateRayNV)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpTraceNV,
				R"(OpTraceNV)",
				&s_operands[784],
				nullptr,
				11,
			},
			{
				SpirvOp::OpTraceMotionNV,
				R"(OpTraceMotionNV)",
				&s_operands[795],
				nullptr,
				12,
			},
			{
				SpirvOp::OpTraceRayMotionNV,
				R"(OpTraceRayMotionNV)",
				&s_operands[807],
				nullptr,
				12,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionTriangleVertexPositionsKHR,
				R"(OpRayQueryGetIntersectionTriangleVertexPositionsKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpTypeAccelerationStructureKHR,
				R"(OpTypeAccelerationStructureKHR)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpExecuteCallableNV,
				R"(OpExecuteCallableNV)",
				&s_operands[819],
				nullptr,
				2,
			},
			{
				SpirvOp::OpTypeCooperativeMatrixNV,
				R"(OpTypeCooperativeMatrixNV)",
				&s_operands[821],
				&s_operands[821],
				5,
			},
			{
				SpirvOp::OpCooperativeMatrixLoadNV,
				R"(OpCooperativeMatrixLoadNV)",
				&s_operands[826],
				&s_operands[827],
				6,
			},
			{
				SpirvOp::OpCooperativeMatrixStoreNV,
				R"(OpCooperativeMatrixStoreNV)",
				&s_operands[832],
				nullptr,
				5,
			},
			{
				SpirvOp::OpCooperativeMatrixMulAddNV,
				R"(OpCooperativeMatrixMulAddNV)",
				&s_operands[586],
				&s_operands[587],
				5,
			},
			{
				SpirvOp::OpCooperativeMatrixLengthNV,
				R"(OpCooperativeMatrixLengthNV)",
				&s_operands[592],
				&s_operands[593],
				3,
			},
			{
				SpirvOp::OpBeginInvocationInterlockEXT,
				R"(OpBeginInvocationInterlockEXT)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpEndInvocationInterlockEXT,
				R"(OpEndInvocationInterlockEXT)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpCooperativeMatrixReduceNV,
				R"(OpCooperativeMatrixReduceNV)",
				&s_operands[837],
				&s_operands[838],
				5,
			},
			{
				SpirvOp::OpCooperativeMatrixLoadTensorNV,
				R"(OpCooperativeMatrixLoadTensorNV)",
				&s_operands[842],
				&s_operands[843],
				7,
			},
			{
				SpirvOp::OpCooperativeMatrixStoreTensorNV,
				R"(OpCooperativeMatrixStoreTensorNV)",
				&s_operands[844],
				nullptr,
				5,
			},
			{
				SpirvOp::OpCooperativeMatrixPerElementOpNV,
				R"(OpCooperativeMatrixPerElementOpNV)",
				&s_operands[849],
				&s_operands[850],
				5,
			},
			{
				SpirvOp::OpTypeTensorLayoutNV,
				R"(OpTypeTensorLayoutNV)",
				&s_operands[854],
				&s_operands[854],
				3,
			},
			{
				SpirvOp::OpTypeTensorViewNV,
				R"(OpTypeTensorViewNV)",
				&s_operands[857],
				&s_operands[857],
				4,
			},
			{
				SpirvOp::OpCreateTensorLayoutNV,
				R"(OpCreateTensorLayoutNV)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpTensorLayoutSetDimensionNV,
				R"(OpTensorLayoutSetDimensionNV)",
				&s_operands[861],
				&s_operands[862],
				4,
			},
			{
				SpirvOp::OpTensorLayoutSetStrideNV,
				R"(OpTensorLayoutSetStrideNV)",
				&s_operands[865],
				&s_operands[866],
				4,
			},
			{
				SpirvOp::OpTensorLayoutSliceNV,
				R"(OpTensorLayoutSliceNV)",
				&s_operands[869],
				&s_operands[870],
				4,
			},
			{
				SpirvOp::OpTensorLayoutSetClampValueNV,
				R"(OpTensorLayoutSetClampValueNV)",
				&s_operands[873],
				&s_operands[874],
				4,
			},
			{
				SpirvOp::OpCreateTensorViewNV,
				R"(OpCreateTensorViewNV)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpTensorViewSetDimensionNV,
				R"(OpTensorViewSetDimensionNV)",
				&s_operands[877],
				&s_operands[878],
				4,
			},
			{
				SpirvOp::OpTensorViewSetStrideNV,
				R"(OpTensorViewSetStrideNV)",
				&s_operands[881],
				&s_operands[882],
				4,
			},
			{
				SpirvOp::OpDemoteToHelperInvocation,
				R"(OpDemoteToHelperInvocation)",
				nullptr,
				nullptr,
				0,
			},
			{
				SpirvOp::OpIsHelperInvocationEXT,
				R"(OpIsHelperInvocationEXT)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpTensorViewSetClipNV,
				R"(OpTensorViewSetClipNV)",
				&s_operands[885],
				&s_operands[886],
				7,
			},
			{
				SpirvOp::OpTensorLayoutSetBlockSizeNV,
				R"(OpTensorLayoutSetBlockSizeNV)",
				&s_operands[892],
				&s_operands[893],
				4,
			},
			{
				SpirvOp::OpCooperativeMatrixTransposeNV,
				R"(OpCooperativeMatrixTransposeNV)",
				&s_operands[169],
				&s_operands[170],
				3,
			},
			{
				SpirvOp::OpConvertUToImageNV,
				R"(OpConvertUToImageNV)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpConvertUToSamplerNV,
				R"(OpConvertUToSamplerNV)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpConvertImageToUNV,
				R"(OpConvertImageToUNV)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpConvertSamplerToUNV,
				R"(OpConvertSamplerToUNV)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpConvertUToSampledImageNV,
				R"(OpConvertUToSampledImageNV)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpConvertSampledImageToUNV,
				R"(OpConvertSampledImageToUNV)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpSamplerImageAddressingModeNV,
				R"(OpSamplerImageAddressingModeNV)",
				&s_operands[896],
				nullptr,
				1,
			},
			{
				SpirvOp::OpRawAccessChainNV,
				R"(OpRawAccessChainNV)",
				&s_operands[897],
				&s_operands[898],
				7,
			},
			{
				SpirvOp::OpSubgroupShuffleINTEL,
				R"(OpSubgroupShuffleINTEL)",
				&s_operands[904],
				&s_operands[905],
				4,
			},
			{
				SpirvOp::OpSubgroupShuffleDownINTEL,
				R"(OpSubgroupShuffleDownINTEL)",
				&s_operands[908],
				&s_operands[909],
				5,
			},
			{
				SpirvOp::OpSubgroupShuffleUpINTEL,
				R"(OpSubgroupShuffleUpINTEL)",
				&s_operands[913],
				&s_operands[914],
				5,
			},
			{
				SpirvOp::OpSubgroupShuffleXorINTEL,
				R"(OpSubgroupShuffleXorINTEL)",
				&s_operands[918],
				&s_operands[919],
				4,
			},
			{
				SpirvOp::OpSubgroupBlockReadINTEL,
				R"(OpSubgroupBlockReadINTEL)",
				&s_operands[922],
				&s_operands[923],
				3,
			},
			{
				SpirvOp::OpSubgroupBlockWriteINTEL,
				R"(OpSubgroupBlockWriteINTEL)",
				&s_operands[925],
				nullptr,
				2,
			},
			{
				SpirvOp::OpSubgroupImageBlockReadINTEL,
				R"(OpSubgroupImageBlockReadINTEL)",
				&s_operands[100],
				&s_operands[101],
				4,
			},
			{
				SpirvOp::OpSubgroupImageBlockWriteINTEL,
				R"(OpSubgroupImageBlockWriteINTEL)",
				&s_operands[927],
				nullptr,
				3,
			},
			{
				SpirvOp::OpSubgroupImageMediaBlockReadINTEL,
				R"(OpSubgroupImageMediaBlockReadINTEL)",
				&s_operands[930],
				&s_operands[931],
				6,
			},
			{
				SpirvOp::OpSubgroupImageMediaBlockWriteINTEL,
				R"(OpSubgroupImageMediaBlockWriteINTEL)",
				&s_operands[936],
				nullptr,
				5,
			},
			{
				SpirvOp::OpUCountLeadingZerosINTEL,
				R"(OpUCountLeadingZerosINTEL)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpUCountTrailingZerosINTEL,
				R"(OpUCountTrailingZerosINTEL)",
				&s_operands[166],
				&s_operands[167],
				3,
			},
			{
				SpirvOp::OpAbsISubINTEL,
				R"(OpAbsISubINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpAbsUSubINTEL,
				R"(OpAbsUSubINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpIAddSatINTEL,
				R"(OpIAddSatINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUAddSatINTEL,
				R"(OpUAddSatINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpIAverageINTEL,
				R"(OpIAverageINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUAverageINTEL,
				R"(OpUAverageINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpIAverageRoundedINTEL,
				R"(OpIAverageRoundedINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUAverageRoundedINTEL,
				R"(OpUAverageRoundedINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpISubSatINTEL,
				R"(OpISubSatINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUSubSatINTEL,
				R"(OpUSubSatINTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpIMul32x16INTEL,
				R"(OpIMul32x16INTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpUMul32x16INTEL,
				R"(OpUMul32x16INTEL)",
				&s_operands[225],
				&s_operands[226],
				4,
			},
			{
				SpirvOp::OpConstantFunctionPointerINTEL,
				R"(OpConstantFunctionPointerINTEL)",
				&s_operands[92],
				&s_operands[93],
				3,
			},
			{
				SpirvOp::OpFunctionPointerCallINTEL,
				R"(OpFunctionPointerCallINTEL)",
				&s_operands[225],
				&s_operands[226],
				3,
			},
			{
				SpirvOp::OpAsmTargetINTEL,
				R"(OpAsmTargetINTEL)",
				&s_operands[941],
				&s_operands[942],
				3,
			},
			{
				SpirvOp::OpAsmINTEL,
				R"(OpAsmINTEL)",
				&s_operands[944],
				&s_operands[945],
				6,
			},
			{
				SpirvOp::OpAsmCallINTEL,
				R"(OpAsmCallINTEL)",
				&s_operands[950],
				&s_operands[951],
				4,
			},
			{
				SpirvOp::OpAtomicFMinEXT,
				R"(OpAtomicFMinEXT)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAtomicFMaxEXT,
				R"(OpAtomicFMaxEXT)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpAssumeTrueKHR,
				R"(OpAssumeTrueKHR)",
				&s_operands[258],
				nullptr,
				1,
			},
			{
				SpirvOp::OpExpectKHR,
				R"(OpExpectKHR)",
				&s_operands[954],
				&s_operands[955],
				4,
			},
			{
				SpirvOp::OpDecorateString,
				R"(OpDecorateString)",
				&s_operands[134],
				nullptr,
				2,
			},
			{
				SpirvOp::OpMemberDecorateString,
				R"(OpMemberDecorateString)",
				&s_operands[958],
				nullptr,
				3,
			},
			{
				SpirvOp::OpVmeImageINTEL,
				R"(OpVmeImageINTEL)",
				&s_operands[961],
				&s_operands[962],
				4,
			},
			{
				SpirvOp::OpTypeVmeImageINTEL,
				R"(OpTypeVmeImageINTEL)",
				&s_operands[55],
				&s_operands[55],
				2,
			},
			{
				SpirvOp::OpTypeAvcImePayloadINTEL,
				R"(OpTypeAvcImePayloadINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcRefPayloadINTEL,
				R"(OpTypeAvcRefPayloadINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcSicPayloadINTEL,
				R"(OpTypeAvcSicPayloadINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcMcePayloadINTEL,
				R"(OpTypeAvcMcePayloadINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcMceResultINTEL,
				R"(OpTypeAvcMceResultINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcImeResultINTEL,
				R"(OpTypeAvcImeResultINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcImeResultSingleReferenceStreamoutINTEL,
				R"(OpTypeAvcImeResultSingleReferenceStreamoutINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcImeResultDualReferenceStreamoutINTEL,
				R"(OpTypeAvcImeResultDualReferenceStreamoutINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcImeSingleReferenceStreaminINTEL,
				R"(OpTypeAvcImeSingleReferenceStreaminINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcImeDualReferenceStreaminINTEL,
				R"(OpTypeAvcImeDualReferenceStreaminINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcRefResultINTEL,
				R"(OpTypeAvcRefResultINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpTypeAvcSicResultINTEL,
				R"(OpTypeAvcSicResultINTEL)",
				&s_operands[1],
				&s_operands[1],
				1,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL,
				R"(OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL)",
				&s_operands[965],
				&s_operands[966],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL,
				R"(OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL)",
				&s_operands[969],
				&s_operands[970],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL,
				R"(OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL)",
				&s_operands[965],
				&s_operands[966],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetInterShapePenaltyINTEL,
				R"(OpSubgroupAvcMceSetInterShapePenaltyINTEL)",
				&s_operands[973],
				&s_operands[974],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL,
				R"(OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL)",
				&s_operands[965],
				&s_operands[966],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL,
				R"(OpSubgroupAvcMceSetInterDirectionPenaltyINTEL)",
				&s_operands[977],
				&s_operands[978],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL,
				R"(OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL)",
				&s_operands[965],
				&s_operands[966],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL,
				R"(OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL)",
				&s_operands[965],
				&s_operands[966],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL,
				R"(OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL,
				R"(OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL,
				R"(OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL,
				R"(OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL)",
				&s_operands[981],
				&s_operands[982],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL,
				R"(OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL)",
				&s_operands[965],
				&s_operands[966],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL,
				R"(OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL,
				R"(OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetAcOnlyHaarINTEL,
				R"(OpSubgroupAvcMceSetAcOnlyHaarINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL,
				R"(OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL)",
				&s_operands[987],
				&s_operands[988],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL,
				R"(OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL)",
				&s_operands[991],
				&s_operands[992],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL,
				R"(OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL)",
				&s_operands[995],
				&s_operands[996],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcMceConvertToImePayloadINTEL,
				R"(OpSubgroupAvcMceConvertToImePayloadINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceConvertToImeResultINTEL,
				R"(OpSubgroupAvcMceConvertToImeResultINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceConvertToRefPayloadINTEL,
				R"(OpSubgroupAvcMceConvertToRefPayloadINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceConvertToRefResultINTEL,
				R"(OpSubgroupAvcMceConvertToRefResultINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceConvertToSicPayloadINTEL,
				R"(OpSubgroupAvcMceConvertToSicPayloadINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceConvertToSicResultINTEL,
				R"(OpSubgroupAvcMceConvertToSicResultINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetMotionVectorsINTEL,
				R"(OpSubgroupAvcMceGetMotionVectorsINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetInterDistortionsINTEL,
				R"(OpSubgroupAvcMceGetInterDistortionsINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetBestInterDistortionsINTEL,
				R"(OpSubgroupAvcMceGetBestInterDistortionsINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetInterMajorShapeINTEL,
				R"(OpSubgroupAvcMceGetInterMajorShapeINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetInterMinorShapeINTEL,
				R"(OpSubgroupAvcMceGetInterMinorShapeINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetInterDirectionsINTEL,
				R"(OpSubgroupAvcMceGetInterDirectionsINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetInterMotionVectorCountINTEL,
				R"(OpSubgroupAvcMceGetInterMotionVectorCountINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetInterReferenceIdsINTEL,
				R"(OpSubgroupAvcMceGetInterReferenceIdsINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL,
				R"(OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL)",
				&s_operands[1000],
				&s_operands[1001],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeInitializeINTEL,
				R"(OpSubgroupAvcImeInitializeINTEL)",
				&s_operands[1005],
				&s_operands[1006],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeSetSingleReferenceINTEL,
				R"(OpSubgroupAvcImeSetSingleReferenceINTEL)",
				&s_operands[1010],
				&s_operands[1011],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeSetDualReferenceINTEL,
				R"(OpSubgroupAvcImeSetDualReferenceINTEL)",
				&s_operands[1015],
				&s_operands[1016],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcImeRefWindowSizeINTEL,
				R"(OpSubgroupAvcImeRefWindowSizeINTEL)",
				&s_operands[1021],
				&s_operands[1022],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeAdjustRefOffsetINTEL,
				R"(OpSubgroupAvcImeAdjustRefOffsetINTEL)",
				&s_operands[1025],
				&s_operands[1026],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcImeConvertToMcePayloadINTEL,
				R"(OpSubgroupAvcImeConvertToMcePayloadINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL,
				R"(OpSubgroupAvcImeSetMaxMotionVectorCountINTEL)",
				&s_operands[1031],
				&s_operands[1032],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL,
				R"(OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL,
				R"(OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL)",
				&s_operands[1035],
				&s_operands[1036],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeSetWeightedSadINTEL,
				R"(OpSubgroupAvcImeSetWeightedSadINTEL)",
				&s_operands[1039],
				&s_operands[1040],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL,
				R"(OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL)",
				&s_operands[1043],
				&s_operands[1044],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL,
				R"(OpSubgroupAvcImeEvaluateWithDualReferenceINTEL)",
				&s_operands[1048],
				&s_operands[1049],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL,
				R"(OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL)",
				&s_operands[1054],
				&s_operands[1055],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL,
				R"(OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL)",
				&s_operands[1060],
				&s_operands[1061],
				7,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL,
				R"(OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL)",
				&s_operands[1043],
				&s_operands[1044],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL,
				R"(OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL)",
				&s_operands[1048],
				&s_operands[1049],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL,
				R"(OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL)",
				&s_operands[1054],
				&s_operands[1055],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL,
				R"(OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL)",
				&s_operands[1060],
				&s_operands[1061],
				7,
			},
			{
				SpirvOp::OpSubgroupAvcImeConvertToMceResultINTEL,
				R"(OpSubgroupAvcImeConvertToMceResultINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL,
				R"(OpSubgroupAvcImeGetSingleReferenceStreaminINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetDualReferenceStreaminINTEL,
				R"(OpSubgroupAvcImeGetDualReferenceStreaminINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL,
				R"(OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL,
				R"(OpSubgroupAvcImeStripDualReferenceStreamoutINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL,
				R"(OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL)",
				&s_operands[1067],
				&s_operands[1068],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL,
				R"(OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL)",
				&s_operands[1067],
				&s_operands[1068],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL,
				R"(OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL)",
				&s_operands[1067],
				&s_operands[1068],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL,
				R"(OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL)",
				&s_operands[1071],
				&s_operands[1072],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL,
				R"(OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL)",
				&s_operands[1071],
				&s_operands[1072],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL,
				R"(OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL)",
				&s_operands[1071],
				&s_operands[1072],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetBorderReachedINTEL,
				R"(OpSubgroupAvcImeGetBorderReachedINTEL)",
				&s_operands[1076],
				&s_operands[1077],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL,
				R"(OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL,
				R"(OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL,
				R"(OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL,
				R"(OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcFmeInitializeINTEL,
				R"(OpSubgroupAvcFmeInitializeINTEL)",
				&s_operands[1080],
				&s_operands[1081],
				9,
			},
			{
				SpirvOp::OpSubgroupAvcBmeInitializeINTEL,
				R"(OpSubgroupAvcBmeInitializeINTEL)",
				&s_operands[1089],
				&s_operands[1090],
				10,
			},
			{
				SpirvOp::OpSubgroupAvcRefConvertToMcePayloadINTEL,
				R"(OpSubgroupAvcRefConvertToMcePayloadINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL,
				R"(OpSubgroupAvcRefSetBidirectionalMixDisableINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcRefSetBilinearFilterEnableINTEL,
				R"(OpSubgroupAvcRefSetBilinearFilterEnableINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL,
				R"(OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL)",
				&s_operands[1043],
				&s_operands[1044],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL,
				R"(OpSubgroupAvcRefEvaluateWithDualReferenceINTEL)",
				&s_operands[1048],
				&s_operands[1049],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL,
				R"(OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL)",
				&s_operands[1099],
				&s_operands[1100],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL,
				R"(OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL)",
				&s_operands[1104],
				&s_operands[1105],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcRefConvertToMceResultINTEL,
				R"(OpSubgroupAvcRefConvertToMceResultINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicInitializeINTEL,
				R"(OpSubgroupAvcSicInitializeINTEL)",
				&s_operands[1005],
				&s_operands[1006],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicConfigureSkcINTEL,
				R"(OpSubgroupAvcSicConfigureSkcINTEL)",
				&s_operands[1110],
				&s_operands[1111],
				8,
			},
			{
				SpirvOp::OpSubgroupAvcSicConfigureIpeLumaINTEL,
				R"(OpSubgroupAvcSicConfigureIpeLumaINTEL)",
				&s_operands[1118],
				&s_operands[1119],
				10,
			},
			{
				SpirvOp::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL,
				R"(OpSubgroupAvcSicConfigureIpeLumaChromaINTEL)",
				&s_operands[1128],
				&s_operands[1129],
				13,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetMotionVectorMaskINTEL,
				R"(OpSubgroupAvcSicGetMotionVectorMaskINTEL)",
				&s_operands[1141],
				&s_operands[1142],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcSicConvertToMcePayloadINTEL,
				R"(OpSubgroupAvcSicConvertToMcePayloadINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL,
				R"(OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL)",
				&s_operands[973],
				&s_operands[974],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL,
				R"(OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL)",
				&s_operands[1145],
				&s_operands[1146],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL,
				R"(OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL)",
				&s_operands[1151],
				&s_operands[1152],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcSicSetBilinearFilterEnableINTEL,
				R"(OpSubgroupAvcSicSetBilinearFilterEnableINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL,
				R"(OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL)",
				&s_operands[1155],
				&s_operands[1156],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL,
				R"(OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL)",
				&s_operands[1159],
				&s_operands[1160],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcSicEvaluateIpeINTEL,
				R"(OpSubgroupAvcSicEvaluateIpeINTEL)",
				&s_operands[1163],
				&s_operands[1164],
				4,
			},
			{
				SpirvOp::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL,
				R"(OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL)",
				&s_operands[1043],
				&s_operands[1044],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL,
				R"(OpSubgroupAvcSicEvaluateWithDualReferenceINTEL)",
				&s_operands[1048],
				&s_operands[1049],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL,
				R"(OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL)",
				&s_operands[1099],
				&s_operands[1100],
				5,
			},
			{
				SpirvOp::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL,
				R"(OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL)",
				&s_operands[1104],
				&s_operands[1105],
				6,
			},
			{
				SpirvOp::OpSubgroupAvcSicConvertToMceResultINTEL,
				R"(OpSubgroupAvcSicConvertToMceResultINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetIpeLumaShapeINTEL,
				R"(OpSubgroupAvcSicGetIpeLumaShapeINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL,
				R"(OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL,
				R"(OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL,
				R"(OpSubgroupAvcSicGetPackedIpeLumaModesINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetIpeChromaModeINTEL,
				R"(OpSubgroupAvcSicGetIpeChromaModeINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL,
				R"(OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL,
				R"(OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpSubgroupAvcSicGetInterRawSadsINTEL,
				R"(OpSubgroupAvcSicGetInterRawSadsINTEL)",
				&s_operands[652],
				&s_operands[653],
				3,
			},
			{
				SpirvOp::OpVariableLengthArrayINTEL,
				R"(OpVariableLengthArrayINTEL)",
				&s_operands[1167],
				&s_operands[1168],
				3,
			},
			{
				SpirvOp::OpSaveMemoryINTEL,
				R"(OpSaveMemoryINTEL)",
				&s_operands[0],
				&s_operands[1],
				2,
			},
			{
				SpirvOp::OpRestoreMemoryINTEL,
				R"(OpRestoreMemoryINTEL)",
				&s_operands[924],
				nullptr,
				1,
			},
			{
				SpirvOp::OpArbitraryFloatSinCosPiINTEL,
				R"(OpArbitraryFloatSinCosPiINTEL)",
				&s_operands[1170],
				&s_operands[1171],
				9,
			},
			{
				SpirvOp::OpArbitraryFloatCastINTEL,
				R"(OpArbitraryFloatCastINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatCastFromIntINTEL,
				R"(OpArbitraryFloatCastFromIntINTEL)",
				&s_operands[1187],
				&s_operands[1188],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatCastToIntINTEL,
				R"(OpArbitraryFloatCastToIntINTEL)",
				&s_operands[1195],
				&s_operands[1196],
				7,
			},
			{
				SpirvOp::OpArbitraryFloatAddINTEL,
				R"(OpArbitraryFloatAddINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatSubINTEL,
				R"(OpArbitraryFloatSubINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatMulINTEL,
				R"(OpArbitraryFloatMulINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatDivINTEL,
				R"(OpArbitraryFloatDivINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatGTINTEL,
				R"(OpArbitraryFloatGTINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				6,
			},
			{
				SpirvOp::OpArbitraryFloatGEINTEL,
				R"(OpArbitraryFloatGEINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				6,
			},
			{
				SpirvOp::OpArbitraryFloatLTINTEL,
				R"(OpArbitraryFloatLTINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				6,
			},
			{
				SpirvOp::OpArbitraryFloatLEINTEL,
				R"(OpArbitraryFloatLEINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				6,
			},
			{
				SpirvOp::OpArbitraryFloatEQINTEL,
				R"(OpArbitraryFloatEQINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				6,
			},
			{
				SpirvOp::OpArbitraryFloatRecipINTEL,
				R"(OpArbitraryFloatRecipINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatRSqrtINTEL,
				R"(OpArbitraryFloatRSqrtINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatCbrtINTEL,
				R"(OpArbitraryFloatCbrtINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatHypotINTEL,
				R"(OpArbitraryFloatHypotINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatSqrtINTEL,
				R"(OpArbitraryFloatSqrtINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatLogINTEL,
				R"(OpArbitraryFloatLogINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatLog2INTEL,
				R"(OpArbitraryFloatLog2INTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatLog10INTEL,
				R"(OpArbitraryFloatLog10INTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatLog1pINTEL,
				R"(OpArbitraryFloatLog1pINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatExpINTEL,
				R"(OpArbitraryFloatExpINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatExp2INTEL,
				R"(OpArbitraryFloatExp2INTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatExp10INTEL,
				R"(OpArbitraryFloatExp10INTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatExpm1INTEL,
				R"(OpArbitraryFloatExpm1INTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatSinINTEL,
				R"(OpArbitraryFloatSinINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatCosINTEL,
				R"(OpArbitraryFloatCosINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatSinCosINTEL,
				R"(OpArbitraryFloatSinCosINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatSinPiINTEL,
				R"(OpArbitraryFloatSinPiINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatCosPiINTEL,
				R"(OpArbitraryFloatCosPiINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatASinINTEL,
				R"(OpArbitraryFloatASinINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatASinPiINTEL,
				R"(OpArbitraryFloatASinPiINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatACosINTEL,
				R"(OpArbitraryFloatACosINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatACosPiINTEL,
				R"(OpArbitraryFloatACosPiINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatATanINTEL,
				R"(OpArbitraryFloatATanINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatATanPiINTEL,
				R"(OpArbitraryFloatATanPiINTEL)",
				&s_operands[1179],
				&s_operands[1180],
				8,
			},
			{
				SpirvOp::OpArbitraryFloatATan2INTEL,
				R"(OpArbitraryFloatATan2INTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatPowINTEL,
				R"(OpArbitraryFloatPowINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatPowRINTEL,
				R"(OpArbitraryFloatPowRINTEL)",
				&s_operands[1202],
				&s_operands[1203],
				10,
			},
			{
				SpirvOp::OpArbitraryFloatPowNINTEL,
				R"(OpArbitraryFloatPowNINTEL)",
				&s_operands[1212],
				&s_operands[1213],
				9,
			},
			{
				SpirvOp::OpLoopControlINTEL,
				R"(OpLoopControlINTEL)",
				&s_operands[1221],
				nullptr,
				1,
			},
			{
				SpirvOp::OpAliasDomainDeclINTEL,
				R"(OpAliasDomainDeclINTEL)",
				&s_operands[1222],
				&s_operands[1222],
				2,
			},
			{
				SpirvOp::OpAliasScopeDeclINTEL,
				R"(OpAliasScopeDeclINTEL)",
				&s_operands[1224],
				&s_operands[1224],
				3,
			},
			{
				SpirvOp::OpAliasScopeListDeclINTEL,
				R"(OpAliasScopeListDeclINTEL)",
				&s_operands[1227],
				&s_operands[1227],
				2,
			},
			{
				SpirvOp::OpFixedSqrtINTEL,
				R"(OpFixedSqrtINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedRecipINTEL,
				R"(OpFixedRecipINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedRsqrtINTEL,
				R"(OpFixedRsqrtINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedSinINTEL,
				R"(OpFixedSinINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedCosINTEL,
				R"(OpFixedCosINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedSinCosINTEL,
				R"(OpFixedSinCosINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedSinPiINTEL,
				R"(OpFixedSinPiINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedCosPiINTEL,
				R"(OpFixedCosPiINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedSinCosPiINTEL,
				R"(OpFixedSinCosPiINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedLogINTEL,
				R"(OpFixedLogINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpFixedExpINTEL,
				R"(OpFixedExpINTEL)",
				&s_operands[1229],
				&s_operands[1230],
				9,
			},
			{
				SpirvOp::OpPtrCastToCrossWorkgroupINTEL,
				R"(OpPtrCastToCrossWorkgroupINTEL)",
				&s_operands[105],
				&s_operands[106],
				3,
			},
			{
				SpirvOp::OpCrossWorkgroupCastToPtrINTEL,
				R"(OpCrossWorkgroupCastToPtrINTEL)",
				&s_operands[105],
				&s_operands[106],
				3,
			},
			{
				SpirvOp::OpReadPipeBlockingINTEL,
				R"(OpReadPipeBlockingINTEL)",
				&s_operands[1238],
				&s_operands[1239],
				4,
			},
			{
				SpirvOp::OpWritePipeBlockingINTEL,
				R"(OpWritePipeBlockingINTEL)",
				&s_operands[1238],
				&s_operands[1239],
				4,
			},
			{
				SpirvOp::OpFPGARegINTEL,
				R"(OpFPGARegINTEL)",
				&s_operands[1242],
				&s_operands[1243],
				4,
			},
			{
				SpirvOp::OpRayQueryGetRayTMinKHR,
				R"(OpRayQueryGetRayTMinKHR)",
				&s_operands[605],
				&s_operands[606],
				3,
			},
			{
				SpirvOp::OpRayQueryGetRayFlagsKHR,
				R"(OpRayQueryGetRayFlagsKHR)",
				&s_operands[605],
				&s_operands[606],
				3,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionTKHR,
				R"(OpRayQueryGetIntersectionTKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionInstanceCustomIndexKHR,
				R"(OpRayQueryGetIntersectionInstanceCustomIndexKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionInstanceIdKHR,
				R"(OpRayQueryGetIntersectionInstanceIdKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR,
				R"(OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionGeometryIndexKHR,
				R"(OpRayQueryGetIntersectionGeometryIndexKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionPrimitiveIndexKHR,
				R"(OpRayQueryGetIntersectionPrimitiveIndexKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionBarycentricsKHR,
				R"(OpRayQueryGetIntersectionBarycentricsKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionFrontFaceKHR,
				R"(OpRayQueryGetIntersectionFrontFaceKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR,
				R"(OpRayQueryGetIntersectionCandidateAABBOpaqueKHR)",
				&s_operands[605],
				&s_operands[606],
				3,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionObjectRayDirectionKHR,
				R"(OpRayQueryGetIntersectionObjectRayDirectionKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionObjectRayOriginKHR,
				R"(OpRayQueryGetIntersectionObjectRayOriginKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetWorldRayDirectionKHR,
				R"(OpRayQueryGetWorldRayDirectionKHR)",
				&s_operands[605],
				&s_operands[606],
				3,
			},
			{
				SpirvOp::OpRayQueryGetWorldRayOriginKHR,
				R"(OpRayQueryGetWorldRayOriginKHR)",
				&s_operands[605],
				&s_operands[606],
				3,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionObjectToWorldKHR,
				R"(OpRayQueryGetIntersectionObjectToWorldKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpRayQueryGetIntersectionWorldToObjectKHR,
				R"(OpRayQueryGetIntersectionWorldToObjectKHR)",
				&s_operands[608],
				&s_operands[609],
				4,
			},
			{
				SpirvOp::OpAtomicFAddEXT,
				R"(OpAtomicFAddEXT)",
				&s_operands[292],
				&s_operands[293],
				6,
			},
			{
				SpirvOp::OpTypeBufferSurfaceINTEL,
				R"(OpTypeBufferSurfaceINTEL)",
				&s_operands[1246],
				&s_operands[1246],
				2,
			},
			{
				SpirvOp::OpTypeStructContinuedINTEL,
				R"(OpTypeStructContinuedINTEL)",
				&s_operands[61],
				nullptr,
				1,
			},
			{
				SpirvOp::OpConstantCompositeContinuedINTEL,
				R"(OpConstantCompositeContinuedINTEL)",
				&s_operands[79],
				nullptr,
				1,
			},
			{
				SpirvOp::OpSpecConstantCompositeContinuedINTEL,
				R"(OpSpecConstantCompositeContinuedINTEL)",
				&s_operands[79],
				nullptr,
				1,
			},
			{
				SpirvOp::OpCompositeConstructContinuedINTEL,
				R"(OpCompositeConstructContinuedINTEL)",
				&s_operands[77],
				&s_operands[78],
				3,
			},
			{
				SpirvOp::OpConvertFToBF16INTEL,
				R"(OpConvertFToBF16INTEL)",
				&s_operands[206],
				&s_operands[207],
				3,
			},
			{
				SpirvOp::OpConvertBF16ToFINTEL,
				R"(OpConvertBF16ToFINTEL)",
				&s_operands[1248],
				&s_operands[1249],
				3,
			},
			{
				SpirvOp::OpControlBarrierArriveINTEL,
				R"(OpControlBarrierArriveINTEL)",
				&s_operands[280],
				nullptr,
				3,
			},
			{
				SpirvOp::OpControlBarrierWaitINTEL,
				R"(OpControlBarrierWaitINTEL)",
				&s_operands[280],
				nullptr,
				3,
			},
			{
				SpirvOp::OpArithmeticFenceEXT,
				R"(OpArithmeticFenceEXT)",
				&s_operands[1251],
				&s_operands[1252],
				3,
			},
			{
				SpirvOp::OpSubgroupBlockPrefetchINTEL,
				R"(OpSubgroupBlockPrefetchINTEL)",
				&s_operands[1254],
				nullptr,
				3,
			},
			{
				SpirvOp::OpGroupIMulKHR,
				R"(OpGroupIMulKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupFMulKHR,
				R"(OpGroupFMulKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupBitwiseAndKHR,
				R"(OpGroupBitwiseAndKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupBitwiseOrKHR,
				R"(OpGroupBitwiseOrKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupBitwiseXorKHR,
				R"(OpGroupBitwiseXorKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupLogicalAndKHR,
				R"(OpGroupLogicalAndKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupLogicalOrKHR,
				R"(OpGroupLogicalOrKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpGroupLogicalXorKHR,
				R"(OpGroupLogicalXorKHR)",
				&s_operands[344],
				&s_operands[345],
				5,
			},
			{
				SpirvOp::OpMaskedGatherINTEL,
				R"(OpMaskedGatherINTEL)",
				&s_operands[1257],
				&s_operands[1258],
				6,
			},
			{
				SpirvOp::OpMaskedScatterINTEL,
				R"(OpMaskedScatterINTEL)",
				&s_operands[1263],
				nullptr,
				4,
			},
		}
	};

	static std::array<SpirvGlslStd450Instruction, 81> s_instructionsGlslStd450 = {
		{
			{
				SpirvGlslStd450Op::Round,
				R"(Round)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::RoundEven,
				R"(RoundEven)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Trunc,
				R"(Trunc)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::FAbs,
				R"(FAbs)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::SAbs,
				R"(SAbs)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::FSign,
				R"(FSign)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::SSign,
				R"(SSign)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Floor,
				R"(Floor)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Ceil,
				R"(Ceil)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Fract,
				R"(Fract)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Radians,
				R"(Radians)",
				&s_operands[1267],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Degrees,
				R"(Degrees)",
				&s_operands[1268],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Sin,
				R"(Sin)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Cos,
				R"(Cos)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Tan,
				R"(Tan)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Asin,
				R"(Asin)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Acos,
				R"(Acos)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Atan,
				R"(Atan)",
				&s_operands[1269],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Sinh,
				R"(Sinh)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Cosh,
				R"(Cosh)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Tanh,
				R"(Tanh)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Asinh,
				R"(Asinh)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Acosh,
				R"(Acosh)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Atanh,
				R"(Atanh)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Atan2,
				R"(Atan2)",
				&s_operands[1270],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::Pow,
				R"(Pow)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::Exp,
				R"(Exp)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Log,
				R"(Log)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Exp2,
				R"(Exp2)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Log2,
				R"(Log2)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Sqrt,
				R"(Sqrt)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::InverseSqrt,
				R"(InverseSqrt)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Determinant,
				R"(Determinant)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::MatrixInverse,
				R"(MatrixInverse)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Modf,
				R"(Modf)",
				&s_operands[1272],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::ModfStruct,
				R"(ModfStruct)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::FMin,
				R"(FMin)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::UMin,
				R"(UMin)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::SMin,
				R"(SMin)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::FMax,
				R"(FMax)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::UMax,
				R"(UMax)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::SMax,
				R"(SMax)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::FClamp,
				R"(FClamp)",
				&s_operands[1274],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::UClamp,
				R"(UClamp)",
				&s_operands[1274],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::SClamp,
				R"(SClamp)",
				&s_operands[1274],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::FMix,
				R"(FMix)",
				&s_operands[1277],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::IMix,
				R"(IMix)",
				&s_operands[1277],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::Step,
				R"(Step)",
				&s_operands[1280],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::SmoothStep,
				R"(SmoothStep)",
				&s_operands[1282],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::Fma,
				R"(Fma)",
				&s_operands[1285],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::Frexp,
				R"(Frexp)",
				&s_operands[1288],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::FrexpStruct,
				R"(FrexpStruct)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Ldexp,
				R"(Ldexp)",
				&s_operands[1288],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::PackSnorm4x8,
				R"(PackSnorm4x8)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::PackUnorm4x8,
				R"(PackUnorm4x8)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::PackSnorm2x16,
				R"(PackSnorm2x16)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::PackUnorm2x16,
				R"(PackUnorm2x16)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::PackHalf2x16,
				R"(PackHalf2x16)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::PackDouble2x32,
				R"(PackDouble2x32)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::UnpackSnorm2x16,
				R"(UnpackSnorm2x16)",
				&s_operands[860],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::UnpackUnorm2x16,
				R"(UnpackUnorm2x16)",
				&s_operands[860],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::UnpackHalf2x16,
				R"(UnpackHalf2x16)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::UnpackSnorm4x8,
				R"(UnpackSnorm4x8)",
				&s_operands[860],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::UnpackUnorm4x8,
				R"(UnpackUnorm4x8)",
				&s_operands[860],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::UnpackDouble2x32,
				R"(UnpackDouble2x32)",
				&s_operands[1290],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Length,
				R"(Length)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::Distance,
				R"(Distance)",
				&s_operands[1291],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::Cross,
				R"(Cross)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::Normalize,
				R"(Normalize)",
				&s_operands[251],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::FaceForward,
				R"(FaceForward)",
				&s_operands[1293],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::Reflect,
				R"(Reflect)",
				&s_operands[1296],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::Refract,
				R"(Refract)",
				&s_operands[1298],
				nullptr,
				3,
			},
			{
				SpirvGlslStd450Op::FindILsb,
				R"(FindILsb)",
				&s_operands[217],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::FindSMsb,
				R"(FindSMsb)",
				&s_operands[217],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::FindUMsb,
				R"(FindUMsb)",
				&s_operands[217],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::InterpolateAtCentroid,
				R"(InterpolateAtCentroid)",
				&s_operands[1301],
				nullptr,
				1,
			},
			{
				SpirvGlslStd450Op::InterpolateAtSample,
				R"(InterpolateAtSample)",
				&s_operands[1302],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::InterpolateAtOffset,
				R"(InterpolateAtOffset)",
				&s_operands[1304],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::NMin,
				R"(NMin)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::NMax,
				R"(NMax)",
				&s_operands[254],
				nullptr,
				2,
			},
			{
				SpirvGlslStd450Op::NClamp,
				R"(NClamp)",
				&s_operands[1274],
				nullptr,
				3,
			},
		}
	};


	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvAccessQualifier kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvAddressingModel kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvBuiltIn kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvCapability kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvCooperativeMatrixLayout kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvCooperativeMatrixUse kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvDecoration kind)
	{
		switch(kind)
		{
			case SpirvDecoration::SpecId:
				return { &s_operands[1342], 1 };
			case SpirvDecoration::ArrayStride:
				return { &s_operands[1343], 1 };
			case SpirvDecoration::MatrixStride:
				return { &s_operands[1344], 1 };
			case SpirvDecoration::BuiltIn:
				return { &s_operands[1345], 1 };
			case SpirvDecoration::UniformId:
				return { &s_operands[280], 1 };
			case SpirvDecoration::Stream:
				return { &s_operands[1346], 1 };
			case SpirvDecoration::Location:
				return { &s_operands[1347], 1 };
			case SpirvDecoration::Component:
				return { &s_operands[1348], 1 };
			case SpirvDecoration::Index:
				return { &s_operands[1349], 1 };
			case SpirvDecoration::Binding:
				return { &s_operands[1350], 1 };
			case SpirvDecoration::DescriptorSet:
				return { &s_operands[1351], 1 };
			case SpirvDecoration::Offset:
				return { &s_operands[1352], 1 };
			case SpirvDecoration::XfbBuffer:
				return { &s_operands[1353], 1 };
			case SpirvDecoration::XfbStride:
				return { &s_operands[1354], 1 };
			case SpirvDecoration::FuncParamAttr:
				return { &s_operands[1355], 1 };
			case SpirvDecoration::FPRoundingMode:
				return { &s_operands[1356], 1 };
			case SpirvDecoration::FPFastMathMode:
				return { &s_operands[1357], 1 };
			case SpirvDecoration::LinkageAttributes:
				return { &s_operands[1358], 2 };
			case SpirvDecoration::InputAttachmentIndex:
				return { &s_operands[1360], 1 };
			case SpirvDecoration::Alignment:
				return { &s_operands[1260], 1 };
			case SpirvDecoration::MaxByteOffset:
				return { &s_operands[1361], 1 };
			case SpirvDecoration::AlignmentId:
				return { &s_operands[1362], 1 };
			case SpirvDecoration::MaxByteOffsetId:
				return { &s_operands[1363], 1 };
			case SpirvDecoration::NodeSharesPayloadLimitsWithAMDX:
				return { &s_operands[651], 1 };
			case SpirvDecoration::NodeMaxPayloadsAMDX:
				return { &s_operands[1364], 1 };
			case SpirvDecoration::PayloadNodeNameAMDX:
				return { &s_operands[1325], 1 };
			case SpirvDecoration::PayloadNodeBaseIndexAMDX:
				return { &s_operands[1365], 1 };
			case SpirvDecoration::PayloadNodeArraySizeAMDX:
				return { &s_operands[1366], 1 };
			case SpirvDecoration::SecondaryViewportRelativeNV:
				return { &s_operands[1367], 1 };
			case SpirvDecoration::SIMTCallINTEL:
				return { &s_operands[1368], 1 };
			case SpirvDecoration::ClobberINTEL:
				return { &s_operands[1369], 1 };
			case SpirvDecoration::FuncParamIOKindINTEL:
				return { &s_operands[1370], 1 };
			case SpirvDecoration::GlobalVariableOffsetINTEL:
				return { &s_operands[1367], 1 };
			case SpirvDecoration::CounterBuffer:
				return { &s_operands[1371], 1 };
			case SpirvDecoration::UserSemantic:
				return { &s_operands[1372], 1 };
			case SpirvDecoration::UserTypeGOOGLE:
				return { &s_operands[1373], 1 };
			case SpirvDecoration::FunctionRoundingModeINTEL:
				return { &s_operands[1374], 2 };
			case SpirvDecoration::FunctionDenormModeINTEL:
				return { &s_operands[1376], 2 };
			case SpirvDecoration::MemoryINTEL:
				return { &s_operands[1378], 1 };
			case SpirvDecoration::NumbanksINTEL:
				return { &s_operands[1379], 1 };
			case SpirvDecoration::BankwidthINTEL:
				return { &s_operands[1380], 1 };
			case SpirvDecoration::MaxPrivateCopiesINTEL:
				return { &s_operands[1381], 1 };
			case SpirvDecoration::MaxReplicatesINTEL:
				return { &s_operands[1382], 1 };
			case SpirvDecoration::MergeINTEL:
				return { &s_operands[1383], 2 };
			case SpirvDecoration::BankBitsINTEL:
				return { &s_operands[1385], 1 };
			case SpirvDecoration::ForcePow2DepthINTEL:
				return { &s_operands[1386], 1 };
			case SpirvDecoration::StridesizeINTEL:
				return { &s_operands[1387], 1 };
			case SpirvDecoration::WordsizeINTEL:
				return { &s_operands[1388], 1 };
			case SpirvDecoration::CacheSizeINTEL:
				return { &s_operands[1389], 1 };
			case SpirvDecoration::PrefetchINTEL:
				return { &s_operands[1390], 1 };
			case SpirvDecoration::MathOpDSPModeINTEL:
				return { &s_operands[1391], 2 };
			case SpirvDecoration::AliasScopeINTEL:
				return { &s_operands[1393], 1 };
			case SpirvDecoration::NoAliasINTEL:
				return { &s_operands[1393], 1 };
			case SpirvDecoration::InitiationIntervalINTEL:
				return { &s_operands[1394], 1 };
			case SpirvDecoration::MaxConcurrencyINTEL:
				return { &s_operands[1395], 1 };
			case SpirvDecoration::PipelineEnableINTEL:
				return { &s_operands[1396], 1 };
			case SpirvDecoration::BufferLocationINTEL:
				return { &s_operands[1397], 1 };
			case SpirvDecoration::IOPipeStorageINTEL:
				return { &s_operands[1398], 1 };
			case SpirvDecoration::FunctionFloatingPointModeINTEL:
				return { &s_operands[1399], 2 };
			case SpirvDecoration::FPMaxErrorDecorationINTEL:
				return { &s_operands[1401], 1 };
			case SpirvDecoration::LatencyControlLabelINTEL:
				return { &s_operands[1402], 1 };
			case SpirvDecoration::LatencyControlConstraintINTEL:
				return { &s_operands[1403], 3 };
			case SpirvDecoration::MMHostInterfaceAddressWidthINTEL:
				return { &s_operands[1406], 1 };
			case SpirvDecoration::MMHostInterfaceDataWidthINTEL:
				return { &s_operands[1407], 1 };
			case SpirvDecoration::MMHostInterfaceLatencyINTEL:
				return { &s_operands[1408], 1 };
			case SpirvDecoration::MMHostInterfaceReadWriteModeINTEL:
				return { &s_operands[1409], 1 };
			case SpirvDecoration::MMHostInterfaceMaxBurstINTEL:
				return { &s_operands[1410], 1 };
			case SpirvDecoration::MMHostInterfaceWaitRequestINTEL:
				return { &s_operands[1411], 1 };
			case SpirvDecoration::HostAccessINTEL:
				return { &s_operands[1412], 2 };
			case SpirvDecoration::InitModeINTEL:
				return { &s_operands[1414], 1 };
			case SpirvDecoration::ImplementInRegisterMapINTEL:
				return { &s_operands[1415], 1 };
			case SpirvDecoration::CacheControlLoadINTEL:
				return { &s_operands[1416], 2 };
			case SpirvDecoration::CacheControlStoreINTEL:
				return { &s_operands[1418], 2 };
			default:
				return { nullptr, 0 };
		}
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvDim kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvExecutionMode kind)
	{
		switch(kind)
		{
			case SpirvExecutionMode::Invocations:
				return { &s_operands[1306], 1 };
			case SpirvExecutionMode::LocalSize:
				return { &s_operands[1307], 3 };
			case SpirvExecutionMode::LocalSizeHint:
				return { &s_operands[1307], 3 };
			case SpirvExecutionMode::OutputVertices:
				return { &s_operands[1310], 1 };
			case SpirvExecutionMode::VecTypeHint:
				return { &s_operands[1311], 1 };
			case SpirvExecutionMode::SubgroupSize:
				return { &s_operands[1312], 1 };
			case SpirvExecutionMode::SubgroupsPerWorkgroup:
				return { &s_operands[1313], 1 };
			case SpirvExecutionMode::SubgroupsPerWorkgroupId:
				return { &s_operands[1314], 1 };
			case SpirvExecutionMode::LocalSizeId:
				return { &s_operands[1315], 3 };
			case SpirvExecutionMode::LocalSizeHintId:
				return { &s_operands[1318], 3 };
			case SpirvExecutionMode::DenormPreserve:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::DenormFlushToZero:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::SignedZeroInfNanPreserve:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::RoundingModeRTE:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::RoundingModeRTZ:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::IsApiEntryAMDX:
				return { &s_operands[1322], 1 };
			case SpirvExecutionMode::MaxNodeRecursionAMDX:
				return { &s_operands[1323], 1 };
			case SpirvExecutionMode::StaticNumWorkgroupsAMDX:
				return { &s_operands[1315], 3 };
			case SpirvExecutionMode::ShaderIndexAMDX:
				return { &s_operands[1324], 1 };
			case SpirvExecutionMode::MaxNumWorkgroupsAMDX:
				return { &s_operands[1315], 3 };
			case SpirvExecutionMode::SharesInputWithAMDX:
				return { &s_operands[1325], 2 };
			case SpirvExecutionMode::OutputPrimitivesEXT:
				return { &s_operands[1327], 1 };
			case SpirvExecutionMode::SharedLocalMemorySizeINTEL:
				return { &s_operands[323], 1 };
			case SpirvExecutionMode::RoundingModeRTPINTEL:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::RoundingModeRTNINTEL:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::FloatingPointModeALTINTEL:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::FloatingPointModeIEEEINTEL:
				return { &s_operands[1321], 1 };
			case SpirvExecutionMode::MaxWorkgroupSizeINTEL:
				return { &s_operands[1328], 3 };
			case SpirvExecutionMode::MaxWorkDimINTEL:
				return { &s_operands[1331], 1 };
			case SpirvExecutionMode::NumSIMDWorkitemsINTEL:
				return { &s_operands[1332], 1 };
			case SpirvExecutionMode::SchedulerTargetFmaxMhzINTEL:
				return { &s_operands[1333], 1 };
			case SpirvExecutionMode::FPFastMathDefault:
				return { &s_operands[1334], 2 };
			case SpirvExecutionMode::StreamingInterfaceINTEL:
				return { &s_operands[1336], 1 };
			case SpirvExecutionMode::RegisterMapInterfaceINTEL:
				return { &s_operands[1337], 1 };
			case SpirvExecutionMode::NamedBarrierCountINTEL:
				return { &s_operands[1338], 1 };
			case SpirvExecutionMode::MaximumRegistersINTEL:
				return { &s_operands[1339], 1 };
			case SpirvExecutionMode::MaximumRegistersIdINTEL:
				return { &s_operands[1340], 1 };
			case SpirvExecutionMode::NamedMaximumRegistersINTEL:
				return { &s_operands[1341], 1 };
			default:
				return { nullptr, 0 };
		}
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvExecutionModel kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvFPDenormMode kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvFPEncoding kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvFPOperationMode kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvFPRoundingMode kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvFunctionParameterAttribute kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvGroupOperation kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvHostAccessQualifier kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvImageChannelDataType kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvImageChannelOrder kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvImageFormat kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvInitializationModeQualifier kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvKernelEnqueueFlags kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvLinkageType kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvLoadCacheControl kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvMemoryModel kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvNamedMaximumNumberOfRegisters kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvOverflowModes kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvPackedVectorFormat kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvQuantizationModes kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvRayQueryCandidateIntersectionType kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvRayQueryCommittedIntersectionType kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvRayQueryIntersection kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvSamplerAddressingMode kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvSamplerFilterMode kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvScope kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvSourceLanguage kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvStorageClass kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvStoreCacheControl kind)
	{
		return { nullptr, 0 };
	}

	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands([[maybe_unused]] SpirvTensorClampMode kind)
	{
		return { nullptr, 0 };
	}

	const SpirvInstruction* GetSpirvInstruction(std::uint16_t op)
	{
		auto it = std::lower_bound(std::begin(s_instructions), std::end(s_instructions), op, [](const SpirvInstruction& inst, std::uint16_t op) { return std::uint16_t(inst.op) < op; });
		if (it != std::end(s_instructions) && std::uint16_t(it->op) == op)
			return &*it;
		else
			return nullptr;
	}

	const SpirvGlslStd450Instruction* GetSpirvGlslStd450Instruction(std::uint16_t op)
	{
		auto it = std::lower_bound(std::begin(s_instructionsGlslStd450), std::end(s_instructionsGlslStd450), op, [](const SpirvGlslStd450Instruction& inst, std::uint16_t op) { return std::uint16_t(inst.op) < op; });
		if (it != std::end(s_instructionsGlslStd450) && std::uint16_t(it->op) == op)
			return &*it;
		else
			return nullptr;
	}

	std::string_view ToString(SpirvAccessQualifier value)
	{
		switch (value)
		{
			case SpirvAccessQualifier::ReadOnly: return R"(ReadOnly)";
			case SpirvAccessQualifier::WriteOnly: return R"(WriteOnly)";
			case SpirvAccessQualifier::ReadWrite: return R"(ReadWrite)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvAddressingModel value)
	{
		switch (value)
		{
			case SpirvAddressingModel::Logical: return R"(Logical)";
			case SpirvAddressingModel::Physical32: return R"(Physical32)";
			case SpirvAddressingModel::Physical64: return R"(Physical64)";
			case SpirvAddressingModel::PhysicalStorageBuffer64: return R"(PhysicalStorageBuffer64)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvBuiltIn value)
	{
		switch (value)
		{
			case SpirvBuiltIn::Position: return R"(Position)";
			case SpirvBuiltIn::PointSize: return R"(PointSize)";
			case SpirvBuiltIn::ClipDistance: return R"(ClipDistance)";
			case SpirvBuiltIn::CullDistance: return R"(CullDistance)";
			case SpirvBuiltIn::VertexId: return R"(VertexId)";
			case SpirvBuiltIn::InstanceId: return R"(InstanceId)";
			case SpirvBuiltIn::PrimitiveId: return R"(PrimitiveId)";
			case SpirvBuiltIn::InvocationId: return R"(InvocationId)";
			case SpirvBuiltIn::Layer: return R"(Layer)";
			case SpirvBuiltIn::ViewportIndex: return R"(ViewportIndex)";
			case SpirvBuiltIn::TessLevelOuter: return R"(TessLevelOuter)";
			case SpirvBuiltIn::TessLevelInner: return R"(TessLevelInner)";
			case SpirvBuiltIn::TessCoord: return R"(TessCoord)";
			case SpirvBuiltIn::PatchVertices: return R"(PatchVertices)";
			case SpirvBuiltIn::FragCoord: return R"(FragCoord)";
			case SpirvBuiltIn::PointCoord: return R"(PointCoord)";
			case SpirvBuiltIn::FrontFacing: return R"(FrontFacing)";
			case SpirvBuiltIn::SampleId: return R"(SampleId)";
			case SpirvBuiltIn::SamplePosition: return R"(SamplePosition)";
			case SpirvBuiltIn::SampleMask: return R"(SampleMask)";
			case SpirvBuiltIn::FragDepth: return R"(FragDepth)";
			case SpirvBuiltIn::HelperInvocation: return R"(HelperInvocation)";
			case SpirvBuiltIn::NumWorkgroups: return R"(NumWorkgroups)";
			case SpirvBuiltIn::WorkgroupSize: return R"(WorkgroupSize)";
			case SpirvBuiltIn::WorkgroupId: return R"(WorkgroupId)";
			case SpirvBuiltIn::LocalInvocationId: return R"(LocalInvocationId)";
			case SpirvBuiltIn::GlobalInvocationId: return R"(GlobalInvocationId)";
			case SpirvBuiltIn::LocalInvocationIndex: return R"(LocalInvocationIndex)";
			case SpirvBuiltIn::WorkDim: return R"(WorkDim)";
			case SpirvBuiltIn::GlobalSize: return R"(GlobalSize)";
			case SpirvBuiltIn::EnqueuedWorkgroupSize: return R"(EnqueuedWorkgroupSize)";
			case SpirvBuiltIn::GlobalOffset: return R"(GlobalOffset)";
			case SpirvBuiltIn::GlobalLinearId: return R"(GlobalLinearId)";
			case SpirvBuiltIn::SubgroupSize: return R"(SubgroupSize)";
			case SpirvBuiltIn::SubgroupMaxSize: return R"(SubgroupMaxSize)";
			case SpirvBuiltIn::NumSubgroups: return R"(NumSubgroups)";
			case SpirvBuiltIn::NumEnqueuedSubgroups: return R"(NumEnqueuedSubgroups)";
			case SpirvBuiltIn::SubgroupId: return R"(SubgroupId)";
			case SpirvBuiltIn::SubgroupLocalInvocationId: return R"(SubgroupLocalInvocationId)";
			case SpirvBuiltIn::VertexIndex: return R"(VertexIndex)";
			case SpirvBuiltIn::InstanceIndex: return R"(InstanceIndex)";
			case SpirvBuiltIn::CoreIDARM: return R"(CoreIDARM)";
			case SpirvBuiltIn::CoreCountARM: return R"(CoreCountARM)";
			case SpirvBuiltIn::CoreMaxIDARM: return R"(CoreMaxIDARM)";
			case SpirvBuiltIn::WarpIDARM: return R"(WarpIDARM)";
			case SpirvBuiltIn::WarpMaxIDARM: return R"(WarpMaxIDARM)";
			case SpirvBuiltIn::SubgroupEqMask: return R"(SubgroupEqMask)";
			case SpirvBuiltIn::SubgroupGeMask: return R"(SubgroupGeMask)";
			case SpirvBuiltIn::SubgroupGtMask: return R"(SubgroupGtMask)";
			case SpirvBuiltIn::SubgroupLeMask: return R"(SubgroupLeMask)";
			case SpirvBuiltIn::SubgroupLtMask: return R"(SubgroupLtMask)";
			case SpirvBuiltIn::BaseVertex: return R"(BaseVertex)";
			case SpirvBuiltIn::BaseInstance: return R"(BaseInstance)";
			case SpirvBuiltIn::DrawIndex: return R"(DrawIndex)";
			case SpirvBuiltIn::PrimitiveShadingRateKHR: return R"(PrimitiveShadingRateKHR)";
			case SpirvBuiltIn::DeviceIndex: return R"(DeviceIndex)";
			case SpirvBuiltIn::ViewIndex: return R"(ViewIndex)";
			case SpirvBuiltIn::ShadingRateKHR: return R"(ShadingRateKHR)";
			case SpirvBuiltIn::BaryCoordNoPerspAMD: return R"(BaryCoordNoPerspAMD)";
			case SpirvBuiltIn::BaryCoordNoPerspCentroidAMD: return R"(BaryCoordNoPerspCentroidAMD)";
			case SpirvBuiltIn::BaryCoordNoPerspSampleAMD: return R"(BaryCoordNoPerspSampleAMD)";
			case SpirvBuiltIn::BaryCoordSmoothAMD: return R"(BaryCoordSmoothAMD)";
			case SpirvBuiltIn::BaryCoordSmoothCentroidAMD: return R"(BaryCoordSmoothCentroidAMD)";
			case SpirvBuiltIn::BaryCoordSmoothSampleAMD: return R"(BaryCoordSmoothSampleAMD)";
			case SpirvBuiltIn::BaryCoordPullModelAMD: return R"(BaryCoordPullModelAMD)";
			case SpirvBuiltIn::FragStencilRefEXT: return R"(FragStencilRefEXT)";
			case SpirvBuiltIn::RemainingRecursionLevelsAMDX: return R"(RemainingRecursionLevelsAMDX)";
			case SpirvBuiltIn::ShaderIndexAMDX: return R"(ShaderIndexAMDX)";
			case SpirvBuiltIn::ViewportMaskNV: return R"(ViewportMaskNV)";
			case SpirvBuiltIn::SecondaryPositionNV: return R"(SecondaryPositionNV)";
			case SpirvBuiltIn::SecondaryViewportMaskNV: return R"(SecondaryViewportMaskNV)";
			case SpirvBuiltIn::PositionPerViewNV: return R"(PositionPerViewNV)";
			case SpirvBuiltIn::ViewportMaskPerViewNV: return R"(ViewportMaskPerViewNV)";
			case SpirvBuiltIn::FullyCoveredEXT: return R"(FullyCoveredEXT)";
			case SpirvBuiltIn::TaskCountNV: return R"(TaskCountNV)";
			case SpirvBuiltIn::PrimitiveCountNV: return R"(PrimitiveCountNV)";
			case SpirvBuiltIn::PrimitiveIndicesNV: return R"(PrimitiveIndicesNV)";
			case SpirvBuiltIn::ClipDistancePerViewNV: return R"(ClipDistancePerViewNV)";
			case SpirvBuiltIn::CullDistancePerViewNV: return R"(CullDistancePerViewNV)";
			case SpirvBuiltIn::LayerPerViewNV: return R"(LayerPerViewNV)";
			case SpirvBuiltIn::MeshViewCountNV: return R"(MeshViewCountNV)";
			case SpirvBuiltIn::MeshViewIndicesNV: return R"(MeshViewIndicesNV)";
			case SpirvBuiltIn::BaryCoordKHR: return R"(BaryCoordKHR)";
			case SpirvBuiltIn::BaryCoordNoPerspKHR: return R"(BaryCoordNoPerspKHR)";
			case SpirvBuiltIn::FragSizeEXT: return R"(FragSizeEXT)";
			case SpirvBuiltIn::FragInvocationCountEXT: return R"(FragInvocationCountEXT)";
			case SpirvBuiltIn::PrimitivePointIndicesEXT: return R"(PrimitivePointIndicesEXT)";
			case SpirvBuiltIn::PrimitiveLineIndicesEXT: return R"(PrimitiveLineIndicesEXT)";
			case SpirvBuiltIn::PrimitiveTriangleIndicesEXT: return R"(PrimitiveTriangleIndicesEXT)";
			case SpirvBuiltIn::CullPrimitiveEXT: return R"(CullPrimitiveEXT)";
			case SpirvBuiltIn::LaunchIdKHR: return R"(LaunchIdKHR)";
			case SpirvBuiltIn::LaunchSizeKHR: return R"(LaunchSizeKHR)";
			case SpirvBuiltIn::WorldRayOriginKHR: return R"(WorldRayOriginKHR)";
			case SpirvBuiltIn::WorldRayDirectionKHR: return R"(WorldRayDirectionKHR)";
			case SpirvBuiltIn::ObjectRayOriginKHR: return R"(ObjectRayOriginKHR)";
			case SpirvBuiltIn::ObjectRayDirectionKHR: return R"(ObjectRayDirectionKHR)";
			case SpirvBuiltIn::RayTminKHR: return R"(RayTminKHR)";
			case SpirvBuiltIn::RayTmaxKHR: return R"(RayTmaxKHR)";
			case SpirvBuiltIn::InstanceCustomIndexKHR: return R"(InstanceCustomIndexKHR)";
			case SpirvBuiltIn::ObjectToWorldKHR: return R"(ObjectToWorldKHR)";
			case SpirvBuiltIn::WorldToObjectKHR: return R"(WorldToObjectKHR)";
			case SpirvBuiltIn::HitTNV: return R"(HitTNV)";
			case SpirvBuiltIn::HitKindKHR: return R"(HitKindKHR)";
			case SpirvBuiltIn::CurrentRayTimeNV: return R"(CurrentRayTimeNV)";
			case SpirvBuiltIn::HitTriangleVertexPositionsKHR: return R"(HitTriangleVertexPositionsKHR)";
			case SpirvBuiltIn::HitMicroTriangleVertexPositionsNV: return R"(HitMicroTriangleVertexPositionsNV)";
			case SpirvBuiltIn::HitMicroTriangleVertexBarycentricsNV: return R"(HitMicroTriangleVertexBarycentricsNV)";
			case SpirvBuiltIn::IncomingRayFlagsKHR: return R"(IncomingRayFlagsKHR)";
			case SpirvBuiltIn::RayGeometryIndexKHR: return R"(RayGeometryIndexKHR)";
			case SpirvBuiltIn::WarpsPerSMNV: return R"(WarpsPerSMNV)";
			case SpirvBuiltIn::SMCountNV: return R"(SMCountNV)";
			case SpirvBuiltIn::WarpIDNV: return R"(WarpIDNV)";
			case SpirvBuiltIn::SMIDNV: return R"(SMIDNV)";
			case SpirvBuiltIn::HitKindFrontFacingMicroTriangleNV: return R"(HitKindFrontFacingMicroTriangleNV)";
			case SpirvBuiltIn::HitKindBackFacingMicroTriangleNV: return R"(HitKindBackFacingMicroTriangleNV)";
			case SpirvBuiltIn::CullMaskKHR: return R"(CullMaskKHR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvCapability value)
	{
		switch (value)
		{
			case SpirvCapability::Matrix: return R"(Matrix)";
			case SpirvCapability::Shader: return R"(Shader)";
			case SpirvCapability::Geometry: return R"(Geometry)";
			case SpirvCapability::Tessellation: return R"(Tessellation)";
			case SpirvCapability::Addresses: return R"(Addresses)";
			case SpirvCapability::Linkage: return R"(Linkage)";
			case SpirvCapability::Kernel: return R"(Kernel)";
			case SpirvCapability::Vector16: return R"(Vector16)";
			case SpirvCapability::Float16Buffer: return R"(Float16Buffer)";
			case SpirvCapability::Float16: return R"(Float16)";
			case SpirvCapability::Float64: return R"(Float64)";
			case SpirvCapability::Int64: return R"(Int64)";
			case SpirvCapability::Int64Atomics: return R"(Int64Atomics)";
			case SpirvCapability::ImageBasic: return R"(ImageBasic)";
			case SpirvCapability::ImageReadWrite: return R"(ImageReadWrite)";
			case SpirvCapability::ImageMipmap: return R"(ImageMipmap)";
			case SpirvCapability::Pipes: return R"(Pipes)";
			case SpirvCapability::Groups: return R"(Groups)";
			case SpirvCapability::DeviceEnqueue: return R"(DeviceEnqueue)";
			case SpirvCapability::LiteralSampler: return R"(LiteralSampler)";
			case SpirvCapability::AtomicStorage: return R"(AtomicStorage)";
			case SpirvCapability::Int16: return R"(Int16)";
			case SpirvCapability::TessellationPointSize: return R"(TessellationPointSize)";
			case SpirvCapability::GeometryPointSize: return R"(GeometryPointSize)";
			case SpirvCapability::ImageGatherExtended: return R"(ImageGatherExtended)";
			case SpirvCapability::StorageImageMultisample: return R"(StorageImageMultisample)";
			case SpirvCapability::UniformBufferArrayDynamicIndexing: return R"(UniformBufferArrayDynamicIndexing)";
			case SpirvCapability::SampledImageArrayDynamicIndexing: return R"(SampledImageArrayDynamicIndexing)";
			case SpirvCapability::StorageBufferArrayDynamicIndexing: return R"(StorageBufferArrayDynamicIndexing)";
			case SpirvCapability::StorageImageArrayDynamicIndexing: return R"(StorageImageArrayDynamicIndexing)";
			case SpirvCapability::ClipDistance: return R"(ClipDistance)";
			case SpirvCapability::CullDistance: return R"(CullDistance)";
			case SpirvCapability::ImageCubeArray: return R"(ImageCubeArray)";
			case SpirvCapability::SampleRateShading: return R"(SampleRateShading)";
			case SpirvCapability::ImageRect: return R"(ImageRect)";
			case SpirvCapability::SampledRect: return R"(SampledRect)";
			case SpirvCapability::GenericPointer: return R"(GenericPointer)";
			case SpirvCapability::Int8: return R"(Int8)";
			case SpirvCapability::InputAttachment: return R"(InputAttachment)";
			case SpirvCapability::SparseResidency: return R"(SparseResidency)";
			case SpirvCapability::MinLod: return R"(MinLod)";
			case SpirvCapability::Sampled1D: return R"(Sampled1D)";
			case SpirvCapability::Image1D: return R"(Image1D)";
			case SpirvCapability::SampledCubeArray: return R"(SampledCubeArray)";
			case SpirvCapability::SampledBuffer: return R"(SampledBuffer)";
			case SpirvCapability::ImageBuffer: return R"(ImageBuffer)";
			case SpirvCapability::ImageMSArray: return R"(ImageMSArray)";
			case SpirvCapability::StorageImageExtendedFormats: return R"(StorageImageExtendedFormats)";
			case SpirvCapability::ImageQuery: return R"(ImageQuery)";
			case SpirvCapability::DerivativeControl: return R"(DerivativeControl)";
			case SpirvCapability::InterpolationFunction: return R"(InterpolationFunction)";
			case SpirvCapability::TransformFeedback: return R"(TransformFeedback)";
			case SpirvCapability::GeometryStreams: return R"(GeometryStreams)";
			case SpirvCapability::StorageImageReadWithoutFormat: return R"(StorageImageReadWithoutFormat)";
			case SpirvCapability::StorageImageWriteWithoutFormat: return R"(StorageImageWriteWithoutFormat)";
			case SpirvCapability::MultiViewport: return R"(MultiViewport)";
			case SpirvCapability::SubgroupDispatch: return R"(SubgroupDispatch)";
			case SpirvCapability::NamedBarrier: return R"(NamedBarrier)";
			case SpirvCapability::PipeStorage: return R"(PipeStorage)";
			case SpirvCapability::GroupNonUniform: return R"(GroupNonUniform)";
			case SpirvCapability::GroupNonUniformVote: return R"(GroupNonUniformVote)";
			case SpirvCapability::GroupNonUniformArithmetic: return R"(GroupNonUniformArithmetic)";
			case SpirvCapability::GroupNonUniformBallot: return R"(GroupNonUniformBallot)";
			case SpirvCapability::GroupNonUniformShuffle: return R"(GroupNonUniformShuffle)";
			case SpirvCapability::GroupNonUniformShuffleRelative: return R"(GroupNonUniformShuffleRelative)";
			case SpirvCapability::GroupNonUniformClustered: return R"(GroupNonUniformClustered)";
			case SpirvCapability::GroupNonUniformQuad: return R"(GroupNonUniformQuad)";
			case SpirvCapability::ShaderLayer: return R"(ShaderLayer)";
			case SpirvCapability::ShaderViewportIndex: return R"(ShaderViewportIndex)";
			case SpirvCapability::UniformDecoration: return R"(UniformDecoration)";
			case SpirvCapability::CoreBuiltinsARM: return R"(CoreBuiltinsARM)";
			case SpirvCapability::TileImageColorReadAccessEXT: return R"(TileImageColorReadAccessEXT)";
			case SpirvCapability::TileImageDepthReadAccessEXT: return R"(TileImageDepthReadAccessEXT)";
			case SpirvCapability::TileImageStencilReadAccessEXT: return R"(TileImageStencilReadAccessEXT)";
			case SpirvCapability::CooperativeMatrixLayoutsARM: return R"(CooperativeMatrixLayoutsARM)";
			case SpirvCapability::FragmentShadingRateKHR: return R"(FragmentShadingRateKHR)";
			case SpirvCapability::SubgroupBallotKHR: return R"(SubgroupBallotKHR)";
			case SpirvCapability::DrawParameters: return R"(DrawParameters)";
			case SpirvCapability::WorkgroupMemoryExplicitLayoutKHR: return R"(WorkgroupMemoryExplicitLayoutKHR)";
			case SpirvCapability::WorkgroupMemoryExplicitLayout8BitAccessKHR: return R"(WorkgroupMemoryExplicitLayout8BitAccessKHR)";
			case SpirvCapability::WorkgroupMemoryExplicitLayout16BitAccessKHR: return R"(WorkgroupMemoryExplicitLayout16BitAccessKHR)";
			case SpirvCapability::SubgroupVoteKHR: return R"(SubgroupVoteKHR)";
			case SpirvCapability::StorageBuffer16BitAccess: return R"(StorageBuffer16BitAccess)";
			case SpirvCapability::UniformAndStorageBuffer16BitAccess: return R"(UniformAndStorageBuffer16BitAccess)";
			case SpirvCapability::StoragePushConstant16: return R"(StoragePushConstant16)";
			case SpirvCapability::StorageInputOutput16: return R"(StorageInputOutput16)";
			case SpirvCapability::DeviceGroup: return R"(DeviceGroup)";
			case SpirvCapability::MultiView: return R"(MultiView)";
			case SpirvCapability::VariablePointersStorageBuffer: return R"(VariablePointersStorageBuffer)";
			case SpirvCapability::VariablePointers: return R"(VariablePointers)";
			case SpirvCapability::AtomicStorageOps: return R"(AtomicStorageOps)";
			case SpirvCapability::SampleMaskPostDepthCoverage: return R"(SampleMaskPostDepthCoverage)";
			case SpirvCapability::StorageBuffer8BitAccess: return R"(StorageBuffer8BitAccess)";
			case SpirvCapability::UniformAndStorageBuffer8BitAccess: return R"(UniformAndStorageBuffer8BitAccess)";
			case SpirvCapability::StoragePushConstant8: return R"(StoragePushConstant8)";
			case SpirvCapability::DenormPreserve: return R"(DenormPreserve)";
			case SpirvCapability::DenormFlushToZero: return R"(DenormFlushToZero)";
			case SpirvCapability::SignedZeroInfNanPreserve: return R"(SignedZeroInfNanPreserve)";
			case SpirvCapability::RoundingModeRTE: return R"(RoundingModeRTE)";
			case SpirvCapability::RoundingModeRTZ: return R"(RoundingModeRTZ)";
			case SpirvCapability::RayQueryProvisionalKHR: return R"(RayQueryProvisionalKHR)";
			case SpirvCapability::RayQueryKHR: return R"(RayQueryKHR)";
			case SpirvCapability::UntypedPointersKHR: return R"(UntypedPointersKHR)";
			case SpirvCapability::RayTraversalPrimitiveCullingKHR: return R"(RayTraversalPrimitiveCullingKHR)";
			case SpirvCapability::RayTracingKHR: return R"(RayTracingKHR)";
			case SpirvCapability::TextureSampleWeightedQCOM: return R"(TextureSampleWeightedQCOM)";
			case SpirvCapability::TextureBoxFilterQCOM: return R"(TextureBoxFilterQCOM)";
			case SpirvCapability::TextureBlockMatchQCOM: return R"(TextureBlockMatchQCOM)";
			case SpirvCapability::TextureBlockMatch2QCOM: return R"(TextureBlockMatch2QCOM)";
			case SpirvCapability::Float16ImageAMD: return R"(Float16ImageAMD)";
			case SpirvCapability::ImageGatherBiasLodAMD: return R"(ImageGatherBiasLodAMD)";
			case SpirvCapability::FragmentMaskAMD: return R"(FragmentMaskAMD)";
			case SpirvCapability::StencilExportEXT: return R"(StencilExportEXT)";
			case SpirvCapability::ImageReadWriteLodAMD: return R"(ImageReadWriteLodAMD)";
			case SpirvCapability::Int64ImageEXT: return R"(Int64ImageEXT)";
			case SpirvCapability::ShaderClockKHR: return R"(ShaderClockKHR)";
			case SpirvCapability::ShaderEnqueueAMDX: return R"(ShaderEnqueueAMDX)";
			case SpirvCapability::QuadControlKHR: return R"(QuadControlKHR)";
			case SpirvCapability::SampleMaskOverrideCoverageNV: return R"(SampleMaskOverrideCoverageNV)";
			case SpirvCapability::GeometryShaderPassthroughNV: return R"(GeometryShaderPassthroughNV)";
			case SpirvCapability::ShaderViewportIndexLayerEXT: return R"(ShaderViewportIndexLayerEXT)";
			case SpirvCapability::ShaderViewportMaskNV: return R"(ShaderViewportMaskNV)";
			case SpirvCapability::ShaderStereoViewNV: return R"(ShaderStereoViewNV)";
			case SpirvCapability::PerViewAttributesNV: return R"(PerViewAttributesNV)";
			case SpirvCapability::FragmentFullyCoveredEXT: return R"(FragmentFullyCoveredEXT)";
			case SpirvCapability::MeshShadingNV: return R"(MeshShadingNV)";
			case SpirvCapability::ImageFootprintNV: return R"(ImageFootprintNV)";
			case SpirvCapability::MeshShadingEXT: return R"(MeshShadingEXT)";
			case SpirvCapability::FragmentBarycentricKHR: return R"(FragmentBarycentricKHR)";
			case SpirvCapability::ComputeDerivativeGroupQuadsKHR: return R"(ComputeDerivativeGroupQuadsKHR)";
			case SpirvCapability::FragmentDensityEXT: return R"(FragmentDensityEXT)";
			case SpirvCapability::GroupNonUniformPartitionedNV: return R"(GroupNonUniformPartitionedNV)";
			case SpirvCapability::ShaderNonUniform: return R"(ShaderNonUniform)";
			case SpirvCapability::RuntimeDescriptorArray: return R"(RuntimeDescriptorArray)";
			case SpirvCapability::InputAttachmentArrayDynamicIndexing: return R"(InputAttachmentArrayDynamicIndexing)";
			case SpirvCapability::UniformTexelBufferArrayDynamicIndexing: return R"(UniformTexelBufferArrayDynamicIndexing)";
			case SpirvCapability::StorageTexelBufferArrayDynamicIndexing: return R"(StorageTexelBufferArrayDynamicIndexing)";
			case SpirvCapability::UniformBufferArrayNonUniformIndexing: return R"(UniformBufferArrayNonUniformIndexing)";
			case SpirvCapability::SampledImageArrayNonUniformIndexing: return R"(SampledImageArrayNonUniformIndexing)";
			case SpirvCapability::StorageBufferArrayNonUniformIndexing: return R"(StorageBufferArrayNonUniformIndexing)";
			case SpirvCapability::StorageImageArrayNonUniformIndexing: return R"(StorageImageArrayNonUniformIndexing)";
			case SpirvCapability::InputAttachmentArrayNonUniformIndexing: return R"(InputAttachmentArrayNonUniformIndexing)";
			case SpirvCapability::UniformTexelBufferArrayNonUniformIndexing: return R"(UniformTexelBufferArrayNonUniformIndexing)";
			case SpirvCapability::StorageTexelBufferArrayNonUniformIndexing: return R"(StorageTexelBufferArrayNonUniformIndexing)";
			case SpirvCapability::RayTracingPositionFetchKHR: return R"(RayTracingPositionFetchKHR)";
			case SpirvCapability::RayTracingNV: return R"(RayTracingNV)";
			case SpirvCapability::RayTracingMotionBlurNV: return R"(RayTracingMotionBlurNV)";
			case SpirvCapability::VulkanMemoryModel: return R"(VulkanMemoryModel)";
			case SpirvCapability::VulkanMemoryModelDeviceScope: return R"(VulkanMemoryModelDeviceScope)";
			case SpirvCapability::PhysicalStorageBufferAddresses: return R"(PhysicalStorageBufferAddresses)";
			case SpirvCapability::ComputeDerivativeGroupLinearKHR: return R"(ComputeDerivativeGroupLinearKHR)";
			case SpirvCapability::RayTracingProvisionalKHR: return R"(RayTracingProvisionalKHR)";
			case SpirvCapability::CooperativeMatrixNV: return R"(CooperativeMatrixNV)";
			case SpirvCapability::FragmentShaderSampleInterlockEXT: return R"(FragmentShaderSampleInterlockEXT)";
			case SpirvCapability::FragmentShaderShadingRateInterlockEXT: return R"(FragmentShaderShadingRateInterlockEXT)";
			case SpirvCapability::ShaderSMBuiltinsNV: return R"(ShaderSMBuiltinsNV)";
			case SpirvCapability::FragmentShaderPixelInterlockEXT: return R"(FragmentShaderPixelInterlockEXT)";
			case SpirvCapability::DemoteToHelperInvocation: return R"(DemoteToHelperInvocation)";
			case SpirvCapability::DisplacementMicromapNV: return R"(DisplacementMicromapNV)";
			case SpirvCapability::RayTracingOpacityMicromapEXT: return R"(RayTracingOpacityMicromapEXT)";
			case SpirvCapability::ShaderInvocationReorderNV: return R"(ShaderInvocationReorderNV)";
			case SpirvCapability::BindlessTextureNV: return R"(BindlessTextureNV)";
			case SpirvCapability::RayQueryPositionFetchKHR: return R"(RayQueryPositionFetchKHR)";
			case SpirvCapability::AtomicFloat16VectorNV: return R"(AtomicFloat16VectorNV)";
			case SpirvCapability::RayTracingDisplacementMicromapNV: return R"(RayTracingDisplacementMicromapNV)";
			case SpirvCapability::RawAccessChainsNV: return R"(RawAccessChainsNV)";
			case SpirvCapability::CooperativeMatrixReductionsNV: return R"(CooperativeMatrixReductionsNV)";
			case SpirvCapability::CooperativeMatrixConversionsNV: return R"(CooperativeMatrixConversionsNV)";
			case SpirvCapability::CooperativeMatrixPerElementOperationsNV: return R"(CooperativeMatrixPerElementOperationsNV)";
			case SpirvCapability::CooperativeMatrixTensorAddressingNV: return R"(CooperativeMatrixTensorAddressingNV)";
			case SpirvCapability::CooperativeMatrixBlockLoadsNV: return R"(CooperativeMatrixBlockLoadsNV)";
			case SpirvCapability::TensorAddressingNV: return R"(TensorAddressingNV)";
			case SpirvCapability::SubgroupShuffleINTEL: return R"(SubgroupShuffleINTEL)";
			case SpirvCapability::SubgroupBufferBlockIOINTEL: return R"(SubgroupBufferBlockIOINTEL)";
			case SpirvCapability::SubgroupImageBlockIOINTEL: return R"(SubgroupImageBlockIOINTEL)";
			case SpirvCapability::SubgroupImageMediaBlockIOINTEL: return R"(SubgroupImageMediaBlockIOINTEL)";
			case SpirvCapability::RoundToInfinityINTEL: return R"(RoundToInfinityINTEL)";
			case SpirvCapability::FloatingPointModeINTEL: return R"(FloatingPointModeINTEL)";
			case SpirvCapability::IntegerFunctions2INTEL: return R"(IntegerFunctions2INTEL)";
			case SpirvCapability::FunctionPointersINTEL: return R"(FunctionPointersINTEL)";
			case SpirvCapability::IndirectReferencesINTEL: return R"(IndirectReferencesINTEL)";
			case SpirvCapability::AsmINTEL: return R"(AsmINTEL)";
			case SpirvCapability::AtomicFloat32MinMaxEXT: return R"(AtomicFloat32MinMaxEXT)";
			case SpirvCapability::AtomicFloat64MinMaxEXT: return R"(AtomicFloat64MinMaxEXT)";
			case SpirvCapability::AtomicFloat16MinMaxEXT: return R"(AtomicFloat16MinMaxEXT)";
			case SpirvCapability::VectorComputeINTEL: return R"(VectorComputeINTEL)";
			case SpirvCapability::VectorAnyINTEL: return R"(VectorAnyINTEL)";
			case SpirvCapability::ExpectAssumeKHR: return R"(ExpectAssumeKHR)";
			case SpirvCapability::SubgroupAvcMotionEstimationINTEL: return R"(SubgroupAvcMotionEstimationINTEL)";
			case SpirvCapability::SubgroupAvcMotionEstimationIntraINTEL: return R"(SubgroupAvcMotionEstimationIntraINTEL)";
			case SpirvCapability::SubgroupAvcMotionEstimationChromaINTEL: return R"(SubgroupAvcMotionEstimationChromaINTEL)";
			case SpirvCapability::VariableLengthArrayINTEL: return R"(VariableLengthArrayINTEL)";
			case SpirvCapability::FunctionFloatControlINTEL: return R"(FunctionFloatControlINTEL)";
			case SpirvCapability::FPGAMemoryAttributesINTEL: return R"(FPGAMemoryAttributesINTEL)";
			case SpirvCapability::FPFastMathModeINTEL: return R"(FPFastMathModeINTEL)";
			case SpirvCapability::ArbitraryPrecisionIntegersINTEL: return R"(ArbitraryPrecisionIntegersINTEL)";
			case SpirvCapability::ArbitraryPrecisionFloatingPointINTEL: return R"(ArbitraryPrecisionFloatingPointINTEL)";
			case SpirvCapability::UnstructuredLoopControlsINTEL: return R"(UnstructuredLoopControlsINTEL)";
			case SpirvCapability::FPGALoopControlsINTEL: return R"(FPGALoopControlsINTEL)";
			case SpirvCapability::KernelAttributesINTEL: return R"(KernelAttributesINTEL)";
			case SpirvCapability::FPGAKernelAttributesINTEL: return R"(FPGAKernelAttributesINTEL)";
			case SpirvCapability::FPGAMemoryAccessesINTEL: return R"(FPGAMemoryAccessesINTEL)";
			case SpirvCapability::FPGAClusterAttributesINTEL: return R"(FPGAClusterAttributesINTEL)";
			case SpirvCapability::LoopFuseINTEL: return R"(LoopFuseINTEL)";
			case SpirvCapability::FPGADSPControlINTEL: return R"(FPGADSPControlINTEL)";
			case SpirvCapability::MemoryAccessAliasingINTEL: return R"(MemoryAccessAliasingINTEL)";
			case SpirvCapability::FPGAInvocationPipeliningAttributesINTEL: return R"(FPGAInvocationPipeliningAttributesINTEL)";
			case SpirvCapability::FPGABufferLocationINTEL: return R"(FPGABufferLocationINTEL)";
			case SpirvCapability::ArbitraryPrecisionFixedPointINTEL: return R"(ArbitraryPrecisionFixedPointINTEL)";
			case SpirvCapability::USMStorageClassesINTEL: return R"(USMStorageClassesINTEL)";
			case SpirvCapability::RuntimeAlignedAttributeINTEL: return R"(RuntimeAlignedAttributeINTEL)";
			case SpirvCapability::IOPipesINTEL: return R"(IOPipesINTEL)";
			case SpirvCapability::BlockingPipesINTEL: return R"(BlockingPipesINTEL)";
			case SpirvCapability::FPGARegINTEL: return R"(FPGARegINTEL)";
			case SpirvCapability::DotProductInputAll: return R"(DotProductInputAll)";
			case SpirvCapability::DotProductInput4x8Bit: return R"(DotProductInput4x8Bit)";
			case SpirvCapability::DotProductInput4x8BitPacked: return R"(DotProductInput4x8BitPacked)";
			case SpirvCapability::DotProduct: return R"(DotProduct)";
			case SpirvCapability::RayCullMaskKHR: return R"(RayCullMaskKHR)";
			case SpirvCapability::CooperativeMatrixKHR: return R"(CooperativeMatrixKHR)";
			case SpirvCapability::ReplicatedCompositesEXT: return R"(ReplicatedCompositesEXT)";
			case SpirvCapability::BitInstructions: return R"(BitInstructions)";
			case SpirvCapability::GroupNonUniformRotateKHR: return R"(GroupNonUniformRotateKHR)";
			case SpirvCapability::FloatControls2: return R"(FloatControls2)";
			case SpirvCapability::AtomicFloat32AddEXT: return R"(AtomicFloat32AddEXT)";
			case SpirvCapability::AtomicFloat64AddEXT: return R"(AtomicFloat64AddEXT)";
			case SpirvCapability::LongCompositesINTEL: return R"(LongCompositesINTEL)";
			case SpirvCapability::OptNoneEXT: return R"(OptNoneEXT)";
			case SpirvCapability::AtomicFloat16AddEXT: return R"(AtomicFloat16AddEXT)";
			case SpirvCapability::DebugInfoModuleINTEL: return R"(DebugInfoModuleINTEL)";
			case SpirvCapability::BFloat16ConversionINTEL: return R"(BFloat16ConversionINTEL)";
			case SpirvCapability::SplitBarrierINTEL: return R"(SplitBarrierINTEL)";
			case SpirvCapability::ArithmeticFenceEXT: return R"(ArithmeticFenceEXT)";
			case SpirvCapability::FPGAClusterAttributesV2INTEL: return R"(FPGAClusterAttributesV2INTEL)";
			case SpirvCapability::FPGAKernelAttributesv2INTEL: return R"(FPGAKernelAttributesv2INTEL)";
			case SpirvCapability::FPMaxErrorINTEL: return R"(FPMaxErrorINTEL)";
			case SpirvCapability::FPGALatencyControlINTEL: return R"(FPGALatencyControlINTEL)";
			case SpirvCapability::FPGAArgumentInterfacesINTEL: return R"(FPGAArgumentInterfacesINTEL)";
			case SpirvCapability::GlobalVariableHostAccessINTEL: return R"(GlobalVariableHostAccessINTEL)";
			case SpirvCapability::GlobalVariableFPGADecorationsINTEL: return R"(GlobalVariableFPGADecorationsINTEL)";
			case SpirvCapability::SubgroupBufferPrefetchINTEL: return R"(SubgroupBufferPrefetchINTEL)";
			case SpirvCapability::GroupUniformArithmeticKHR: return R"(GroupUniformArithmeticKHR)";
			case SpirvCapability::MaskedGatherScatterINTEL: return R"(MaskedGatherScatterINTEL)";
			case SpirvCapability::CacheControlsINTEL: return R"(CacheControlsINTEL)";
			case SpirvCapability::RegisterLimitsINTEL: return R"(RegisterLimitsINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvCooperativeMatrixLayout value)
	{
		switch (value)
		{
			case SpirvCooperativeMatrixLayout::RowMajorKHR: return R"(RowMajorKHR)";
			case SpirvCooperativeMatrixLayout::ColumnMajorKHR: return R"(ColumnMajorKHR)";
			case SpirvCooperativeMatrixLayout::RowBlockedInterleavedARM: return R"(RowBlockedInterleavedARM)";
			case SpirvCooperativeMatrixLayout::ColumnBlockedInterleavedARM: return R"(ColumnBlockedInterleavedARM)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvCooperativeMatrixUse value)
	{
		switch (value)
		{
			case SpirvCooperativeMatrixUse::MatrixAKHR: return R"(MatrixAKHR)";
			case SpirvCooperativeMatrixUse::MatrixBKHR: return R"(MatrixBKHR)";
			case SpirvCooperativeMatrixUse::MatrixAccumulatorKHR: return R"(MatrixAccumulatorKHR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvDecoration value)
	{
		switch (value)
		{
			case SpirvDecoration::RelaxedPrecision: return R"(RelaxedPrecision)";
			case SpirvDecoration::SpecId: return R"(SpecId)";
			case SpirvDecoration::Block: return R"(Block)";
			case SpirvDecoration::BufferBlock: return R"(BufferBlock)";
			case SpirvDecoration::RowMajor: return R"(RowMajor)";
			case SpirvDecoration::ColMajor: return R"(ColMajor)";
			case SpirvDecoration::ArrayStride: return R"(ArrayStride)";
			case SpirvDecoration::MatrixStride: return R"(MatrixStride)";
			case SpirvDecoration::GLSLShared: return R"(GLSLShared)";
			case SpirvDecoration::GLSLPacked: return R"(GLSLPacked)";
			case SpirvDecoration::CPacked: return R"(CPacked)";
			case SpirvDecoration::BuiltIn: return R"(BuiltIn)";
			case SpirvDecoration::NoPerspective: return R"(NoPerspective)";
			case SpirvDecoration::Flat: return R"(Flat)";
			case SpirvDecoration::Patch: return R"(Patch)";
			case SpirvDecoration::Centroid: return R"(Centroid)";
			case SpirvDecoration::Sample: return R"(Sample)";
			case SpirvDecoration::Invariant: return R"(Invariant)";
			case SpirvDecoration::Restrict: return R"(Restrict)";
			case SpirvDecoration::Aliased: return R"(Aliased)";
			case SpirvDecoration::Volatile: return R"(Volatile)";
			case SpirvDecoration::Constant: return R"(Constant)";
			case SpirvDecoration::Coherent: return R"(Coherent)";
			case SpirvDecoration::NonWritable: return R"(NonWritable)";
			case SpirvDecoration::NonReadable: return R"(NonReadable)";
			case SpirvDecoration::Uniform: return R"(Uniform)";
			case SpirvDecoration::UniformId: return R"(UniformId)";
			case SpirvDecoration::SaturatedConversion: return R"(SaturatedConversion)";
			case SpirvDecoration::Stream: return R"(Stream)";
			case SpirvDecoration::Location: return R"(Location)";
			case SpirvDecoration::Component: return R"(Component)";
			case SpirvDecoration::Index: return R"(Index)";
			case SpirvDecoration::Binding: return R"(Binding)";
			case SpirvDecoration::DescriptorSet: return R"(DescriptorSet)";
			case SpirvDecoration::Offset: return R"(Offset)";
			case SpirvDecoration::XfbBuffer: return R"(XfbBuffer)";
			case SpirvDecoration::XfbStride: return R"(XfbStride)";
			case SpirvDecoration::FuncParamAttr: return R"(FuncParamAttr)";
			case SpirvDecoration::FPRoundingMode: return R"(FPRoundingMode)";
			case SpirvDecoration::FPFastMathMode: return R"(FPFastMathMode)";
			case SpirvDecoration::LinkageAttributes: return R"(LinkageAttributes)";
			case SpirvDecoration::NoContraction: return R"(NoContraction)";
			case SpirvDecoration::InputAttachmentIndex: return R"(InputAttachmentIndex)";
			case SpirvDecoration::Alignment: return R"(Alignment)";
			case SpirvDecoration::MaxByteOffset: return R"(MaxByteOffset)";
			case SpirvDecoration::AlignmentId: return R"(AlignmentId)";
			case SpirvDecoration::MaxByteOffsetId: return R"(MaxByteOffsetId)";
			case SpirvDecoration::NoSignedWrap: return R"(NoSignedWrap)";
			case SpirvDecoration::NoUnsignedWrap: return R"(NoUnsignedWrap)";
			case SpirvDecoration::WeightTextureQCOM: return R"(WeightTextureQCOM)";
			case SpirvDecoration::BlockMatchTextureQCOM: return R"(BlockMatchTextureQCOM)";
			case SpirvDecoration::BlockMatchSamplerQCOM: return R"(BlockMatchSamplerQCOM)";
			case SpirvDecoration::ExplicitInterpAMD: return R"(ExplicitInterpAMD)";
			case SpirvDecoration::NodeSharesPayloadLimitsWithAMDX: return R"(NodeSharesPayloadLimitsWithAMDX)";
			case SpirvDecoration::NodeMaxPayloadsAMDX: return R"(NodeMaxPayloadsAMDX)";
			case SpirvDecoration::TrackFinishWritingAMDX: return R"(TrackFinishWritingAMDX)";
			case SpirvDecoration::PayloadNodeNameAMDX: return R"(PayloadNodeNameAMDX)";
			case SpirvDecoration::PayloadNodeBaseIndexAMDX: return R"(PayloadNodeBaseIndexAMDX)";
			case SpirvDecoration::PayloadNodeSparseArrayAMDX: return R"(PayloadNodeSparseArrayAMDX)";
			case SpirvDecoration::PayloadNodeArraySizeAMDX: return R"(PayloadNodeArraySizeAMDX)";
			case SpirvDecoration::PayloadDispatchIndirectAMDX: return R"(PayloadDispatchIndirectAMDX)";
			case SpirvDecoration::OverrideCoverageNV: return R"(OverrideCoverageNV)";
			case SpirvDecoration::PassthroughNV: return R"(PassthroughNV)";
			case SpirvDecoration::ViewportRelativeNV: return R"(ViewportRelativeNV)";
			case SpirvDecoration::SecondaryViewportRelativeNV: return R"(SecondaryViewportRelativeNV)";
			case SpirvDecoration::PerPrimitiveEXT: return R"(PerPrimitiveEXT)";
			case SpirvDecoration::PerViewNV: return R"(PerViewNV)";
			case SpirvDecoration::PerTaskNV: return R"(PerTaskNV)";
			case SpirvDecoration::PerVertexKHR: return R"(PerVertexKHR)";
			case SpirvDecoration::NonUniform: return R"(NonUniform)";
			case SpirvDecoration::RestrictPointer: return R"(RestrictPointer)";
			case SpirvDecoration::AliasedPointer: return R"(AliasedPointer)";
			case SpirvDecoration::HitObjectShaderRecordBufferNV: return R"(HitObjectShaderRecordBufferNV)";
			case SpirvDecoration::BindlessSamplerNV: return R"(BindlessSamplerNV)";
			case SpirvDecoration::BindlessImageNV: return R"(BindlessImageNV)";
			case SpirvDecoration::BoundSamplerNV: return R"(BoundSamplerNV)";
			case SpirvDecoration::BoundImageNV: return R"(BoundImageNV)";
			case SpirvDecoration::SIMTCallINTEL: return R"(SIMTCallINTEL)";
			case SpirvDecoration::ReferencedIndirectlyINTEL: return R"(ReferencedIndirectlyINTEL)";
			case SpirvDecoration::ClobberINTEL: return R"(ClobberINTEL)";
			case SpirvDecoration::SideEffectsINTEL: return R"(SideEffectsINTEL)";
			case SpirvDecoration::VectorComputeVariableINTEL: return R"(VectorComputeVariableINTEL)";
			case SpirvDecoration::FuncParamIOKindINTEL: return R"(FuncParamIOKindINTEL)";
			case SpirvDecoration::VectorComputeFunctionINTEL: return R"(VectorComputeFunctionINTEL)";
			case SpirvDecoration::StackCallINTEL: return R"(StackCallINTEL)";
			case SpirvDecoration::GlobalVariableOffsetINTEL: return R"(GlobalVariableOffsetINTEL)";
			case SpirvDecoration::CounterBuffer: return R"(CounterBuffer)";
			case SpirvDecoration::UserSemantic: return R"(UserSemantic)";
			case SpirvDecoration::UserTypeGOOGLE: return R"(UserTypeGOOGLE)";
			case SpirvDecoration::FunctionRoundingModeINTEL: return R"(FunctionRoundingModeINTEL)";
			case SpirvDecoration::FunctionDenormModeINTEL: return R"(FunctionDenormModeINTEL)";
			case SpirvDecoration::RegisterINTEL: return R"(RegisterINTEL)";
			case SpirvDecoration::MemoryINTEL: return R"(MemoryINTEL)";
			case SpirvDecoration::NumbanksINTEL: return R"(NumbanksINTEL)";
			case SpirvDecoration::BankwidthINTEL: return R"(BankwidthINTEL)";
			case SpirvDecoration::MaxPrivateCopiesINTEL: return R"(MaxPrivateCopiesINTEL)";
			case SpirvDecoration::SinglepumpINTEL: return R"(SinglepumpINTEL)";
			case SpirvDecoration::DoublepumpINTEL: return R"(DoublepumpINTEL)";
			case SpirvDecoration::MaxReplicatesINTEL: return R"(MaxReplicatesINTEL)";
			case SpirvDecoration::SimpleDualPortINTEL: return R"(SimpleDualPortINTEL)";
			case SpirvDecoration::MergeINTEL: return R"(MergeINTEL)";
			case SpirvDecoration::BankBitsINTEL: return R"(BankBitsINTEL)";
			case SpirvDecoration::ForcePow2DepthINTEL: return R"(ForcePow2DepthINTEL)";
			case SpirvDecoration::StridesizeINTEL: return R"(StridesizeINTEL)";
			case SpirvDecoration::WordsizeINTEL: return R"(WordsizeINTEL)";
			case SpirvDecoration::TrueDualPortINTEL: return R"(TrueDualPortINTEL)";
			case SpirvDecoration::BurstCoalesceINTEL: return R"(BurstCoalesceINTEL)";
			case SpirvDecoration::CacheSizeINTEL: return R"(CacheSizeINTEL)";
			case SpirvDecoration::DontStaticallyCoalesceINTEL: return R"(DontStaticallyCoalesceINTEL)";
			case SpirvDecoration::PrefetchINTEL: return R"(PrefetchINTEL)";
			case SpirvDecoration::StallEnableINTEL: return R"(StallEnableINTEL)";
			case SpirvDecoration::FuseLoopsInFunctionINTEL: return R"(FuseLoopsInFunctionINTEL)";
			case SpirvDecoration::MathOpDSPModeINTEL: return R"(MathOpDSPModeINTEL)";
			case SpirvDecoration::AliasScopeINTEL: return R"(AliasScopeINTEL)";
			case SpirvDecoration::NoAliasINTEL: return R"(NoAliasINTEL)";
			case SpirvDecoration::InitiationIntervalINTEL: return R"(InitiationIntervalINTEL)";
			case SpirvDecoration::MaxConcurrencyINTEL: return R"(MaxConcurrencyINTEL)";
			case SpirvDecoration::PipelineEnableINTEL: return R"(PipelineEnableINTEL)";
			case SpirvDecoration::BufferLocationINTEL: return R"(BufferLocationINTEL)";
			case SpirvDecoration::IOPipeStorageINTEL: return R"(IOPipeStorageINTEL)";
			case SpirvDecoration::FunctionFloatingPointModeINTEL: return R"(FunctionFloatingPointModeINTEL)";
			case SpirvDecoration::SingleElementVectorINTEL: return R"(SingleElementVectorINTEL)";
			case SpirvDecoration::VectorComputeCallableFunctionINTEL: return R"(VectorComputeCallableFunctionINTEL)";
			case SpirvDecoration::MediaBlockIOINTEL: return R"(MediaBlockIOINTEL)";
			case SpirvDecoration::StallFreeINTEL: return R"(StallFreeINTEL)";
			case SpirvDecoration::FPMaxErrorDecorationINTEL: return R"(FPMaxErrorDecorationINTEL)";
			case SpirvDecoration::LatencyControlLabelINTEL: return R"(LatencyControlLabelINTEL)";
			case SpirvDecoration::LatencyControlConstraintINTEL: return R"(LatencyControlConstraintINTEL)";
			case SpirvDecoration::ConduitKernelArgumentINTEL: return R"(ConduitKernelArgumentINTEL)";
			case SpirvDecoration::RegisterMapKernelArgumentINTEL: return R"(RegisterMapKernelArgumentINTEL)";
			case SpirvDecoration::MMHostInterfaceAddressWidthINTEL: return R"(MMHostInterfaceAddressWidthINTEL)";
			case SpirvDecoration::MMHostInterfaceDataWidthINTEL: return R"(MMHostInterfaceDataWidthINTEL)";
			case SpirvDecoration::MMHostInterfaceLatencyINTEL: return R"(MMHostInterfaceLatencyINTEL)";
			case SpirvDecoration::MMHostInterfaceReadWriteModeINTEL: return R"(MMHostInterfaceReadWriteModeINTEL)";
			case SpirvDecoration::MMHostInterfaceMaxBurstINTEL: return R"(MMHostInterfaceMaxBurstINTEL)";
			case SpirvDecoration::MMHostInterfaceWaitRequestINTEL: return R"(MMHostInterfaceWaitRequestINTEL)";
			case SpirvDecoration::StableKernelArgumentINTEL: return R"(StableKernelArgumentINTEL)";
			case SpirvDecoration::HostAccessINTEL: return R"(HostAccessINTEL)";
			case SpirvDecoration::InitModeINTEL: return R"(InitModeINTEL)";
			case SpirvDecoration::ImplementInRegisterMapINTEL: return R"(ImplementInRegisterMapINTEL)";
			case SpirvDecoration::CacheControlLoadINTEL: return R"(CacheControlLoadINTEL)";
			case SpirvDecoration::CacheControlStoreINTEL: return R"(CacheControlStoreINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvDim value)
	{
		switch (value)
		{
			case SpirvDim::Dim1D: return R"(Dim1D)";
			case SpirvDim::Dim2D: return R"(Dim2D)";
			case SpirvDim::Dim3D: return R"(Dim3D)";
			case SpirvDim::Cube: return R"(Cube)";
			case SpirvDim::Rect: return R"(Rect)";
			case SpirvDim::Buffer: return R"(Buffer)";
			case SpirvDim::SubpassData: return R"(SubpassData)";
			case SpirvDim::TileImageDataEXT: return R"(TileImageDataEXT)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvExecutionMode value)
	{
		switch (value)
		{
			case SpirvExecutionMode::Invocations: return R"(Invocations)";
			case SpirvExecutionMode::SpacingEqual: return R"(SpacingEqual)";
			case SpirvExecutionMode::SpacingFractionalEven: return R"(SpacingFractionalEven)";
			case SpirvExecutionMode::SpacingFractionalOdd: return R"(SpacingFractionalOdd)";
			case SpirvExecutionMode::VertexOrderCw: return R"(VertexOrderCw)";
			case SpirvExecutionMode::VertexOrderCcw: return R"(VertexOrderCcw)";
			case SpirvExecutionMode::PixelCenterInteger: return R"(PixelCenterInteger)";
			case SpirvExecutionMode::OriginUpperLeft: return R"(OriginUpperLeft)";
			case SpirvExecutionMode::OriginLowerLeft: return R"(OriginLowerLeft)";
			case SpirvExecutionMode::EarlyFragmentTests: return R"(EarlyFragmentTests)";
			case SpirvExecutionMode::PointMode: return R"(PointMode)";
			case SpirvExecutionMode::Xfb: return R"(Xfb)";
			case SpirvExecutionMode::DepthReplacing: return R"(DepthReplacing)";
			case SpirvExecutionMode::DepthGreater: return R"(DepthGreater)";
			case SpirvExecutionMode::DepthLess: return R"(DepthLess)";
			case SpirvExecutionMode::DepthUnchanged: return R"(DepthUnchanged)";
			case SpirvExecutionMode::LocalSize: return R"(LocalSize)";
			case SpirvExecutionMode::LocalSizeHint: return R"(LocalSizeHint)";
			case SpirvExecutionMode::InputPoints: return R"(InputPoints)";
			case SpirvExecutionMode::InputLines: return R"(InputLines)";
			case SpirvExecutionMode::InputLinesAdjacency: return R"(InputLinesAdjacency)";
			case SpirvExecutionMode::Triangles: return R"(Triangles)";
			case SpirvExecutionMode::InputTrianglesAdjacency: return R"(InputTrianglesAdjacency)";
			case SpirvExecutionMode::Quads: return R"(Quads)";
			case SpirvExecutionMode::Isolines: return R"(Isolines)";
			case SpirvExecutionMode::OutputVertices: return R"(OutputVertices)";
			case SpirvExecutionMode::OutputPoints: return R"(OutputPoints)";
			case SpirvExecutionMode::OutputLineStrip: return R"(OutputLineStrip)";
			case SpirvExecutionMode::OutputTriangleStrip: return R"(OutputTriangleStrip)";
			case SpirvExecutionMode::VecTypeHint: return R"(VecTypeHint)";
			case SpirvExecutionMode::ContractionOff: return R"(ContractionOff)";
			case SpirvExecutionMode::Initializer: return R"(Initializer)";
			case SpirvExecutionMode::Finalizer: return R"(Finalizer)";
			case SpirvExecutionMode::SubgroupSize: return R"(SubgroupSize)";
			case SpirvExecutionMode::SubgroupsPerWorkgroup: return R"(SubgroupsPerWorkgroup)";
			case SpirvExecutionMode::SubgroupsPerWorkgroupId: return R"(SubgroupsPerWorkgroupId)";
			case SpirvExecutionMode::LocalSizeId: return R"(LocalSizeId)";
			case SpirvExecutionMode::LocalSizeHintId: return R"(LocalSizeHintId)";
			case SpirvExecutionMode::NonCoherentColorAttachmentReadEXT: return R"(NonCoherentColorAttachmentReadEXT)";
			case SpirvExecutionMode::NonCoherentDepthAttachmentReadEXT: return R"(NonCoherentDepthAttachmentReadEXT)";
			case SpirvExecutionMode::NonCoherentStencilAttachmentReadEXT: return R"(NonCoherentStencilAttachmentReadEXT)";
			case SpirvExecutionMode::SubgroupUniformControlFlowKHR: return R"(SubgroupUniformControlFlowKHR)";
			case SpirvExecutionMode::PostDepthCoverage: return R"(PostDepthCoverage)";
			case SpirvExecutionMode::DenormPreserve: return R"(DenormPreserve)";
			case SpirvExecutionMode::DenormFlushToZero: return R"(DenormFlushToZero)";
			case SpirvExecutionMode::SignedZeroInfNanPreserve: return R"(SignedZeroInfNanPreserve)";
			case SpirvExecutionMode::RoundingModeRTE: return R"(RoundingModeRTE)";
			case SpirvExecutionMode::RoundingModeRTZ: return R"(RoundingModeRTZ)";
			case SpirvExecutionMode::EarlyAndLateFragmentTestsAMD: return R"(EarlyAndLateFragmentTestsAMD)";
			case SpirvExecutionMode::StencilRefReplacingEXT: return R"(StencilRefReplacingEXT)";
			case SpirvExecutionMode::CoalescingAMDX: return R"(CoalescingAMDX)";
			case SpirvExecutionMode::IsApiEntryAMDX: return R"(IsApiEntryAMDX)";
			case SpirvExecutionMode::MaxNodeRecursionAMDX: return R"(MaxNodeRecursionAMDX)";
			case SpirvExecutionMode::StaticNumWorkgroupsAMDX: return R"(StaticNumWorkgroupsAMDX)";
			case SpirvExecutionMode::ShaderIndexAMDX: return R"(ShaderIndexAMDX)";
			case SpirvExecutionMode::MaxNumWorkgroupsAMDX: return R"(MaxNumWorkgroupsAMDX)";
			case SpirvExecutionMode::StencilRefUnchangedFrontAMD: return R"(StencilRefUnchangedFrontAMD)";
			case SpirvExecutionMode::StencilRefGreaterFrontAMD: return R"(StencilRefGreaterFrontAMD)";
			case SpirvExecutionMode::StencilRefLessFrontAMD: return R"(StencilRefLessFrontAMD)";
			case SpirvExecutionMode::StencilRefUnchangedBackAMD: return R"(StencilRefUnchangedBackAMD)";
			case SpirvExecutionMode::StencilRefGreaterBackAMD: return R"(StencilRefGreaterBackAMD)";
			case SpirvExecutionMode::StencilRefLessBackAMD: return R"(StencilRefLessBackAMD)";
			case SpirvExecutionMode::QuadDerivativesKHR: return R"(QuadDerivativesKHR)";
			case SpirvExecutionMode::RequireFullQuadsKHR: return R"(RequireFullQuadsKHR)";
			case SpirvExecutionMode::SharesInputWithAMDX: return R"(SharesInputWithAMDX)";
			case SpirvExecutionMode::OutputLinesEXT: return R"(OutputLinesEXT)";
			case SpirvExecutionMode::OutputPrimitivesEXT: return R"(OutputPrimitivesEXT)";
			case SpirvExecutionMode::DerivativeGroupQuadsKHR: return R"(DerivativeGroupQuadsKHR)";
			case SpirvExecutionMode::DerivativeGroupLinearKHR: return R"(DerivativeGroupLinearKHR)";
			case SpirvExecutionMode::OutputTrianglesEXT: return R"(OutputTrianglesEXT)";
			case SpirvExecutionMode::PixelInterlockOrderedEXT: return R"(PixelInterlockOrderedEXT)";
			case SpirvExecutionMode::PixelInterlockUnorderedEXT: return R"(PixelInterlockUnorderedEXT)";
			case SpirvExecutionMode::SampleInterlockOrderedEXT: return R"(SampleInterlockOrderedEXT)";
			case SpirvExecutionMode::SampleInterlockUnorderedEXT: return R"(SampleInterlockUnorderedEXT)";
			case SpirvExecutionMode::ShadingRateInterlockOrderedEXT: return R"(ShadingRateInterlockOrderedEXT)";
			case SpirvExecutionMode::ShadingRateInterlockUnorderedEXT: return R"(ShadingRateInterlockUnorderedEXT)";
			case SpirvExecutionMode::SharedLocalMemorySizeINTEL: return R"(SharedLocalMemorySizeINTEL)";
			case SpirvExecutionMode::RoundingModeRTPINTEL: return R"(RoundingModeRTPINTEL)";
			case SpirvExecutionMode::RoundingModeRTNINTEL: return R"(RoundingModeRTNINTEL)";
			case SpirvExecutionMode::FloatingPointModeALTINTEL: return R"(FloatingPointModeALTINTEL)";
			case SpirvExecutionMode::FloatingPointModeIEEEINTEL: return R"(FloatingPointModeIEEEINTEL)";
			case SpirvExecutionMode::MaxWorkgroupSizeINTEL: return R"(MaxWorkgroupSizeINTEL)";
			case SpirvExecutionMode::MaxWorkDimINTEL: return R"(MaxWorkDimINTEL)";
			case SpirvExecutionMode::NoGlobalOffsetINTEL: return R"(NoGlobalOffsetINTEL)";
			case SpirvExecutionMode::NumSIMDWorkitemsINTEL: return R"(NumSIMDWorkitemsINTEL)";
			case SpirvExecutionMode::SchedulerTargetFmaxMhzINTEL: return R"(SchedulerTargetFmaxMhzINTEL)";
			case SpirvExecutionMode::MaximallyReconvergesKHR: return R"(MaximallyReconvergesKHR)";
			case SpirvExecutionMode::FPFastMathDefault: return R"(FPFastMathDefault)";
			case SpirvExecutionMode::StreamingInterfaceINTEL: return R"(StreamingInterfaceINTEL)";
			case SpirvExecutionMode::RegisterMapInterfaceINTEL: return R"(RegisterMapInterfaceINTEL)";
			case SpirvExecutionMode::NamedBarrierCountINTEL: return R"(NamedBarrierCountINTEL)";
			case SpirvExecutionMode::MaximumRegistersINTEL: return R"(MaximumRegistersINTEL)";
			case SpirvExecutionMode::MaximumRegistersIdINTEL: return R"(MaximumRegistersIdINTEL)";
			case SpirvExecutionMode::NamedMaximumRegistersINTEL: return R"(NamedMaximumRegistersINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvExecutionModel value)
	{
		switch (value)
		{
			case SpirvExecutionModel::Vertex: return R"(Vertex)";
			case SpirvExecutionModel::TessellationControl: return R"(TessellationControl)";
			case SpirvExecutionModel::TessellationEvaluation: return R"(TessellationEvaluation)";
			case SpirvExecutionModel::Geometry: return R"(Geometry)";
			case SpirvExecutionModel::Fragment: return R"(Fragment)";
			case SpirvExecutionModel::GLCompute: return R"(GLCompute)";
			case SpirvExecutionModel::Kernel: return R"(Kernel)";
			case SpirvExecutionModel::TaskNV: return R"(TaskNV)";
			case SpirvExecutionModel::MeshNV: return R"(MeshNV)";
			case SpirvExecutionModel::RayGenerationKHR: return R"(RayGenerationKHR)";
			case SpirvExecutionModel::IntersectionKHR: return R"(IntersectionKHR)";
			case SpirvExecutionModel::AnyHitKHR: return R"(AnyHitKHR)";
			case SpirvExecutionModel::ClosestHitKHR: return R"(ClosestHitKHR)";
			case SpirvExecutionModel::MissKHR: return R"(MissKHR)";
			case SpirvExecutionModel::CallableKHR: return R"(CallableKHR)";
			case SpirvExecutionModel::TaskEXT: return R"(TaskEXT)";
			case SpirvExecutionModel::MeshEXT: return R"(MeshEXT)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvFPDenormMode value)
	{
		switch (value)
		{
			case SpirvFPDenormMode::Preserve: return R"(Preserve)";
			case SpirvFPDenormMode::FlushToZero: return R"(FlushToZero)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvFPEncoding value)
	{
		switch (value)
		{
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvFPOperationMode value)
	{
		switch (value)
		{
			case SpirvFPOperationMode::IEEE: return R"(IEEE)";
			case SpirvFPOperationMode::ALT: return R"(ALT)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvFPRoundingMode value)
	{
		switch (value)
		{
			case SpirvFPRoundingMode::RTE: return R"(RTE)";
			case SpirvFPRoundingMode::RTZ: return R"(RTZ)";
			case SpirvFPRoundingMode::RTP: return R"(RTP)";
			case SpirvFPRoundingMode::RTN: return R"(RTN)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvFunctionParameterAttribute value)
	{
		switch (value)
		{
			case SpirvFunctionParameterAttribute::Zext: return R"(Zext)";
			case SpirvFunctionParameterAttribute::Sext: return R"(Sext)";
			case SpirvFunctionParameterAttribute::ByVal: return R"(ByVal)";
			case SpirvFunctionParameterAttribute::Sret: return R"(Sret)";
			case SpirvFunctionParameterAttribute::NoAlias: return R"(NoAlias)";
			case SpirvFunctionParameterAttribute::NoCapture: return R"(NoCapture)";
			case SpirvFunctionParameterAttribute::NoWrite: return R"(NoWrite)";
			case SpirvFunctionParameterAttribute::NoReadWrite: return R"(NoReadWrite)";
			case SpirvFunctionParameterAttribute::RuntimeAlignedINTEL: return R"(RuntimeAlignedINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvGroupOperation value)
	{
		switch (value)
		{
			case SpirvGroupOperation::Reduce: return R"(Reduce)";
			case SpirvGroupOperation::InclusiveScan: return R"(InclusiveScan)";
			case SpirvGroupOperation::ExclusiveScan: return R"(ExclusiveScan)";
			case SpirvGroupOperation::ClusteredReduce: return R"(ClusteredReduce)";
			case SpirvGroupOperation::PartitionedReduceNV: return R"(PartitionedReduceNV)";
			case SpirvGroupOperation::PartitionedInclusiveScanNV: return R"(PartitionedInclusiveScanNV)";
			case SpirvGroupOperation::PartitionedExclusiveScanNV: return R"(PartitionedExclusiveScanNV)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvHostAccessQualifier value)
	{
		switch (value)
		{
			case SpirvHostAccessQualifier::NoneINTEL: return R"(NoneINTEL)";
			case SpirvHostAccessQualifier::ReadINTEL: return R"(ReadINTEL)";
			case SpirvHostAccessQualifier::WriteINTEL: return R"(WriteINTEL)";
			case SpirvHostAccessQualifier::ReadWriteINTEL: return R"(ReadWriteINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvImageChannelDataType value)
	{
		switch (value)
		{
			case SpirvImageChannelDataType::SnormInt8: return R"(SnormInt8)";
			case SpirvImageChannelDataType::SnormInt16: return R"(SnormInt16)";
			case SpirvImageChannelDataType::UnormInt8: return R"(UnormInt8)";
			case SpirvImageChannelDataType::UnormInt16: return R"(UnormInt16)";
			case SpirvImageChannelDataType::UnormShort565: return R"(UnormShort565)";
			case SpirvImageChannelDataType::UnormShort555: return R"(UnormShort555)";
			case SpirvImageChannelDataType::UnormInt101010: return R"(UnormInt101010)";
			case SpirvImageChannelDataType::SignedInt8: return R"(SignedInt8)";
			case SpirvImageChannelDataType::SignedInt16: return R"(SignedInt16)";
			case SpirvImageChannelDataType::SignedInt32: return R"(SignedInt32)";
			case SpirvImageChannelDataType::UnsignedInt8: return R"(UnsignedInt8)";
			case SpirvImageChannelDataType::UnsignedInt16: return R"(UnsignedInt16)";
			case SpirvImageChannelDataType::UnsignedInt32: return R"(UnsignedInt32)";
			case SpirvImageChannelDataType::HalfFloat: return R"(HalfFloat)";
			case SpirvImageChannelDataType::Float: return R"(Float)";
			case SpirvImageChannelDataType::UnormInt24: return R"(UnormInt24)";
			case SpirvImageChannelDataType::UnormInt101010_2: return R"(UnormInt101010_2)";
			case SpirvImageChannelDataType::UnsignedIntRaw10EXT: return R"(UnsignedIntRaw10EXT)";
			case SpirvImageChannelDataType::UnsignedIntRaw12EXT: return R"(UnsignedIntRaw12EXT)";
			case SpirvImageChannelDataType::UnormInt2_101010EXT: return R"(UnormInt2_101010EXT)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvImageChannelOrder value)
	{
		switch (value)
		{
			case SpirvImageChannelOrder::R: return R"(R)";
			case SpirvImageChannelOrder::A: return R"(A)";
			case SpirvImageChannelOrder::RG: return R"(RG)";
			case SpirvImageChannelOrder::RA: return R"(RA)";
			case SpirvImageChannelOrder::RGB: return R"(RGB)";
			case SpirvImageChannelOrder::RGBA: return R"(RGBA)";
			case SpirvImageChannelOrder::BGRA: return R"(BGRA)";
			case SpirvImageChannelOrder::ARGB: return R"(ARGB)";
			case SpirvImageChannelOrder::Intensity: return R"(Intensity)";
			case SpirvImageChannelOrder::Luminance: return R"(Luminance)";
			case SpirvImageChannelOrder::Rx: return R"(Rx)";
			case SpirvImageChannelOrder::RGx: return R"(RGx)";
			case SpirvImageChannelOrder::RGBx: return R"(RGBx)";
			case SpirvImageChannelOrder::Depth: return R"(Depth)";
			case SpirvImageChannelOrder::DepthStencil: return R"(DepthStencil)";
			case SpirvImageChannelOrder::sRGB: return R"(sRGB)";
			case SpirvImageChannelOrder::sRGBx: return R"(sRGBx)";
			case SpirvImageChannelOrder::sRGBA: return R"(sRGBA)";
			case SpirvImageChannelOrder::sBGRA: return R"(sBGRA)";
			case SpirvImageChannelOrder::ABGR: return R"(ABGR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvImageFormat value)
	{
		switch (value)
		{
			case SpirvImageFormat::Unknown: return R"(Unknown)";
			case SpirvImageFormat::Rgba32f: return R"(Rgba32f)";
			case SpirvImageFormat::Rgba16f: return R"(Rgba16f)";
			case SpirvImageFormat::R32f: return R"(R32f)";
			case SpirvImageFormat::Rgba8: return R"(Rgba8)";
			case SpirvImageFormat::Rgba8Snorm: return R"(Rgba8Snorm)";
			case SpirvImageFormat::Rg32f: return R"(Rg32f)";
			case SpirvImageFormat::Rg16f: return R"(Rg16f)";
			case SpirvImageFormat::R11fG11fB10f: return R"(R11fG11fB10f)";
			case SpirvImageFormat::R16f: return R"(R16f)";
			case SpirvImageFormat::Rgba16: return R"(Rgba16)";
			case SpirvImageFormat::Rgb10A2: return R"(Rgb10A2)";
			case SpirvImageFormat::Rg16: return R"(Rg16)";
			case SpirvImageFormat::Rg8: return R"(Rg8)";
			case SpirvImageFormat::R16: return R"(R16)";
			case SpirvImageFormat::R8: return R"(R8)";
			case SpirvImageFormat::Rgba16Snorm: return R"(Rgba16Snorm)";
			case SpirvImageFormat::Rg16Snorm: return R"(Rg16Snorm)";
			case SpirvImageFormat::Rg8Snorm: return R"(Rg8Snorm)";
			case SpirvImageFormat::R16Snorm: return R"(R16Snorm)";
			case SpirvImageFormat::R8Snorm: return R"(R8Snorm)";
			case SpirvImageFormat::Rgba32i: return R"(Rgba32i)";
			case SpirvImageFormat::Rgba16i: return R"(Rgba16i)";
			case SpirvImageFormat::Rgba8i: return R"(Rgba8i)";
			case SpirvImageFormat::R32i: return R"(R32i)";
			case SpirvImageFormat::Rg32i: return R"(Rg32i)";
			case SpirvImageFormat::Rg16i: return R"(Rg16i)";
			case SpirvImageFormat::Rg8i: return R"(Rg8i)";
			case SpirvImageFormat::R16i: return R"(R16i)";
			case SpirvImageFormat::R8i: return R"(R8i)";
			case SpirvImageFormat::Rgba32ui: return R"(Rgba32ui)";
			case SpirvImageFormat::Rgba16ui: return R"(Rgba16ui)";
			case SpirvImageFormat::Rgba8ui: return R"(Rgba8ui)";
			case SpirvImageFormat::R32ui: return R"(R32ui)";
			case SpirvImageFormat::Rgb10a2ui: return R"(Rgb10a2ui)";
			case SpirvImageFormat::Rg32ui: return R"(Rg32ui)";
			case SpirvImageFormat::Rg16ui: return R"(Rg16ui)";
			case SpirvImageFormat::Rg8ui: return R"(Rg8ui)";
			case SpirvImageFormat::R16ui: return R"(R16ui)";
			case SpirvImageFormat::R8ui: return R"(R8ui)";
			case SpirvImageFormat::R64ui: return R"(R64ui)";
			case SpirvImageFormat::R64i: return R"(R64i)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvInitializationModeQualifier value)
	{
		switch (value)
		{
			case SpirvInitializationModeQualifier::InitOnDeviceReprogramINTEL: return R"(InitOnDeviceReprogramINTEL)";
			case SpirvInitializationModeQualifier::InitOnDeviceResetINTEL: return R"(InitOnDeviceResetINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvKernelEnqueueFlags value)
	{
		switch (value)
		{
			case SpirvKernelEnqueueFlags::NoWait: return R"(NoWait)";
			case SpirvKernelEnqueueFlags::WaitKernel: return R"(WaitKernel)";
			case SpirvKernelEnqueueFlags::WaitWorkGroup: return R"(WaitWorkGroup)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvLinkageType value)
	{
		switch (value)
		{
			case SpirvLinkageType::Export: return R"(Export)";
			case SpirvLinkageType::Import: return R"(Import)";
			case SpirvLinkageType::LinkOnceODR: return R"(LinkOnceODR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvLoadCacheControl value)
	{
		switch (value)
		{
			case SpirvLoadCacheControl::UncachedINTEL: return R"(UncachedINTEL)";
			case SpirvLoadCacheControl::CachedINTEL: return R"(CachedINTEL)";
			case SpirvLoadCacheControl::StreamingINTEL: return R"(StreamingINTEL)";
			case SpirvLoadCacheControl::InvalidateAfterReadINTEL: return R"(InvalidateAfterReadINTEL)";
			case SpirvLoadCacheControl::ConstCachedINTEL: return R"(ConstCachedINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvMemoryModel value)
	{
		switch (value)
		{
			case SpirvMemoryModel::Simple: return R"(Simple)";
			case SpirvMemoryModel::GLSL450: return R"(GLSL450)";
			case SpirvMemoryModel::OpenCL: return R"(OpenCL)";
			case SpirvMemoryModel::Vulkan: return R"(Vulkan)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvNamedMaximumNumberOfRegisters value)
	{
		switch (value)
		{
			case SpirvNamedMaximumNumberOfRegisters::AutoINTEL: return R"(AutoINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvOverflowModes value)
	{
		switch (value)
		{
			case SpirvOverflowModes::WRAP: return R"(WRAP)";
			case SpirvOverflowModes::SAT: return R"(SAT)";
			case SpirvOverflowModes::SAT_ZERO: return R"(SAT_ZERO)";
			case SpirvOverflowModes::SAT_SYM: return R"(SAT_SYM)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvPackedVectorFormat value)
	{
		switch (value)
		{
			case SpirvPackedVectorFormat::PackedVectorFormat4x8Bit: return R"(PackedVectorFormat4x8Bit)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvQuantizationModes value)
	{
		switch (value)
		{
			case SpirvQuantizationModes::TRN: return R"(TRN)";
			case SpirvQuantizationModes::TRN_ZERO: return R"(TRN_ZERO)";
			case SpirvQuantizationModes::RND: return R"(RND)";
			case SpirvQuantizationModes::RND_ZERO: return R"(RND_ZERO)";
			case SpirvQuantizationModes::RND_INF: return R"(RND_INF)";
			case SpirvQuantizationModes::RND_MIN_INF: return R"(RND_MIN_INF)";
			case SpirvQuantizationModes::RND_CONV: return R"(RND_CONV)";
			case SpirvQuantizationModes::RND_CONV_ODD: return R"(RND_CONV_ODD)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvRayQueryCandidateIntersectionType value)
	{
		switch (value)
		{
			case SpirvRayQueryCandidateIntersectionType::RayQueryCandidateIntersectionTriangleKHR: return R"(RayQueryCandidateIntersectionTriangleKHR)";
			case SpirvRayQueryCandidateIntersectionType::RayQueryCandidateIntersectionAABBKHR: return R"(RayQueryCandidateIntersectionAABBKHR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvRayQueryCommittedIntersectionType value)
	{
		switch (value)
		{
			case SpirvRayQueryCommittedIntersectionType::RayQueryCommittedIntersectionNoneKHR: return R"(RayQueryCommittedIntersectionNoneKHR)";
			case SpirvRayQueryCommittedIntersectionType::RayQueryCommittedIntersectionTriangleKHR: return R"(RayQueryCommittedIntersectionTriangleKHR)";
			case SpirvRayQueryCommittedIntersectionType::RayQueryCommittedIntersectionGeneratedKHR: return R"(RayQueryCommittedIntersectionGeneratedKHR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvRayQueryIntersection value)
	{
		switch (value)
		{
			case SpirvRayQueryIntersection::RayQueryCandidateIntersectionKHR: return R"(RayQueryCandidateIntersectionKHR)";
			case SpirvRayQueryIntersection::RayQueryCommittedIntersectionKHR: return R"(RayQueryCommittedIntersectionKHR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvSamplerAddressingMode value)
	{
		switch (value)
		{
			case SpirvSamplerAddressingMode::None: return R"(None)";
			case SpirvSamplerAddressingMode::ClampToEdge: return R"(ClampToEdge)";
			case SpirvSamplerAddressingMode::Clamp: return R"(Clamp)";
			case SpirvSamplerAddressingMode::Repeat: return R"(Repeat)";
			case SpirvSamplerAddressingMode::RepeatMirrored: return R"(RepeatMirrored)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvSamplerFilterMode value)
	{
		switch (value)
		{
			case SpirvSamplerFilterMode::Nearest: return R"(Nearest)";
			case SpirvSamplerFilterMode::Linear: return R"(Linear)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvScope value)
	{
		switch (value)
		{
			case SpirvScope::CrossDevice: return R"(CrossDevice)";
			case SpirvScope::Device: return R"(Device)";
			case SpirvScope::Workgroup: return R"(Workgroup)";
			case SpirvScope::Subgroup: return R"(Subgroup)";
			case SpirvScope::Invocation: return R"(Invocation)";
			case SpirvScope::QueueFamily: return R"(QueueFamily)";
			case SpirvScope::ShaderCallKHR: return R"(ShaderCallKHR)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvSourceLanguage value)
	{
		switch (value)
		{
			case SpirvSourceLanguage::Unknown: return R"(Unknown)";
			case SpirvSourceLanguage::ESSL: return R"(ESSL)";
			case SpirvSourceLanguage::GLSL: return R"(GLSL)";
			case SpirvSourceLanguage::OpenCL_C: return R"(OpenCL_C)";
			case SpirvSourceLanguage::OpenCL_CPP: return R"(OpenCL_CPP)";
			case SpirvSourceLanguage::HLSL: return R"(HLSL)";
			case SpirvSourceLanguage::CPP_for_OpenCL: return R"(CPP_for_OpenCL)";
			case SpirvSourceLanguage::SYCL: return R"(SYCL)";
			case SpirvSourceLanguage::HERO_C: return R"(HERO_C)";
			case SpirvSourceLanguage::NZSL: return R"(NZSL)";
			case SpirvSourceLanguage::WGSL: return R"(WGSL)";
			case SpirvSourceLanguage::Slang: return R"(Slang)";
			case SpirvSourceLanguage::Zig: return R"(Zig)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvStorageClass value)
	{
		switch (value)
		{
			case SpirvStorageClass::UniformConstant: return R"(UniformConstant)";
			case SpirvStorageClass::Input: return R"(Input)";
			case SpirvStorageClass::Uniform: return R"(Uniform)";
			case SpirvStorageClass::Output: return R"(Output)";
			case SpirvStorageClass::Workgroup: return R"(Workgroup)";
			case SpirvStorageClass::CrossWorkgroup: return R"(CrossWorkgroup)";
			case SpirvStorageClass::Private: return R"(Private)";
			case SpirvStorageClass::Function: return R"(Function)";
			case SpirvStorageClass::Generic: return R"(Generic)";
			case SpirvStorageClass::PushConstant: return R"(PushConstant)";
			case SpirvStorageClass::AtomicCounter: return R"(AtomicCounter)";
			case SpirvStorageClass::Image: return R"(Image)";
			case SpirvStorageClass::StorageBuffer: return R"(StorageBuffer)";
			case SpirvStorageClass::TileImageEXT: return R"(TileImageEXT)";
			case SpirvStorageClass::NodePayloadAMDX: return R"(NodePayloadAMDX)";
			case SpirvStorageClass::CallableDataKHR: return R"(CallableDataKHR)";
			case SpirvStorageClass::IncomingCallableDataKHR: return R"(IncomingCallableDataKHR)";
			case SpirvStorageClass::RayPayloadKHR: return R"(RayPayloadKHR)";
			case SpirvStorageClass::HitAttributeKHR: return R"(HitAttributeKHR)";
			case SpirvStorageClass::IncomingRayPayloadKHR: return R"(IncomingRayPayloadKHR)";
			case SpirvStorageClass::ShaderRecordBufferKHR: return R"(ShaderRecordBufferKHR)";
			case SpirvStorageClass::PhysicalStorageBuffer: return R"(PhysicalStorageBuffer)";
			case SpirvStorageClass::HitObjectAttributeNV: return R"(HitObjectAttributeNV)";
			case SpirvStorageClass::TaskPayloadWorkgroupEXT: return R"(TaskPayloadWorkgroupEXT)";
			case SpirvStorageClass::CodeSectionINTEL: return R"(CodeSectionINTEL)";
			case SpirvStorageClass::DeviceOnlyINTEL: return R"(DeviceOnlyINTEL)";
			case SpirvStorageClass::HostOnlyINTEL: return R"(HostOnlyINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvStoreCacheControl value)
	{
		switch (value)
		{
			case SpirvStoreCacheControl::UncachedINTEL: return R"(UncachedINTEL)";
			case SpirvStoreCacheControl::WriteThroughINTEL: return R"(WriteThroughINTEL)";
			case SpirvStoreCacheControl::WriteBackINTEL: return R"(WriteBackINTEL)";
			case SpirvStoreCacheControl::StreamingINTEL: return R"(StreamingINTEL)";
		}

		return "<unhandled value>";
	}

	std::string_view ToString(SpirvTensorClampMode value)
	{
		switch (value)
		{
			case SpirvTensorClampMode::Undefined: return R"(Undefined)";
			case SpirvTensorClampMode::Constant: return R"(Constant)";
			case SpirvTensorClampMode::ClampToEdge: return R"(ClampToEdge)";
			case SpirvTensorClampMode::Repeat: return R"(Repeat)";
			case SpirvTensorClampMode::RepeatMirrored: return R"(RepeatMirrored)";
		}

		return "<unhandled value>";
	}
}
