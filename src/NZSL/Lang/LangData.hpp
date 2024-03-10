// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
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
		{ Ast::BuiltinEntry::GlocalInvocationIndices,   { "global_invocation_indices", ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } },
		{ Ast::BuiltinEntry::InstanceIndex,             { "instance_index",            ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::LocalInvocationIndex,      { "local_invocation_index",    ShaderStageType::Compute,  Ast::PrimitiveType::UInt32 } },
		{ Ast::BuiltinEntry::LocalInvocationIndices,    { "local_invocation_indices",  ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } },
		{ Ast::BuiltinEntry::VertexIndex,               { "vertex_index",              ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::VertexPosition,            { "position",                  ShaderStageType::Vertex,   Ast::VectorType { 4, Ast::PrimitiveType::Float32 } } },
		{ Ast::BuiltinEntry::WorkgroupCount,            { "workgroup_count",           ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } },
		{ Ast::BuiltinEntry::WorkgroupIndices,          { "workgroup_indices",         ShaderStageType::Compute,  Ast::VectorType { 3, Ast::PrimitiveType::UInt32 } } }
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

	namespace IntrinsicHelper
	{
		enum class ParameterType
		{
			ArrayDyn,           // array or dyn_array
			BValVec,            // Boolean value or vector of booleans
			F32,                // Floating-point value of 32bits
			FVal,               // Floating-point value
			FValVec,            // Floating-point value or vector of floating-point
			FValVec1632,        // Floating-point value or vector of floating-point of 16/32 bits (no f64)
			FVec,               // Floating-point vector
			FVec3,              // Floating-point vector3
			Matrix,             // Matrix (N*M)
			MatrixSquare,       // Square matrix (N*N)
			Numerical,          // Integer/Floating-point/Unsigned integer
			NumericalVec,       // Numerical or vector of numerical
			Sampler,            // sampler
			SampleCoordinates,  // floating-point vector used to sample the texture parameter
			Scalar,             // Boolean/Integer/Floating-point/Unsigned integer
			ScalarVec,          // Scalar or vector of scalar
			SignedNumerical,    // Integer/Floating-point value
			SignedNumericalVec, // signed numerical or vector of signed numerical
			Texture,            // texture
			TextureCoordinates, // integer vector used to sample the texture parameter
			TextureData,        // texture content

			// Constraints
			SameType,
			SameTypeBarrier,
			SameVecComponentCount,
			SameVecComponentCountBarrier,
		};

		enum class ReturnType
		{
			Param0SampledValue,
			Param0TextureValue,
			Param0Transposed,
			Param0Type,
			Param0VecComponent,
			Param1Type,
			U32,
			Void
		};
		
		struct IntrinsicData
		{
			std::string_view functionName; // empty if not a function
			ReturnType returnType;
			const ParameterType* parameterTypes;
			std::size_t parameterCount;
		};

		template<ParameterType... Types>
		struct Params {};

		template<ParameterType... Types>
		struct IntrinsicFuncHelper
		{
			static constexpr std::array parameterArray { Types... };
		};

		template<ParameterType... Types>
		constexpr IntrinsicData Build(std::string_view name, ReturnType retType, Params<Types...>)
		{
			return { name, retType, IntrinsicFuncHelper<Types...>::parameterArray.data(), IntrinsicFuncHelper<Types...>::parameterArray.size()};
		}

		constexpr auto data = frozen::make_unordered_map<Ast::IntrinsicType, IntrinsicData>({
			{ Ast::IntrinsicType::Abs,                               Build("abs",       ReturnType::Param0Type,         Params<ParameterType::SignedNumericalVec>{}) },
			{ Ast::IntrinsicType::ArcCos,                            Build("acos",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcCosh,                           Build("acosh",     ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcSin,                            Build("asin",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcSinh,                           Build("asinh",     ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcTan,                            Build("atan",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArcTan2,                           Build("atan2",     ReturnType::Param0Type,         Params<ParameterType::FValVec1632, ParameterType::FValVec1632, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::ArcTanh,                           Build("atanh",     ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::ArraySize,                         Build("",          ReturnType::U32,                Params<ParameterType::ArrayDyn>{}) },
			{ Ast::IntrinsicType::Ceil,                              Build("ceil",      ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Clamp,                             Build("clamp",     ReturnType::Param0Type,         Params<ParameterType::FValVec, ParameterType::FValVec, ParameterType::FValVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Cos,                               Build("cos",       ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Cosh,                              Build("cosh",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::CrossProduct,                      Build("cross",     ReturnType::Param0Type,         Params<ParameterType::FVec3, ParameterType::FVec3, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::DegToRad,                          Build("deg2rad",   ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Distance,                          Build("distance",  ReturnType::Param0VecComponent, Params<ParameterType::FVec, ParameterType::FVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::DotProduct,                        Build("dot",       ReturnType::Param0VecComponent, Params<ParameterType::FVec, ParameterType::FVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Exp,                               Build("exp",       ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Exp2,                              Build("exp2",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Floor,                             Build("floor",     ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Fract,                             Build("fract",     ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::InverseSqrt,                       Build("rsqrt",     ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Length,                            Build("length",    ReturnType::Param0VecComponent, Params<ParameterType::FVec>{}) },
			{ Ast::IntrinsicType::Lerp,                              Build("lerp",      ReturnType::Param0Type,         Params<ParameterType::FValVec, ParameterType::FValVec, ParameterType::FValVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Log,                               Build("log",       ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Log2,                              Build("log2",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::MatrixInverse,                     Build("inverse",   ReturnType::Param0Type,         Params<ParameterType::MatrixSquare>{}) },
			{ Ast::IntrinsicType::MatrixTranspose,                   Build("transpose", ReturnType::Param0Transposed,   Params<ParameterType::Matrix>{}) },
			{ Ast::IntrinsicType::Max,                               Build("max",       ReturnType::Param0Type,         Params<ParameterType::NumericalVec, ParameterType::NumericalVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Min,                               Build("min",       ReturnType::Param0Type,         Params<ParameterType::NumericalVec, ParameterType::NumericalVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Normalize,                         Build("normalize", ReturnType::Param0Type,         Params<ParameterType::FVec>{}) },
			{ Ast::IntrinsicType::Pow,                               Build("pow",       ReturnType::Param0Type,         Params<ParameterType::FValVec1632, ParameterType::FValVec1632, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::RadToDeg,                          Build("rad2deg",   ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Reflect,                           Build("reflect",   ReturnType::Param0Type,         Params<ParameterType::FVec3, ParameterType::FVec3, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Round,                             Build("round",     ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::RoundEven,                         Build("roundeven", ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Select,                            Build("select",    ReturnType::Param1Type,         Params<ParameterType::BValVec, ParameterType::SameTypeBarrier, ParameterType::ScalarVec, ParameterType::ScalarVec, ParameterType::SameType>{}) },
			{ Ast::IntrinsicType::Sign,                              Build("sign",      ReturnType::Param0Type,         Params<ParameterType::SignedNumericalVec>{}) },
			{ Ast::IntrinsicType::Sin,                               Build("sin",       ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Sinh,                              Build("sinh",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Sqrt,                              Build("sqrt",      ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
			{ Ast::IntrinsicType::Tan,                               Build("tan",       ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::Tanh,                              Build("tanh",      ReturnType::Param0Type,         Params<ParameterType::FValVec1632>{}) },
			{ Ast::IntrinsicType::TextureRead,                       Build("",          ReturnType::Param0TextureValue, Params<ParameterType::Texture, ParameterType::TextureCoordinates>{}) },
			{ Ast::IntrinsicType::TextureSampleImplicitLod,          Build("",          ReturnType::Param0SampledValue, Params<ParameterType::Sampler, ParameterType::SampleCoordinates>{}) },
			{ Ast::IntrinsicType::TextureSampleImplicitLodDepthComp, Build("",          ReturnType::Param0SampledValue, Params<ParameterType::Sampler, ParameterType::SampleCoordinates, ParameterType::F32>{}) },
			{ Ast::IntrinsicType::TextureWrite,                      Build("",          ReturnType::Void,               Params<ParameterType::Texture, ParameterType::TextureCoordinates, ParameterType::TextureData>{}) },
			{ Ast::IntrinsicType::Trunc,                             Build("trunc",     ReturnType::Param0Type,         Params<ParameterType::FValVec>{}) },
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
		{ Ast::MemoryLayout::Std430, { "std430", StructLayout::Std140 } }
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
