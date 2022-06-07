// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_EXPRESSIONTYPE_HPP
#define NZSL_AST_EXPRESSIONTYPE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/SourceLocation.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/ExpressionValue.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace nzsl::Ast
{
	struct ContainedType;

	struct NZSL_API AliasType
	{
		AliasType() = default;
		AliasType(const AliasType& alias);
		AliasType(AliasType&&) noexcept = default;

		AliasType& operator=(const AliasType& alias);
		AliasType& operator=(AliasType&&) noexcept = default;

		std::size_t aliasIndex;
		std::unique_ptr<ContainedType> targetType;

		bool operator==(const AliasType& rhs) const;
		inline bool operator!=(const AliasType& rhs) const;
	};

	struct NZSL_API ArrayType
	{
		ArrayType() = default;
		ArrayType(const ArrayType& array);
		ArrayType(ArrayType&&) noexcept = default;

		ArrayType& operator=(const ArrayType& array);
		ArrayType& operator=(ArrayType&&) noexcept = default;

		std::uint32_t length;
		std::unique_ptr<ContainedType> containedType;

		bool operator==(const ArrayType& rhs) const;
		inline bool operator!=(const ArrayType& rhs) const;
	};

	struct FunctionType
	{
		std::size_t funcIndex;

		inline bool operator==(const FunctionType& rhs) const;
		inline bool operator!=(const FunctionType& rhs) const;
	};

	struct IntrinsicFunctionType
	{
		IntrinsicType intrinsic;

		inline bool operator==(const IntrinsicFunctionType& rhs) const;
		inline bool operator!=(const IntrinsicFunctionType& rhs) const;
	};

	struct MatrixType
	{
		std::size_t columnCount;
		std::size_t rowCount;
		PrimitiveType type;

		inline bool operator==(const MatrixType& rhs) const;
		inline bool operator!=(const MatrixType& rhs) const;
	};

	struct NZSL_API MethodType
	{
		MethodType() = default;
		MethodType(const MethodType& methodType);
		MethodType(MethodType&&) noexcept = default;

		MethodType& operator=(const MethodType& methodType);
		MethodType& operator=(MethodType&&) noexcept = default;

		std::unique_ptr<ContainedType> objectType;
		std::size_t methodIndex;

		bool operator==(const MethodType& rhs) const;
		inline bool operator!=(const MethodType& rhs) const;
	};

	struct NoType
	{
		inline bool operator==(const NoType& rhs) const;
		inline bool operator!=(const NoType& rhs) const;
	};

	struct SamplerType
	{
		ImageType dim;
		PrimitiveType sampledType;

		inline bool operator==(const SamplerType& rhs) const;
		inline bool operator!=(const SamplerType& rhs) const;
	};

	struct StructType
	{
		std::size_t structIndex;

		inline bool operator==(const StructType& rhs) const;
		inline bool operator!=(const StructType& rhs) const;
	};

	struct Type
	{
		std::size_t typeIndex;

		inline bool operator==(const Type& rhs) const;
		inline bool operator!=(const Type& rhs) const;
	};

	struct UniformType
	{
		StructType containedType;

		inline bool operator==(const UniformType& rhs) const;
		inline bool operator!=(const UniformType& rhs) const;
	};

	struct VectorType
	{
		std::size_t componentCount;
		PrimitiveType type;

		inline bool operator==(const VectorType& rhs) const;
		inline bool operator!=(const VectorType& rhs) const;
	};

	using ExpressionType = std::variant<NoType, AliasType, ArrayType, FunctionType, IntrinsicFunctionType, MatrixType, MethodType, PrimitiveType, SamplerType, StructType, Type, UniformType, VectorType>;

	struct ContainedType
	{
		ExpressionType type;
	};

	struct StructDescription
	{
		struct StructMember
		{
			ExpressionValue<BuiltinEntry> builtin;
			ExpressionValue<bool> cond;
			ExpressionValue<std::uint32_t> locationIndex;
			ExpressionValue<ExpressionType> type;
			std::string name;
			SourceLocation sourceLocation;
		};

		ExpressionValue<StructLayout> layout;
		std::string name;
		std::vector<StructMember> members;
		bool isConditional = false;
	};

	inline bool IsAliasType(const ExpressionType& type);
	inline bool IsArrayType(const ExpressionType& type);
	inline bool IsFunctionType(const ExpressionType& type);
	inline bool IsIntrinsicFunctionType(const ExpressionType& type);
	inline bool IsMatrixType(const ExpressionType& type);
	inline bool IsMethodType(const ExpressionType& type);
	inline bool IsNoType(const ExpressionType& type);
	inline bool IsPrimitiveType(const ExpressionType& type);
	inline bool IsSamplerType(const ExpressionType& type);
	inline bool IsStructType(const ExpressionType& type);
	inline bool IsTypeExpression(const ExpressionType& type);
	inline bool IsUniformType(const ExpressionType& type);
	inline bool IsVectorType(const ExpressionType& type);

	struct Stringifier
	{
		std::function<std::string(std::size_t aliasIndex)> aliasStringifier;
		std::function<std::string(std::size_t structIndex)> structStringifier;
		std::function<std::string(std::size_t typeIndex)> typeStringifier;
	};

	std::string ToString(const AliasType& type, const Stringifier& stringifier = {});
	std::string ToString(const ArrayType& type, const Stringifier& stringifier = {});
	std::string ToString(const ExpressionType& type, const Stringifier& stringifier = {});
	std::string ToString(const FunctionType& type, const Stringifier& stringifier = {});
	std::string ToString(const IntrinsicFunctionType& type, const Stringifier& stringifier = {});
	std::string ToString(const MatrixType& type, const Stringifier& stringifier = {});
	std::string ToString(const MethodType& type, const Stringifier& stringifier = {});
	std::string ToString(NoType type, const Stringifier& stringifier = {});
	std::string ToString(PrimitiveType type, const Stringifier& stringifier = {});
	std::string ToString(const SamplerType& type, const Stringifier& stringifier = {});
	std::string ToString(const StructType& type, const Stringifier& stringifier = {});
	std::string ToString(const Type& type, const Stringifier& stringifier = {});
	std::string ToString(const UniformType& type, const Stringifier& stringifier = {});
	std::string ToString(const VectorType& type, const Stringifier& stringifier = {});
}

#include <NZSL/Ast/ExpressionType.inl>

#endif // NZSL_AST_EXPRESSIONTYPE_HPP
