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

namespace nzsl::Ast
{
	struct BuiltinData
	{
		std::string_view identifier;
		ShaderStageTypeFlags compatibleStages;
		std::variant<PrimitiveType, VectorType> type; //< Can't use ExpressionType because it's not constexpr
	};

	constexpr auto s_builtinData = frozen::make_unordered_map<BuiltinEntry, BuiltinData>({
		{ Ast::BuiltinEntry::BaseInstance,   { "base_instance",  ShaderStageType::Vertex,   PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::BaseVertex,     { "base_vertex",    ShaderStageType::Vertex,   PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::DrawIndex,      { "draw_index",     ShaderStageType::Vertex,   PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::FragCoord,      { "frag_coord",     ShaderStageType::Fragment, VectorType { 4, PrimitiveType::Float32 } } },
		{ Ast::BuiltinEntry::FragDepth,      { "frag_depth",     ShaderStageType::Fragment, PrimitiveType::Float32 } },
		{ Ast::BuiltinEntry::InstanceIndex,  { "instance_index", ShaderStageType::Vertex,   PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::VertexIndex,    { "vertex_index",   ShaderStageType::Vertex,   PrimitiveType::Int32 } },
		{ Ast::BuiltinEntry::VertexPosition, { "position",       ShaderStageType::Vertex,   VectorType { 4, PrimitiveType::Float32 } } }
	});
}

#endif // NZSL_LANG_LANGDATA_HPP
