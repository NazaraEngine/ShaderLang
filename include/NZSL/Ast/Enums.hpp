// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ENUMS_HPP
#define NZSL_AST_ENUMS_HPP

#include <NZSL/Config.hpp>
#include <Nazara/Utils/Flags.hpp>

namespace nzsl::Ast
{
	enum class AssignType
	{
		Simple             = 0, //< a = b
		CompoundAdd        = 1, //< a += b
		CompoundDivide     = 2, //< a /= b
		CompoundMultiply   = 3, //< a *= b
		CompoundLogicalAnd = 4, //< a &&= b
		CompoundLogicalOr  = 5, //< a ||= b
		CompoundSubtract   = 6, //< a -= b
	};

	enum class AttributeType
	{
		Binding            =  0, //< Binding (external var only) - has argument index
		Builtin            =  1, //< Builtin (struct member only) - has argument type
		Cond               =  2, //< Conditional compilation option - has argument expr
		DepthWrite         =  3, //< Depth write mode (function only) - has argument type
		EarlyFragmentTests =  4, //< Entry point (function only) - has argument on/off
		Entry              =  5, //< Entry point (function only) - has argument type
		Export             =  6, //< Exported (external block, function and struct only)
		Layout             =  7, //< Struct layout (struct only) - has argument style
		Location           =  8, //< Location (struct member only) - has argument index
		LangVersion        =  9, //< NZSL version - has argument version string
		Set                = 10, //< Binding set (external var only) - has argument index
		Unroll             = 11, //< Unroll (for/for each only) - has argument mode
	};

	enum class BinaryType
	{
		Add        = 0,  //< +
		CompEq     = 1,  //< ==
		CompGe     = 2,  //< >=
		CompGt     = 3,  //< >
		CompLe     = 4,  //< <=
		CompLt     = 5,  //< <
		CompNe     = 6,  //< <=
		Divide     = 7,  //< /
		LogicalAnd = 9,  //< &&
		LogicalOr  = 10, //< ||
		Multiply   = 8,  //< *
		Subtract   = 11, //< -
	};

	enum class BuiltinEntry
	{
		BaseInstance   = 3, // gl_BaseInstance (GLSL 450) / BaseInstance (SPIR-V 1.3)
		BaseVertex     = 4, // gl_BaseVertex (GLSL 450) / BaseVertex (SPIR-V 1.3)
		DrawIndex      = 5, // gl_DrawID (GLSL 450) / DrawIndex (SPIR-V 1.3)
		FragCoord      = 1, // gl_FragCoord / FragCoord
		FragDepth      = 2, // gl_FragDepth / FragDepth
		InstanceIndex  = 6, // gl_InstanceIndex (or gl_BaseInstance + gl_InstanceID) / InstanceId
		VertexIndex    = 7, // gl_VertexID/gl_VertexIndex / VertexId
		VertexPosition = 0, // gl_Position / Position
	};

	enum class DepthWriteMode
	{
		Greater   = 0,
		Less      = 1,
		Replace   = 2,
		Unchanged = 3,
	};

	enum class ExpressionCategory
	{
		LValue = 0,
		RValue = 1
	};

	enum class IntrinsicType
	{
		ArraySize     = 10,
		CrossProduct  = 0,
		DotProduct    = 1,
		Exp           = 7,
		Inverse       = 11,
		Length        = 3,
		Max           = 4,
		Min           = 5,
		Normalize     = 9,
		Pow           = 6,
		Reflect       = 8,
		SampleTexture = 2,
		Transpose     = 12
	};

	enum class LoopUnroll
	{
		Always = 0,
		Hint   = 1,
		Never  = 2
	};

	enum class MemoryLayout
	{
		Std140 = 0
	};

	enum class NodeType
	{
		None = -1,

#define NZSL_SHADERAST_NODE(Node, Category) Node##Category,
#define NZSL_SHADERAST_STATEMENT_LAST(Node) Node##Statement, Max = Node##Statement
#include <NZSL/Ast/NodeList.hpp>
	};

	enum class PrimitiveType
	{
		Boolean = 0, //< bool
		Float32 = 1, //< f32
		Int32   = 2, //< i32
		UInt32  = 3, //< u32
		String  = 4  //< str
	};

	enum class UnaryType
	{
		LogicalNot = 0, //< !v
		Minus      = 1, //< -v
		Plus       = 2, //< +v
	};
}

#endif // NZSL_AST_ENUMS_HPP
