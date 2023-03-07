// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRV_SPIRVCONSTANTCACHE_HPP
#define NZSL_SPIRV_SPIRVCONSTANTCACHE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace nzsl
{
	class FieldOffsets;
	class SpirvSection;

	class NZSL_API SpirvConstantCache
	{
		public:
			using StructCallback = std::function<const Ast::StructDescription&(std::size_t structIndex)>;

			SpirvConstantCache(std::uint32_t& resultId);
			SpirvConstantCache(const SpirvConstantCache& cache) = delete;
			SpirvConstantCache(SpirvConstantCache&& cache) noexcept;
			~SpirvConstantCache();

			struct Constant;
			struct Identifier;
			struct Type;

			using ConstantPtr = std::shared_ptr<Constant>;
			using TypePtr = std::shared_ptr<Type>;

			struct Array
			{
				TypePtr elementType;
				ConstantPtr length; //< nullptr = RuntimeArray
				std::optional<std::uint32_t> stride;
			};

			struct Bool {};

			struct Float
			{
				std::uint32_t width;
			};

			struct Integer
			{
				std::uint32_t width;
				bool signedness;
			};

			struct Void {};

			struct Vector
			{
				TypePtr componentType;
				std::uint32_t componentCount;
			};

			struct Matrix
			{
				TypePtr columnType;
				std::uint32_t columnCount;
			};

			struct Image
			{
				std::optional<SpirvAccessQualifier> qualifier;
				std::optional<bool> depth;
				std::optional<bool> sampled;
				SpirvDim dim;
				SpirvImageFormat format = SpirvImageFormat::Unknown;
				TypePtr sampledType;
				bool arrayed = false;
				bool multisampled = false;
			};

			struct Pointer
			{
				TypePtr type;
				SpirvStorageClass storageClass;
			};

			struct Function
			{
				TypePtr returnType;
				std::vector<TypePtr> parameters;
			};

			struct SampledImage
			{
				TypePtr image;
			};

			struct Structure
			{
				struct Member
				{
					std::string name;
					TypePtr type;
					mutable std::optional<std::uint32_t> offset;
				};

				std::string name;
				std::vector<Member> members;
				std::vector<SpirvDecoration> decorations;
			};

			using AnyType = std::variant<Array, Bool, Float, Function, Image, Integer, Matrix, Pointer, SampledImage, Structure, Vector, Void>;

			struct ConstantBool
			{
				bool value;
			};

			struct ConstantComposite
			{
				TypePtr type;
				std::vector<ConstantPtr> values;
			};

			struct ConstantScalar
			{
				std::variant<float, double, std::int32_t, std::int64_t, std::uint32_t, std::uint64_t> value;
			};

			using AnyConstant = std::variant<ConstantBool, ConstantComposite, ConstantScalar>;

			struct Variable
			{
				std::optional<std::size_t> funcId; //< For inputs/outputs
				std::optional<ConstantPtr> initializer;
				std::string debugName;
				TypePtr type;
				SpirvStorageClass storageClass;
			};

			using BaseType = std::variant<Bool, Float, Integer, Vector, Matrix, Image>;
			using CompositeValue = std::variant<ConstantBool, ConstantScalar, ConstantComposite>;
			using PointerOrBaseType = std::variant<BaseType, Pointer>;
			using PrimitiveType = std::variant<Bool, Float, Integer>;
			using ScalarType = std::variant<Float, Integer>;

			struct Constant
			{
				Constant(AnyConstant c) :
				constant(std::move(c))
				{
				}

				AnyConstant constant;
			};

			struct Type
			{
				Type(AnyType c) :
				type(std::move(c))
				{
				}

				AnyType type;
			};

			ConstantPtr BuildArrayConstant(const Ast::ConstantArrayValue& value) const;
			ConstantPtr BuildConstant(const Ast::ConstantSingleValue& value) const;
			FieldOffsets BuildFieldOffsets(const Structure& structData) const;
			TypePtr BuildFunctionType(const Ast::ExpressionType& retType, const std::vector<Ast::ExpressionType>& parameters) const;
			TypePtr BuildPointerType(const Ast::PrimitiveType& type, SpirvStorageClass storageClass) const;
			TypePtr BuildPointerType(const Ast::ExpressionType& type, SpirvStorageClass storageClass) const;
			TypePtr BuildPointerType(const TypePtr& type, SpirvStorageClass storageClass) const;
			TypePtr BuildType(const Ast::AliasType& type) const;
			TypePtr BuildType(const Ast::ArrayType& type) const;
			TypePtr BuildType(const Ast::DynArrayType& type) const;
			TypePtr BuildType(const Ast::ExpressionType& type) const;
			TypePtr BuildType(const Ast::MatrixType& type) const;
			TypePtr BuildType(const Ast::NoType& type) const;
			TypePtr BuildType(const Ast::PrimitiveType& type) const;
			TypePtr BuildType(const Ast::PushConstantType& type) const;
			TypePtr BuildType(const Ast::SamplerType& type) const;
			TypePtr BuildType(const Ast::StorageType& type) const;
			TypePtr BuildType(const Ast::StructType& type) const;
			TypePtr BuildType(const Ast::StructDescription& structDesc, std::vector<SpirvDecoration> decorations = {}) const;
			TypePtr BuildType(const Ast::TextureType& type) const;
			TypePtr BuildType(const Ast::VectorType& type) const;
			TypePtr BuildType(const Ast::UniformType& type) const;

			std::uint32_t GetId(const Constant& c);
			std::uint32_t GetId(const Type& t);
			std::uint32_t GetId(const Variable& v);

			std::uint32_t Register(Constant c);
			std::uint32_t Register(Type t);
			std::uint32_t Register(Variable v);

			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Array& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Bool& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Float& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Function& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Image& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Integer& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Matrix& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Pointer& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const SampledImage& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Structure& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Type& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Vector& type, std::size_t arrayLength) const;
			std::size_t RegisterArrayField(FieldOffsets& fieldOffsets, const Void& type, std::size_t arrayLength) const;

			void SetStructCallback(StructCallback callback);

			void Write(SpirvSection& annotations, SpirvSection& constants, SpirvSection& debugInfos);

			SpirvConstantCache& operator=(const SpirvConstantCache& cache) = delete;
			SpirvConstantCache& operator=(SpirvConstantCache&& cache) noexcept;

		private:
			struct DepRegisterer;
			struct Eq;
			struct Internal;
			template<typename T, typename Enable = void> struct TypeBuilder;

			template<typename T> static Type BuildSingleType();

			void Write(const AnyConstant& constant, std::uint32_t resultId, SpirvSection& constants);
			void Write(const AnyType& type, std::uint32_t resultId, SpirvSection& annotations, SpirvSection& constants, SpirvSection& debugInfos);

			void WriteStruct(const Structure& structData, std::uint32_t resultId, SpirvSection& annotations, SpirvSection& constants, SpirvSection& debugInfos);

			std::unique_ptr<Internal> m_internal;
	};
}

#include <NZSL/SpirV/SpirvConstantCache.inl>

#endif // NZSL_SPIRV_SPIRVCONSTANTCACHE_HPP
