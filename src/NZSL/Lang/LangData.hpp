// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LANG_LANGDATA_HPP
#define NZSL_LANG_LANGDATA_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <string_view>
#include <variant>

namespace nzsl::LangData
{
	struct AccessPolicyData
	{
		std::string_view identifier;
	};

	constexpr auto s_accessPolicies = frozen::make_unordered_map<AccessPolicy, AccessPolicyData>({
		{ AccessPolicy::ReadOnly,  { "readonly" } },
		{ AccessPolicy::ReadWrite, { "readwrite" } },
		{ AccessPolicy::WriteOnly, { "writeonly" } },
	});

	struct AttributeData
	{
		std::string_view identifier;
	};

	constexpr auto s_attributeData = frozen::make_unordered_map<Ast::AttributeType, AttributeData>({
		{ Ast::AttributeType::AutoBinding,        { "auto_binding" } },
		{ Ast::AttributeType::Author,             { "author" } },
		{ Ast::AttributeType::Binding,            { "binding" } },
		{ Ast::AttributeType::Builtin,            { "builtin" } },
		{ Ast::AttributeType::Cond,               { "cond" } },
		{ Ast::AttributeType::DepthWrite,         { "depth_write" } },
		{ Ast::AttributeType::Description,        { "desc" } },
		{ Ast::AttributeType::EarlyFragmentTests, { "early_fragment_tests" } },
		{ Ast::AttributeType::Entry,              { "entry" } },
		{ Ast::AttributeType::Export,             { "export" } },
		{ Ast::AttributeType::Feature,            { "feature" } },
		{ Ast::AttributeType::Interp,             { "interp" } },
		{ Ast::AttributeType::Layout,             { "layout" } },
		{ Ast::AttributeType::License,            { "license" } },
		{ Ast::AttributeType::Location,           { "location" } },
		{ Ast::AttributeType::LangVersion,        { "nzsl_version" } },
		{ Ast::AttributeType::Set,                { "set" } },
		{ Ast::AttributeType::Tag,                { "tag" } },
		{ Ast::AttributeType::Unroll,             { "unroll" } },
		{ Ast::AttributeType::Workgroup,          { "workgroup" } }
	});

	struct BuiltinData
	{
		std::string_view identifier;
		ShaderStageTypeFlags compatibleStages;
		std::variant<Ast::PrimitiveType, Ast::VectorType> type; //< Can't use ExpressionType because it's not constexpr
	};

	constexpr auto s_builtinData = frozen::make_unordered_map<Ast::BuiltinEntry, BuiltinData>({
		{ Ast::BuiltinEntry::BaseInstance,              { "base_instance",             ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::BaseVertex,                { "base_vertex",               ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::DrawIndex,                 { "draw_index",                ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::FragCoord,                 { "frag_coord",                ShaderStageType::Fragment, Ast::VectorType { 4, Ast::PrimitiveType::Float32 } } },
		{ Ast::BuiltinEntry::FragDepth,                 { "frag_depth",                ShaderStageType::Fragment, Ast::PrimitiveType::Float32 } },
		{ Ast::BuiltinEntry::FrontFacing,               { "front_facing",              ShaderStageType::Fragment, Ast::PrimitiveType::Boolean } },
		{ Ast::BuiltinEntry::GlocalInvocationIndices,   { "global_invocation_indices", ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } },
		{ Ast::BuiltinEntry::InstanceIndex,             { "instance_index",            ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::LocalInvocationIndex,      { "local_invocation_index",    ShaderStageType::Compute,  Ast::PrimitiveType::UInt32 } },
		{ Ast::BuiltinEntry::LocalInvocationIndices,    { "local_invocation_indices",  ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } },
		{ Ast::BuiltinEntry::VertexIndex,               { "vertex_index",              ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::VertexPosition,            { "position",                  ShaderStageType::Vertex,   Ast::VectorType { 4, Ast::PrimitiveType::Float32 } } },
		{ Ast::BuiltinEntry::WorkgroupCount,            { "workgroup_count",           ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } },
		{ Ast::BuiltinEntry::WorkgroupIndices,          { "workgroup_indices",         ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } }
	});

	struct ConstantData
	{
		std::size_t constantIndex;
		std::uint32_t value;
	};

	constexpr auto s_constants = frozen::make_unordered_map<frozen::string, ConstantData>({
		{ "readonly", { 0, Nz::SafeCast<std::uint32_t>(AccessPolicy::ReadOnly) } },
		{ "readwrite", { 1, Nz::SafeCast<std::uint32_t>(AccessPolicy::ReadWrite) } },
		{ "writeonly", { 2, Nz::SafeCast<std::uint32_t>(AccessPolicy::WriteOnly) } },

		// TODO: Register integer image formats once integer textures are supported (see s_imageFormats)
		{ "rgba8", { 3, Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA8) }},
		{ "rgba8_snorm", { 4, Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA8Snorm) }},
		{ "rgba16f", { 5, Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA16f) }},
		{ "rgba32f", { 6, Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA32f) }},
		{ "r32f", { 7, Nz::SafeCast<std::uint32_t>(ImageFormat::R32f) }},
		{ "rg32f", { 8, Nz::SafeCast<std::uint32_t>(ImageFormat::RG32f) }},
		{ "rg16f", { 9, Nz::SafeCast<std::uint32_t>(ImageFormat::RG16f) }},
		{ "r16f", { 10, Nz::SafeCast<std::uint32_t>(ImageFormat::R16f) }},
		{ "r11f_g11f_b10f", { 11, Nz::SafeCast<std::uint32_t>(ImageFormat::R11fG11fB10f) }},
		{ "rgba16", { 12, Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA16) }},
		{ "rgb10_a2", { 13, Nz::SafeCast<std::uint32_t>(ImageFormat::RGB10A2) }},
		{ "rg16", { 14, Nz::SafeCast<std::uint32_t>(ImageFormat::RG16) }},
		{ "rg8", { 15, Nz::SafeCast<std::uint32_t>(ImageFormat::RG8) }},
		{ "r16", { 16, Nz::SafeCast<std::uint32_t>(ImageFormat::R16) }},
		{ "r8", { 17, Nz::SafeCast<std::uint32_t>(ImageFormat::R8) }},
		{ "rgba16_snorm", { 18, Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA16Snorm) }},
		{ "rg16_snorm", { 19, Nz::SafeCast<std::uint32_t>(ImageFormat::RG16Snorm) }},
		{ "rg8_snorm", { 20, Nz::SafeCast<std::uint32_t>(ImageFormat::RG8Snorm) }},
		{ "r16_snorm", { 21, Nz::SafeCast<std::uint32_t>(ImageFormat::R16Snorm) }},
		{ "r8_snorm", { 22, Nz::SafeCast<std::uint32_t>(ImageFormat::R8Snorm) }}
	});

	struct DepthWriteModeData
	{
		std::string_view identifier;
	};

	constexpr auto s_depthWriteModes = frozen::make_unordered_map<Ast::DepthWriteMode, DepthWriteModeData>({
		{ Ast::DepthWriteMode::Greater,   { "greater" } },
		{ Ast::DepthWriteMode::Less,      { "less" } },
		{ Ast::DepthWriteMode::Replace,   { "replace" } },
		{ Ast::DepthWriteMode::Unchanged, { "unchanged" } },
	});

	struct EntryPointData
	{
		std::string_view identifier;
		std::string_view name;
	};

	constexpr auto s_entryPoints = frozen::make_unordered_map<ShaderStageType, EntryPointData>({
		{ ShaderStageType::Compute,  { "comp", "compute" }},
		{ ShaderStageType::Fragment, { "frag", "fragment" }},
		{ ShaderStageType::Vertex,   { "vert", "vertex" }},
	});
	
	struct ImageFormatData
	{
		std::string_view identifier;
		bool extended; //< requires SPIR-V StorageImageExtendedFormats capability and is not available in GLSL ES
	};

	constexpr auto s_imageFormats = frozen::make_unordered_map<ImageFormat, ImageFormatData>({
		{ ImageFormat::RGBA8,        { "rgba8",          false } },
		{ ImageFormat::RGBA8Snorm,   { "rgba8_snorm",    false } },
		{ ImageFormat::RGBA16f,      { "rgba16f",        false } },
		{ ImageFormat::RGBA32f,      { "rgba32f",        false } },
		{ ImageFormat::R32f,         { "r32f",           false } },
		{ ImageFormat::RG32f,        { "rg32f",          true } },
		{ ImageFormat::RG16f,        { "rg16f",          true } },
		{ ImageFormat::R16f,         { "r16f",           true } },
		{ ImageFormat::R11fG11fB10f, { "r11f_g11f_b10f", true } },
		{ ImageFormat::RGBA16,       { "rgba16",         true } },
		{ ImageFormat::RGB10A2,      { "rgb10_a2",       true } },
		{ ImageFormat::RG16,         { "rg16",           true } },
		{ ImageFormat::RG8,          { "rg8",            true } },
		{ ImageFormat::R16,          { "r16",            true } },
		{ ImageFormat::R8,           { "r8",             true } },
		{ ImageFormat::RGBA16Snorm,  { "rgba16_snorm",   true } },
		{ ImageFormat::RG16Snorm,    { "rg16_snorm",     true } },
		{ ImageFormat::RG8Snorm,     { "rg8_snorm",      true } },
		{ ImageFormat::R16Snorm,     { "r16_snorm",      true } },
		{ ImageFormat::R8Snorm,      { "r8_snorm",       true } },
	});

	struct InterpolationData
	{
		std::string_view identifier;
	};

	constexpr auto s_interpolations = frozen::make_unordered_map<Ast::InterpolationQualifier, InterpolationData>({
		{ Ast::InterpolationQualifier::Flat,          { "flat" } },
		{ Ast::InterpolationQualifier::NoPerspective, { "no_perspective" } },
		{ Ast::InterpolationQualifier::Smooth,        { "smooth" } }
	});

	namespace IntrinsicHelper
	{
		enum class ParameterType
		{
			ArrayDyn,           // array or dyn_array
			BValVec,            // Boolean value or vector of booleans
			BVec,               // Boolean vector of booleans
			F32,                // Floating-point value of 32bits
			FVal,               // Floating-point value
			FValVec,            // Floating-point value or vector of floating-point
			FValVec1632,        // Floating-point value or vector of floating-point of 16/32 bits (no f64)
			FVec,               // Floating-point vector
			FVec3,              // Floating-point vector3
			FetchableSampler,   // Non-depth sampler whose texels can be fetched (no cubemap)
			I32,                // Integer value of 32bits
			IntegerScalar,      // Integer/Unsigned integer
			Matrix,             // Matrix (N*M)
			MatrixSquare,       // Square matrix (N*N)
			Numerical,          // Integer/Floating-point/Unsigned integer
			NumericalVec,       // Numerical or vector of numerical
			Sampler,            // sampler
			SampleCoordinates,  // Floating-point vector used to sample the texture parameter
			Scalar,             // Boolean/Integer/Floating-point/Unsigned integer
			ScalarVec,          // Scalar or vector of scalar
			SignedNumerical,    // Integer/Floating-point value
			SignedNumericalVec, // Signed numerical or vector of signed numerical
			Texture,            // texture
			TextureCoordinates, // Integer vector used to address a texel of the sampler/texture parameter
			TextureData,        // Texture content

			// Constraints
			SameType,                     // Checks that all types since the last SameTypeBarrier (or first parameter) are the same (note that literal types are taken into account, i.e. FloatLiteral is compatible with f32 and f64)
			SameTypeBarrier,
			SameVecComponentCount,        // Checks that all vectors since the last SameVecComponentCountBarrier (or first parameter) have the same component count
			SameVecComponentCountBarrier,

			ConstraintStart = SameType
		};

		enum class ReturnType
		{
			None,               // ()
			Bool,               // bool
			Param0AsBool,       // Boolean or vector of booleans of the same size of the first parameter
			Param0SampledValue, // Assuming first parameter is a sampler, this represents the return type of a Sample operation on it
			Param0TextureSize,  // Assuming first parameter is a sampler or a texture, this represents the integer type holding its dimensions (array layer count as last component)
			Param0TextureValue, // Assuming first parameter is a texture, this represents the return type of a Read operation on it
			Param0Transposed,   // Assuming first parameter is a matrix, this represents the transposed matrix type
			Param0Type,         // Same type as the first parameter (after same type resolving)
			Param0VecComponent, // Assuming first parameter is a vector, this represents the component type of this vector
			Param1Type,         // Same type as the second parameter (after same type resolving)
			U32                 // u32
		};

		struct IntrinsicData
		{
			std::string_view name;
			bool isMethod;
			ReturnType returnType;
			const ParameterType* parameterTypes;
			std::size_t nonConstraintParameterCount;
			std::size_t parameterCount;
			std::optional<ShaderStageType> requiredStage;
		};

		template<ParameterType... Types>
		struct Params {};

		template<ParameterType... Types>
		struct IntrinsicFuncHelper
		{
			static constexpr std::array parameterArray { Types... };
		};

		template<>
		struct IntrinsicFuncHelper<>
		{
			static constexpr std::array<ParameterType, 0> parameterArray {};
		};

		template<ParameterType... Types>
		constexpr IntrinsicData Build(std::string_view name, bool isMethod, ReturnType retType, Params<Types...>, std::optional<ShaderStageType> requiredStage = std::nullopt)
		{
			constexpr auto& parameterArray = IntrinsicFuncHelper<Types...>::parameterArray;

			std::size_t nonConstraintParameterCount = 0;
			for (ParameterType parameterType : parameterArray)
			{
				if (parameterType < ParameterType::ConstraintStart)
					nonConstraintParameterCount++;
			}

			return { name, isMethod, retType, parameterArray.data(), nonConstraintParameterCount, parameterArray.size(), requiredStage };
		}

		constexpr auto data = frozen::make_unordered_map<Ast::IntrinsicType, IntrinsicData>({
			{ Ast::IntrinsicType::Abs,                               Build("abs",                               false, ReturnType::Param0Type,         Params<ParameterType::SignedNumericalVec>{}) },
			{ Ast::IntrinsicType::All,                               Build("all",                               false, ReturnType::Bool,               Params<ParameterType::BVec>{}) },
			{ Ast::IntrinsicType::Any,                               Build("any",                               false, ReturnType::Bool,               Params<ParameterType::BVec>{}) },
			{ Ast::IntrinsicType::ArcCos,                            Build("acos",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcCosh,                           Build("acosh",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcSin,                            Build("asin",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcSinh,                           Build("asinh",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcTan,                            Build("atan",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcTan2,                           Build("atan2",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632, ParameterType::FValVec1632, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::ArcTanh,                           Build("atanh",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArraySize,                         Build("arraySize",                         true,  ReturnType::U32,                Params<ParameterType::ArrayDyn>{}) },
			{ Ast::IntrinsicType::AtomicAdd,                         Build("atomicAdd",                         false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicAnd,                         Build("atomicAnd",                         false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicCompareExchange,             Build("atomicCompareExchange",             false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicExchange,                    Build("atomicExchange",                    false, ReturnType::Param0Type,         Params<ParameterType::Numerical, ParameterType::Numerical, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicMax,                         Build("atomicMax",                         false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicMin,                         Build("atomicMin",                         false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicOr,                          Build("atomicOr",                          false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicSub,                         Build("atomicSub",                         false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::AtomicXor,                         Build("atomicXor",                         false, ReturnType::Param0Type,         Params<ParameterType::IntegerScalar, ParameterType::IntegerScalar, ParameterType::SameType>{}) },
			//{ Ast::IntrinsicType::ControlAndMemoryBarrierSubgroup,   Build("subgroupBarrierWithMemory",         false, ReturnType::None,               Params<>{}) },
			{ Ast::IntrinsicType::ControlAndMemoryBarrierWorkgroup,  Build("workgroupBarrierWithMemory",        false, ReturnType::None,               Params<>{}) },
			//{ Ast::IntrinsicType::ControlBarrierSubgroup,            Build("subgroupBarrier",                   false, ReturnType::None,               Params<>{}) },
			{ Ast::IntrinsicType::ControlBarrierWorkgroup,           Build("workgroupBarrier",                  false, ReturnType::None,               Params<>{}) },
			{ Ast::IntrinsicType::Ceil,                              Build("ceil",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Clamp,                             Build("clamp",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec, ParameterType::FValVec, ParameterType::FValVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Cos,                               Build("cos",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Cosh,                              Build("cosh",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::CrossProduct,                      Build("cross",                             false, ReturnType::Param0Type,         Params<ParameterType::FVec3, ParameterType::FVec3, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::DegToRad,                          Build("deg2rad",                           false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Ddx,                               Build("ddx",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::DdxCoarse,                         Build("ddxcoarse",                         false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::DdxFine,                           Build("ddxfine",                           false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::Ddy,                               Build("ddy",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::DdyCoarse,                         Build("ddycoarse",                         false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::DdyFine,                           Build("ddyfine",                           false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::Distance,                          Build("distance",                          false, ReturnType::Param0VecComponent, Params<ParameterType::FVec, ParameterType::FVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::DotProduct,                        Build("dot",                               false, ReturnType::Param0VecComponent, Params<ParameterType::FVec, ParameterType::FVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Exp,                               Build("exp",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Exp2,                              Build("exp2",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Floor,                             Build("floor",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Fract,                             Build("fract",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Fwidth,                            Build("fwidth",                            false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::FwidthCoarse,                      Build("fwidthcoarse",                      false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::FwidthFine,                        Build("fwidthfine",                        false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::InverseSqrt,                       Build("rsqrt",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::IsInf,                             Build("isinf",                             false, ReturnType::Param0AsBool,       Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::IsNaN,                             Build("isnan",                             false, ReturnType::Param0AsBool,       Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Length,                            Build("length",                            false, ReturnType::Param0VecComponent, Params<ParameterType::FVec>{}) },
			{ Ast::IntrinsicType::Lerp,                              Build("lerp",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec, ParameterType::FValVec, ParameterType::FValVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Log,                               Build("log",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Log2,                              Build("log2",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::MatrixInverse,                     Build("inverse",                           false, ReturnType::Param0Type,         Params<ParameterType::MatrixSquare>{}) },
			{ Ast::IntrinsicType::MatrixTranspose,                   Build("transpose",                         false, ReturnType::Param0Transposed,   Params<ParameterType::Matrix>{}) },
			{ Ast::IntrinsicType::Max,                               Build("max",                               false, ReturnType::Param0Type,         Params<ParameterType::NumericalVec, ParameterType::NumericalVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::MemoryBarrierDevice,               Build("deviceMemoryBarrier",               false, ReturnType::None,               Params<>{}) },
			{ Ast::IntrinsicType::MemoryBarrierStorage,              Build("storageMemoryBarrier",              false, ReturnType::None,               Params<>{}) },
			//{ Ast::IntrinsicType::MemoryBarrierSubgroup,             Build("subgroupMemoryBarrier",             false, ReturnType::None,               Params<>{}) },
			{ Ast::IntrinsicType::MemoryBarrierTexture,              Build("textureMemoryBarrier",              false, ReturnType::None,               Params<>{}) },
			{ Ast::IntrinsicType::MemoryBarrierWorkgroup,            Build("workgroupMemoryBarrier",            false, ReturnType::None,               Params<>{}) },
			{ Ast::IntrinsicType::Min,                               Build("min",                               false, ReturnType::Param0Type,         Params<ParameterType::NumericalVec, ParameterType::NumericalVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Normalize,                         Build("normalize",                         false, ReturnType::Param0Type,         Params<ParameterType::FVec>{}) },
			{ Ast::IntrinsicType::Not,                               Build("not",                               false, ReturnType::Param0Type,         Params<ParameterType::BVec>{}) },
			{ Ast::IntrinsicType::Pow,                               Build("pow",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632, ParameterType::FValVec1632, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::RadToDeg,                          Build("rad2deg",                           false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Reflect,                           Build("reflect",                           false, ReturnType::Param0Type,         Params<ParameterType::FVec3, ParameterType::FVec3, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Round,                             Build("round",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::RoundEven,                         Build("roundeven",                         false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Select,                            Build("select",                            false, ReturnType::Param1Type,         Params<ParameterType::BValVec, ParameterType::SameTypeBarrier, ParameterType::ScalarVec, ParameterType::ScalarVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Sign,                              Build("sign",                              false, ReturnType::Param0Type,         Params<ParameterType::SignedNumericalVec>{}) },
			{ Ast::IntrinsicType::Sin,                               Build("sin",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Sinh,                              Build("sinh",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::SmoothStep,                        Build("smoothstep",                        false, ReturnType::Param0Type,         Params<ParameterType::FValVec, ParameterType::FValVec, ParameterType::FValVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Step,                              Build("step",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec, ParameterType::FValVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Sqrt,                              Build("sqrt",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Tan,                               Build("tan",                               false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Tanh,                              Build("tanh",                              false, ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::TextureFetch,                      Build("textureFetch",                      true,  ReturnType::Param0SampledValue, Params<ParameterType::FetchableSampler, ParameterType::TextureCoordinates, ParameterType::I32>{}) },
			{ Ast::IntrinsicType::TextureQuerySize,                  Build("textureQuerySize",                  true,  ReturnType::Param0TextureSize,  Params<ParameterType::Texture>{}) },
			{ Ast::IntrinsicType::TextureQuerySizeLod,               Build("textureQuerySizeLod",               true,  ReturnType::Param0TextureSize,  Params<ParameterType::Sampler, ParameterType::I32>{}) },
			{ Ast::IntrinsicType::TextureRead,                       Build("textureRead",                       true,  ReturnType::Param0TextureValue, Params<ParameterType::Texture, ParameterType::TextureCoordinates>{}) },
			{ Ast::IntrinsicType::TextureSampleExplicitLod,          Build("textureSampleExplicitLod",          true,  ReturnType::Param0SampledValue, Params<ParameterType::Sampler, ParameterType::SampleCoordinates, ParameterType::F32>{}) },
			{ Ast::IntrinsicType::TextureSampleImplicitLod,          Build("textureSampleImplicitLod",          true,  ReturnType::Param0SampledValue, Params<ParameterType::Sampler, ParameterType::SampleCoordinates>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::TextureSampleImplicitLodDepthComp, Build("textureSampleImplicitLodDepthComp", true,  ReturnType::Param0SampledValue, Params<ParameterType::Sampler, ParameterType::SampleCoordinates, ParameterType::F32>{}, ShaderStageType::Fragment) },
			{ Ast::IntrinsicType::TextureWrite,                      Build("textureWrite",                      true,  ReturnType::None,               Params<ParameterType::Texture, ParameterType::TextureCoordinates, ParameterType::TextureData>{}) },
			{ Ast::IntrinsicType::Trunc,                             Build("trunc",                             false, ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
		});
	}

	constexpr auto s_intrinsicData = IntrinsicHelper::data;

	struct ModuleFeatureData
	{
		std::string_view identifier;
	};

	struct MemoryLayoutData
	{
		std::string_view identifier;
		StructLayout structLayout;
	};

	constexpr auto s_memoryLayouts = frozen::make_unordered_map<Ast::MemoryLayout, MemoryLayoutData>({
		{ Ast::MemoryLayout::Std140, { "std140", StructLayout::Std140 } },
		{ Ast::MemoryLayout::Std430, { "std430", StructLayout::Std430 } },
		{ Ast::MemoryLayout::Scalar, { "scalar", StructLayout::Scalar } },
	});

	constexpr auto s_moduleFeatures = frozen::make_unordered_map<Ast::ModuleFeature, ModuleFeatureData>({
		{ Ast::ModuleFeature::Float64,            { "float64" } },
		{ Ast::ModuleFeature::PrimitiveExternals, { "primitive_externals" } },
		{ Ast::ModuleFeature::Texture1D,          { "texture1D" } },
	});

	struct LoopUnrollData
	{
		std::string_view identifier;
	};

	constexpr auto s_unrollModes = frozen::make_unordered_map<Ast::LoopUnroll, LoopUnrollData>({
		{ Ast::LoopUnroll::Always, { "always" } },
		{ Ast::LoopUnroll::Hint,   { "hint" } },
		{ Ast::LoopUnroll::Never,  { "never" } }
	});
}

#endif // NZSL_LANG_LANGDATA_HPP
