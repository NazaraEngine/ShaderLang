// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LANG_LANGDATA_HPP
#define NZSL_LANG_LANGDATA_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
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
		{ Ast::AttributeType::Unroll,             { "unroll" } }
	});

	struct BuiltinData
	{
		std::string_view identifier;
		ShaderStageTypeFlags compatibleStages;
		std::variant<Ast::PrimitiveType, Ast::VectorType> type; //< Can't use ExpressionType because it's not constexpr
	};

	constexpr auto s_builtinData = frozen::make_unordered_map<Ast::BuiltinEntry, BuiltinData>({
		{ Ast::BuiltinEntry::BaseInstance,   { "base_instance",  ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::BaseVertex,     { "base_vertex",    ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::DrawIndex,      { "draw_index",     ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::FragCoord,      { "frag_coord",     ShaderStageType::Fragment, Ast::VectorType { 4, Ast::PrimitiveType::Float32 } } },
		{ Ast::BuiltinEntry::FragDepth,      { "frag_depth",     ShaderStageType::Fragment, Ast::PrimitiveType::Float32 } },
		{ Ast::BuiltinEntry::InstanceIndex,  { "instance_index", ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::VertexIndex,    { "vertex_index",   ShaderStageType::Vertex,   Ast::PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::VertexPosition, { "position",       ShaderStageType::Vertex,   Ast::VectorType { 4, Ast::PrimitiveType::Float32 } } }
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
		{ ShaderStageType::Fragment, { "frag", "fragment" }},
		{ ShaderStageType::Vertex,   { "vert", "vertex" }},
	});

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
		{ Ast::MemoryLayout::Std140, { "std140", StructLayout::Std140 } }
	});

	constexpr auto s_moduleFeatures = frozen::make_unordered_map<Ast::ModuleFeature, ModuleFeatureData>({
		{ Ast::ModuleFeature::Float64,            { "float64" } },
		{ Ast::ModuleFeature::PrimitiveExternals, { "primitive_externals" } },
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
