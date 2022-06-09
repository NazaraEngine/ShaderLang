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
		Simple,             //< a = b
		CompoundAdd,        //< a += b
		CompoundDivide,     //< a /= b
		CompoundMultiply,   //< a *= b
		CompoundLogicalAnd, //< a &&= b
		CompoundLogicalOr,  //< a ||= b
		CompoundSubtract,   //< a -= b
	};

	enum class AttributeType
	{
		Binding,            //< Binding (external var only) - has argument index
		Builtin,            //< Builtin (struct member only) - has argument type
		Cond,               //< Conditional compilation option - has argument expr
		DepthWrite,         //< Depth write mode (function only) - has argument type
		EarlyFragmentTests, //< Entry point (function only) - has argument on/off
		Entry,              //< Entry point (function only) - has argument type
		Export,             //< Exported (external block, function and struct only)
		Layout,             //< Struct layout (struct only) - has argument style
		Location,           //< Location (struct member only) - has argument index
		LangVersion,        //< NZSL version - has argument version string
		Set,                //< Binding set (external var only) - has argument index
		Unroll,             //< Unroll (for/for each only) - has argument mode
	};

	enum class BinaryType
	{
		Add,        //< +
		CompEq,     //< ==
		CompGe,     //< >=
		CompGt,     //< >
		CompLe,     //< <=
		CompLt,     //< <
		CompNe,     //< <=
		Divide,     //< /
		Multiply,   //< *
		LogicalAnd, //< &&
		LogicalOr,  //< ||
		Subtract,   //< -
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
		Greater,
		Less,
		Replace,
		Unchanged,
	};

	enum class ExpressionCategory
	{
		LValue,
		RValue
	};

	enum class IntrinsicType
	{
		CrossProduct = 0,
		DotProduct = 1,
		Exp = 7,
		Length = 3,
		Max = 4,
		Min = 5,
		Normalize = 9,
		Pow = 6,
		Reflect = 8,
		SampleTexture = 2,
	};

	enum class LoopUnroll
	{
		Always,
		Hint,
		Never
	};

	enum class MemoryLayout
	{
		Std140
	};

	enum class NodeType
	{
		None = -1,

#define NZSL_SHADERAST_NODE(Node) Node,
#define NZSL_SHADERAST_STATEMENT_LAST(Node) Node, Max = Node
#include <NZSL/Ast/NodeList.hpp>
	};

	enum class PrimitiveType
	{
		Boolean, //< bool
		Float32, //< f32
		Int32,   //< i32
		UInt32,  //< u32
		String   //< str
	};

	enum class UnaryType
	{
		LogicalNot, //< !v
		Minus,      //< -v
		Plus,       //< +v
	};
}

#endif // NZSL_AST_ENUMS_HPP
