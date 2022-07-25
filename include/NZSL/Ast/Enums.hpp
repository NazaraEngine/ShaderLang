// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ENUMS_HPP
#define NZSL_AST_ENUMS_HPP

#include <Nazara/Utils/Flags.hpp>
#include <NZSL/Config.hpp>

namespace nzsl::Ast
{
	enum class AssignType
	{
		Simple             = 0, //< a = b
		CompoundAdd        = 1, //< a += b
		CompoundDivide     = 2, //< a /= b
		CompoundModulo     = 7, //< a %= b
		CompoundMultiply   = 3, //< a *= b
		CompoundLogicalAnd = 4, //< a &&= b
		CompoundLogicalOr  = 5, //< a ||= b
		CompoundSubtract   = 6, //< a -= b
	};

	enum class AttributeType
	{
		Author             = 12, //< Module author (module statement only) - has argument version string
		Binding            =  0, //< Binding (external var only) - has argument index
		Builtin            =  1, //< Builtin (struct member only) - has argument type
		Cond               =  2, //< Conditional compilation option - has argument expr
		DepthWrite         =  3, //< Depth write mode (function only) - has argument type
		Description        = 13, //< Module description (module statement only) - has argument version string
		EarlyFragmentTests =  4, //< Entry point (function only) - has argument on/off
		Entry              =  5, //< Entry point (function only) - has argument type
		Export             =  6, //< Exported (external block, function and struct only)
		Feature            = 15, //< Feature (module statement) - has argument feature
		LangVersion        =  9, //< NZSL version (module statement) - has argument version string
		License            = 14, //< Module license (module statement) - has argument version string
		Layout             =  7, //< Struct layout (struct only) - has argument style
		Location           =  8, //< Location (struct member only) - has argument index
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
		Modulo     = 12, //< %
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

	enum class ModuleFeature
	{
		PrimitiveExternals = 0
	};

	enum class NodeType
	{
		// Remember to update Max value at the end of the enum when adding an entry
		None = -1,

		// Expressions
		AccessIdentifierExpression   = 0,
		AccessIndexExpression        = 1,
		AliasValueExpression         = 2,
		AssignExpression             = 3,
		BinaryExpression             = 4,
		CallFunctionExpression       = 5,
		CallMethodExpression         = 6,
		CastExpression               = 7,
		ConditionalExpression        = 8,
		ConstantExpression           = 9,
		ConstantArrayValueExpression = 10,
		ConstantValueExpression      = 11,
		FunctionExpression           = 12,
		IdentifierExpression         = 13,
		IntrinsicExpression          = 14,
		IntrinsicFunctionExpression  = 15,
		StructTypeExpression         = 16,
		SwizzleExpression            = 17,
		TypeExpression               = 18,
		VariableValueExpression      = 19,
		UnaryExpression              = 20,

		// Statements
		BranchStatement          = 21,
		ConditionalStatement     = 22,
		DeclareAliasStatement    = 23,
		DeclareConstStatement    = 24,
		DeclareExternalStatement = 25,
		DeclareFunctionStatement = 26,
		DeclareOptionStatement   = 27,
		DeclareStructStatement   = 28,
		DeclareVariableStatement = 29,
		DiscardStatement         = 30,
		ForStatement             = 31,
		ForEachStatement         = 32,
		ExpressionStatement      = 33,
		ImportStatement          = 34,
		MultiStatement           = 35,
		NoOpStatement            = 36,
		ReturnStatement          = 37,
		ScopedStatement          = 38,
		WhileStatement           = 39,

		Max = WhileStatement
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
