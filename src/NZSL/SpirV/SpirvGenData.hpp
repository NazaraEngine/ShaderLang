// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRV_SPIRVGENDATA_HPP
#define NZSL_SPIRV_SPIRVGENDATA_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/Nodes.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/SpirV/SpirvAstVisitor.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <frozen/unordered_map.h>
#include <string_view>
#include <variant>

namespace nzsl
{
	class SpirvAstVisitor;
}

namespace nzsl::SpirvGenData
{
	struct SpirvVersion
	{
		std::uint32_t majorVersion;
		std::uint32_t minorVersion;
	};

	struct SpirvBuiltin
	{
		SpirvBuiltIn decoration;
		SpirvCapability capability;
		SpirvVersion requiredVersion;
	};

	constexpr auto s_builtinMapping = frozen::make_unordered_map<Ast::BuiltinEntry, SpirvBuiltin>({
		{ Ast::BuiltinEntry::BaseInstance,            { SpirvBuiltIn::BaseInstance,         SpirvCapability::DrawParameters, SpirvVersion{ 1, 3 } } },
		{ Ast::BuiltinEntry::BaseVertex,              { SpirvBuiltIn::BaseVertex,           SpirvCapability::DrawParameters, SpirvVersion{ 1, 3 } } },
		{ Ast::BuiltinEntry::DrawIndex,               { SpirvBuiltIn::DrawIndex,            SpirvCapability::DrawParameters, SpirvVersion{ 1, 3 } } },
		{ Ast::BuiltinEntry::FragCoord,               { SpirvBuiltIn::FragCoord,            SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::FragDepth,               { SpirvBuiltIn::FragDepth,            SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::GlocalInvocationIndices, { SpirvBuiltIn::GlobalInvocationId,   SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::InstanceIndex,           { SpirvBuiltIn::InstanceIndex,        SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::LocalInvocationIndex,    { SpirvBuiltIn::LocalInvocationIndex, SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::LocalInvocationIndices,  { SpirvBuiltIn::LocalInvocationId,    SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::VertexIndex,             { SpirvBuiltIn::VertexIndex,          SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::VertexPosition,          { SpirvBuiltIn::Position,             SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::WorkgroupCount,          { SpirvBuiltIn::NumWorkgroups,        SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } },
		{ Ast::BuiltinEntry::WorkgroupIndices,        { SpirvBuiltIn::WorkgroupId,          SpirvCapability::Shader,         SpirvVersion{ 1, 0 } } }
	});

	struct SpirvInterp
	{
		SpirvDecoration interpolationDecoration;
	};

	constexpr auto s_interpolationMapping = frozen::make_unordered_map<Ast::InterpolationQualifier, SpirvInterp>({
		{ Ast::InterpolationQualifier::Flat, { SpirvDecoration::Flat }},
		{ Ast::InterpolationQualifier::NoPerspective, { SpirvDecoration::NoPerspective }}
		// No Ast::InterpolationQualifier::Smooth since it's the default
	});

	using SpirvCodeGenerator = void(SpirvAstVisitor::*)(const Ast::IntrinsicExpression& node);
	using SpirvGlslStd450Selector = SpirvGlslStd450Op(*)(const Ast::IntrinsicExpression& node);

	struct IntrinsicData
	{
		std::variant<SpirvOp, SpirvGlslStd450Op, SpirvGlslStd450Selector, SpirvCodeGenerator> op;
	};

	constexpr auto s_intrinsicData = frozen::make_unordered_map<Ast::IntrinsicType, IntrinsicData>({
		{ Ast::IntrinsicType::Abs,                               { &SpirvAstVisitor::SelectAbs } },
		{ Ast::IntrinsicType::ArcCos,                            { SpirvGlslStd450Op::Acos  } },
		{ Ast::IntrinsicType::ArcCosh,                           { SpirvGlslStd450Op::Acosh } },
		{ Ast::IntrinsicType::ArcSin,                            { SpirvGlslStd450Op::Asinh } },
		{ Ast::IntrinsicType::ArcSinh,                           { SpirvGlslStd450Op::Asinh } },
		{ Ast::IntrinsicType::ArcTan,                            { SpirvGlslStd450Op::Atan  } },
		{ Ast::IntrinsicType::ArcTan2,                           { SpirvGlslStd450Op::Atan2 } },
		{ Ast::IntrinsicType::ArcTanh,                           { SpirvGlslStd450Op::Atanh } },
		{ Ast::IntrinsicType::ArraySize,                         { &SpirvAstVisitor::BuildArraySizeIntrinsic } },
		{ Ast::IntrinsicType::Ceil,                              { SpirvGlslStd450Op::Ceil } },
		{ Ast::IntrinsicType::Clamp,                             { &SpirvAstVisitor::SelectClamp } },
		{ Ast::IntrinsicType::Cos,                               { SpirvGlslStd450Op::Cos } },
		{ Ast::IntrinsicType::Cosh,                              { SpirvGlslStd450Op::Cosh } },
		{ Ast::IntrinsicType::CrossProduct,                      { SpirvGlslStd450Op::Cross } },
		{ Ast::IntrinsicType::DegToRad,                          { SpirvGlslStd450Op::Degrees } },
		{ Ast::IntrinsicType::Distance,                          { SpirvGlslStd450Op::Distance } },
		{ Ast::IntrinsicType::DotProduct,                        { SpirvOp::OpDot } },
		{ Ast::IntrinsicType::Exp,                               { SpirvGlslStd450Op::Exp } },
		{ Ast::IntrinsicType::Exp2,                              { SpirvGlslStd450Op::Exp2 } },
		{ Ast::IntrinsicType::Floor,                             { SpirvGlslStd450Op::Floor } },
		{ Ast::IntrinsicType::Fract,                             { SpirvGlslStd450Op::Fract } },
		{ Ast::IntrinsicType::InverseSqrt,                       { SpirvGlslStd450Op::InverseSqrt } },
		{ Ast::IntrinsicType::Length,                            { SpirvGlslStd450Op::Length } },
		{ Ast::IntrinsicType::Lerp,                              { &SpirvAstVisitor::SelectLerp } },
		{ Ast::IntrinsicType::Log,                               { SpirvGlslStd450Op::Log } },
		{ Ast::IntrinsicType::Log2,                              { SpirvGlslStd450Op::Log2 } },
		{ Ast::IntrinsicType::MatrixInverse,                     { SpirvGlslStd450Op::MatrixInverse } },
		{ Ast::IntrinsicType::MatrixTranspose,                   { SpirvOp::OpTranspose } },
		{ Ast::IntrinsicType::Max,                               { &SpirvAstVisitor::SelectMaxMin } },
		{ Ast::IntrinsicType::Min,                               { &SpirvAstVisitor::SelectMaxMin } },
		{ Ast::IntrinsicType::Normalize,                         { SpirvGlslStd450Op::Normalize } },
		{ Ast::IntrinsicType::Pow,                               { SpirvGlslStd450Op::Pow } },
		{ Ast::IntrinsicType::RadToDeg,                          { SpirvGlslStd450Op::Radians } },
		{ Ast::IntrinsicType::Reflect,                           { SpirvGlslStd450Op::Reflect } },
		{ Ast::IntrinsicType::Round,                             { SpirvGlslStd450Op::Round } },
		{ Ast::IntrinsicType::RoundEven,                         { SpirvGlslStd450Op::RoundEven } },
		{ Ast::IntrinsicType::Select,                            { &SpirvAstVisitor::BuildSelectIntrinsic } },
		{ Ast::IntrinsicType::Sign,                              { &SpirvAstVisitor::SelectSign } },
		{ Ast::IntrinsicType::Sin,                               { SpirvGlslStd450Op::Sin } },
		{ Ast::IntrinsicType::Sinh,                              { SpirvGlslStd450Op::Sinh } },
		{ Ast::IntrinsicType::Sqrt,                              { SpirvGlslStd450Op::Sqrt } },
		{ Ast::IntrinsicType::Tan,                               { SpirvGlslStd450Op::Tan } },
		{ Ast::IntrinsicType::Tanh,                              { SpirvGlslStd450Op::Tanh } },
		{ Ast::IntrinsicType::TextureRead,                       { SpirvOp::OpImageRead } },
		{ Ast::IntrinsicType::TextureSampleImplicitLod,          { SpirvOp::OpImageSampleImplicitLod } },
		{ Ast::IntrinsicType::TextureSampleImplicitLodDepthComp, { SpirvOp::OpImageSampleDrefImplicitLod } },
		{ Ast::IntrinsicType::TextureWrite,                      { SpirvOp::OpImageWrite } },
		{ Ast::IntrinsicType::Trunc,                             { SpirvGlslStd450Op::Trunc } },
	});

	static_assert(LangData::s_intrinsicData.size() == s_intrinsicData.size());
}

#endif // NZSL_SPIRV_SPIRVGENDATA_HPP
