// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ExpressionType.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/TypeList.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/Compare.hpp>
#include <NZSL/Math/FieldOffsets.hpp>
#include <fmt/format.h>

namespace nzsl::Ast
{
	AliasType::AliasType(const AliasType& alias) :
	aliasIndex(alias.aliasIndex)
	{
		assert(alias.targetType);
		targetType = std::make_unique<ContainedType>(*alias.targetType);
	}

	AliasType& AliasType::operator=(const AliasType& alias)
	{
		aliasIndex = alias.aliasIndex;

		assert(alias.targetType);
		targetType = std::make_unique<ContainedType>(*alias.targetType);

		return *this;
	}

	bool AliasType::operator==(const AliasType& rhs) const
	{
		assert(targetType);
		assert(rhs.targetType);

		if (aliasIndex != rhs.aliasIndex)
			return false;

		if (targetType->type != rhs.targetType->type)
			return false;

		return true;
	}


	BaseArrayType::BaseArrayType(const BaseArrayType& array)
	{
		assert(array.containedType);
		containedType = std::make_unique<ContainedType>(*array.containedType);
	}

	BaseArrayType& BaseArrayType::operator=(const BaseArrayType& array)
	{
		assert(array.containedType);

		containedType = std::make_unique<ContainedType>(*array.containedType);

		return *this;
	}

	bool BaseArrayType::operator==(const BaseArrayType& rhs) const
	{
		assert(containedType);
		assert(rhs.containedType);

		if (isWrapped != rhs.isWrapped)
			return false;

		if (containedType->type != rhs.containedType->type)
			return false;

		return true;
	}


	MethodType::MethodType(const MethodType& methodType) :
	methodIndex(methodType.methodIndex)
	{
		assert(methodType.objectType);
		objectType = std::make_unique<ContainedType>(*methodType.objectType);
	}

	MethodType& MethodType::operator=(const MethodType& methodType)
	{
		assert(methodType.objectType);

		methodIndex = methodType.methodIndex;
		objectType = std::make_unique<ContainedType>(*methodType.objectType);

		return *this;
	}

	bool MethodType::operator==(const MethodType& rhs) const
	{
		assert(objectType);
		assert(rhs.objectType);
		return objectType->type == rhs.objectType->type && methodIndex == rhs.methodIndex;
	}
	
	using ForbiddenStructTypes = Nz::TypeList<AliasType, FunctionType, IntrinsicFunctionType, MethodType, NoType, PushConstantType, SamplerType, StorageType, TextureType, Type, UniformType>;

	std::size_t RegisterStructField(FieldOffsets& fieldOffsets, const ExpressionType& type, const StructFinder& structFinder)
	{
		return std::visit([&](auto&& arg) -> std::size_t
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (!Nz::TypeListHas<ForbiddenStructTypes, T>)
				return RegisterStructFieldType(fieldOffsets, arg, structFinder);
			else
				throw std::runtime_error("unexpected type (" + ToString(arg) + ") as struct field");
		}, ResolveAlias(type));
	}

	std::size_t RegisterStructField(FieldOffsets& fieldOffsets, const ExpressionType& type, std::size_t arraySize, const StructFinder& structFinder)
	{
		return std::visit([&](auto&& arg) -> std::size_t
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (!Nz::TypeListHas<ForbiddenStructTypes, T>)
				return RegisterStructFieldType(fieldOffsets, arg, arraySize, structFinder);
			else
				throw std::runtime_error("unexpected type (" + ToString(arg) + ") as struct field");
		}, ResolveAlias(type));
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const ArrayType& arrayType, const StructFinder& structFinder)
	{
		return RegisterStructField(fieldOffsets, arrayType.containedType->type, arrayType.length, structFinder);
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const ArrayType& arrayType, std::size_t arraySize, const StructFinder& structFinder)
	{
		FieldOffsets dummyStruct(fieldOffsets.GetLayout());
		RegisterStructField(dummyStruct, arrayType.containedType->type, arrayType.length, structFinder);

		return fieldOffsets.AddStructArray(dummyStruct, arraySize);
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const DynArrayType& arrayType, const StructFinder& structFinder)
	{
		FieldOffsets dummyStruct(fieldOffsets.GetLayout());
		RegisterStructField(dummyStruct, arrayType.containedType->type, 1, structFinder);

		return fieldOffsets.AddStruct(dummyStruct);
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const DynArrayType& arrayType, std::size_t arraySize, const StructFinder& structFinder)
	{
		FieldOffsets dummyStruct(fieldOffsets.GetLayout());
		RegisterStructField(dummyStruct, arrayType.containedType->type, 1, structFinder);

		return fieldOffsets.AddStructArray(dummyStruct, arraySize);
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const MatrixType& matrixType, const StructFinder& /*structFinder*/)
	{
		switch (matrixType.type)
		{
			case PrimitiveType::Boolean: return fieldOffsets.AddMatrix(StructFieldType::Bool1,   Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true);
			case PrimitiveType::Float32: return fieldOffsets.AddMatrix(StructFieldType::Float1,  Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true);
			case PrimitiveType::Float64: return fieldOffsets.AddMatrix(StructFieldType::Double1, Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true);
			case PrimitiveType::Int32:   return fieldOffsets.AddMatrix(StructFieldType::Int1,    Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true);
			case PrimitiveType::UInt32:  return fieldOffsets.AddMatrix(StructFieldType::UInt1,   Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true);
			case PrimitiveType::String:  break;
		}

		throw std::runtime_error("unexpected type");
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const MatrixType& matrixType, std::size_t arraySize, const StructFinder& /*structFinder*/)
	{
		switch (matrixType.type)
		{
			case PrimitiveType::Boolean: return fieldOffsets.AddMatrixArray(StructFieldType::Bool1,   Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true, arraySize);
			case PrimitiveType::Float32: return fieldOffsets.AddMatrixArray(StructFieldType::Float1,  Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true, arraySize);
			case PrimitiveType::Float64: return fieldOffsets.AddMatrixArray(StructFieldType::Double1, Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true, arraySize);
			case PrimitiveType::Int32:   return fieldOffsets.AddMatrixArray(StructFieldType::Int1,    Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true, arraySize);
			case PrimitiveType::UInt32:  return fieldOffsets.AddMatrixArray(StructFieldType::UInt1,   Nz::SafeCast<unsigned int>(matrixType.columnCount), Nz::SafeCast<unsigned int>(matrixType.rowCount), true, arraySize);
			case PrimitiveType::String:  break;
		}

		throw std::runtime_error("unexpected type");
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const PrimitiveType& primitiveType, const StructFinder& /*structFinder*/)
	{
		switch (primitiveType)
		{
			case PrimitiveType::Boolean: return fieldOffsets.AddField(StructFieldType::Bool1);
			case PrimitiveType::Float32: return fieldOffsets.AddField(StructFieldType::Float1);
			case PrimitiveType::Float64: return fieldOffsets.AddField(StructFieldType::Double1);
			case PrimitiveType::Int32:   return fieldOffsets.AddField(StructFieldType::Int1);
			case PrimitiveType::UInt32:  return fieldOffsets.AddField(StructFieldType::UInt1);
			case PrimitiveType::String:  break;
		}

		throw std::runtime_error("unexpected type");
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const PrimitiveType& primitiveType, std::size_t arraySize, const StructFinder& /*structFinder*/)
	{
		switch (primitiveType)
		{
			case PrimitiveType::Boolean: return fieldOffsets.AddFieldArray(StructFieldType::Bool1, arraySize);
			case PrimitiveType::Float32: return fieldOffsets.AddFieldArray(StructFieldType::Float1, arraySize);
			case PrimitiveType::Float64: return fieldOffsets.AddFieldArray(StructFieldType::Double1, arraySize);
			case PrimitiveType::Int32:   return fieldOffsets.AddFieldArray(StructFieldType::Int1, arraySize);
			case PrimitiveType::UInt32:  return fieldOffsets.AddFieldArray(StructFieldType::UInt1, arraySize);
			case PrimitiveType::String:  break;
		}

		throw std::runtime_error("unexpected type");
	}
	
	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const StructType& structType, const StructFinder& structFinder)
	{
		if (!structFinder)
			throw std::runtime_error("struct found with no missing struct finder");

		const FieldOffsets& innerFieldOffsets = structFinder(structType.structIndex);
		return fieldOffsets.AddStruct(innerFieldOffsets);
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const StructType& structType, std::size_t arraySize, const StructFinder& structFinder)
	{
		if (!structFinder)
			throw std::runtime_error("struct found with no missing struct finder");

		const FieldOffsets& innerFieldOffsets = structFinder(structType.structIndex);
		return fieldOffsets.AddStructArray(innerFieldOffsets, arraySize);
	}

	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const VectorType& vectorType, const StructFinder& /*structFinder*/)
	{
		assert(vectorType.componentCount >= 1 && vectorType.componentCount <= 4);

		switch (vectorType.type)
		{
			case PrimitiveType::Boolean: return fieldOffsets.AddField(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Bool1) + vectorType.componentCount - 1));
			case PrimitiveType::Float32: return fieldOffsets.AddField(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Float1) + vectorType.componentCount - 1));
			case PrimitiveType::Float64: return fieldOffsets.AddField(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Double1) + vectorType.componentCount - 1));
			case PrimitiveType::Int32:   return fieldOffsets.AddField(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Int1) + vectorType.componentCount - 1));
			case PrimitiveType::UInt32:  return fieldOffsets.AddField(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::UInt1) + vectorType.componentCount - 1));
			case PrimitiveType::String:  break;
		}

		throw std::runtime_error("unexpected type");
	}
	
	std::size_t RegisterStructFieldType(FieldOffsets& fieldOffsets, const VectorType& vectorType, std::size_t arraySize, const StructFinder& /*structFinder*/)
	{
		assert(vectorType.componentCount >= 1 && vectorType.componentCount <= 4);

		switch (vectorType.type)
		{
			case PrimitiveType::Boolean: return fieldOffsets.AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Bool1) + vectorType.componentCount - 1), arraySize);
			case PrimitiveType::Float32: return fieldOffsets.AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Float1) + vectorType.componentCount - 1), arraySize);
			case PrimitiveType::Float64: return fieldOffsets.AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Double1) + vectorType.componentCount - 1), arraySize);
			case PrimitiveType::Int32:   return fieldOffsets.AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::Int1) + vectorType.componentCount - 1), arraySize);
			case PrimitiveType::UInt32:  return fieldOffsets.AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(StructFieldType::UInt1) + vectorType.componentCount - 1), arraySize);
			case PrimitiveType::String:  break;
		}

		throw std::runtime_error("unexpected type");
	}
	
	std::size_t ResolveStructIndex(const AliasType& aliasType)
	{
		return ResolveStructIndex(aliasType.targetType->type);
	}

	std::size_t ResolveStructIndex(const ExpressionType& exprType)
	{
		return std::visit([&](auto&& arg) -> std::size_t
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, StorageType> || std::is_same_v<T, StructType> || std::is_same_v<T, UniformType> || std::is_same_v<T, AliasType> || std::is_same_v<T, PushConstantType>)
				return ResolveStructIndex(arg);
			else if constexpr (std::is_same_v<T, NoType> ||
			                   std::is_same_v<T, ArrayType> ||
			                   std::is_same_v<T, DynArrayType> ||
			                   std::is_same_v<T, FunctionType> ||
			                   std::is_same_v<T, IntrinsicFunctionType> ||
			                   std::is_same_v<T, PrimitiveType> ||
			                   std::is_same_v<T, MatrixType> ||
			                   std::is_same_v<T, MethodType> ||
			                   std::is_same_v<T, SamplerType> ||
			                   std::is_same_v<T, TextureType> ||
			                   std::is_same_v<T, Type> ||
			                   std::is_same_v<T, VectorType>)
			{
				return std::numeric_limits<std::size_t>::max();
			}
			else
				static_assert(Nz::AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, exprType);
	}

	std::size_t ResolveStructIndex(const StorageType& structType)
	{
		return structType.containedType.structIndex;
	}

	std::size_t ResolveStructIndex(const StructType& structType)
	{
		return structType.structIndex;
	}

	std::size_t ResolveStructIndex(const UniformType& uniformType)
	{
		return uniformType.containedType.structIndex;
	}

	std::size_t ResolveStructIndex(const PushConstantType& pushConstantType)
	{
		return pushConstantType.containedType.structIndex;
	}

	std::string ToString(const AliasType& type, const Stringifier& stringifier)
	{
		if (stringifier.aliasStringifier)
			return fmt::format("alias {} -> {}", stringifier.aliasStringifier(type.aliasIndex), ToString(type.targetType->type, stringifier));
		else
			return fmt::format("alias #{} -> {}", type.aliasIndex, ToString(type.targetType->type, stringifier));
	}

	std::string ToString(const ArrayType& type, const Stringifier& stringifier)
	{
		if (type.length > 0)
			return fmt::format("array[{}, {}]", ToString(type.containedType->type, stringifier), type.length);
		else
			return fmt::format("array[{}]", ToString(type.containedType->type, stringifier));
	}

	std::string ToString(const DynArrayType& type, const Stringifier& stringifier)
	{
		return fmt::format("dyn_array[{}]", ToString(type.containedType->type, stringifier));
	}

	std::string ToString(const ExpressionType& type, const Stringifier& stringifier)
	{
		return std::visit([&](auto&& arg)
		{
			return ToString(arg, stringifier);
		}, type);
	}

	std::string ToString(const FunctionType& /*type*/, const Stringifier& /*stringifier*/)
	{
		return "<function type>";
	}

	std::string ToString(const IntrinsicFunctionType& /*type*/, const Stringifier& /*stringifier*/)
	{
		return "<intrinsic function type>";
	}

	std::string ToString(const MatrixType& type, const Stringifier& /*stringifier*/)
	{
		if (type.columnCount == type.rowCount)
			return fmt::format("mat{}[{}]", type.columnCount, ToString(type.type));
		else
			return fmt::format("mat{}x{}[{}]", type.columnCount, type.rowCount, ToString(type.type));
	}

	std::string ToString(const MethodType& type, const Stringifier& /*stringifier*/)
	{
		return "<method of object " + ToString(type.objectType->type) + " type>";
	}

	std::string ToString(NoType /*type*/, const Stringifier& /*stringifier*/)
	{
		return "()";
	}

	std::string ToString(PrimitiveType type, const Stringifier& /*stringifier*/)
	{
		switch (type)
		{
			case Ast::PrimitiveType::Boolean: return "bool";
			case Ast::PrimitiveType::Float32: return "f32";
			case Ast::PrimitiveType::Float64: return "f64";
			case Ast::PrimitiveType::Int32:   return "i32";
			case Ast::PrimitiveType::UInt32:  return "u32";
			case Ast::PrimitiveType::String:  return "string";
		}

		return "<unhandled primitive type>";
	}

	std::string ToString(const PushConstantType& type, const Stringifier& stringifier)
	{
		return fmt::format("push_constant[{}]", ToString(type.containedType, stringifier));
	}

	std::string ToString(const SamplerType& type, const Stringifier& /*stringifier*/)
	{
		std::string_view dimensionStr;
		switch (type.dim)
		{
			case ImageType::E1D:       dimensionStr = "1D";      break;
			case ImageType::E1D_Array: dimensionStr = "1DArray"; break;
			case ImageType::E2D:       dimensionStr = "2D";      break;
			case ImageType::E2D_Array: dimensionStr = "2DArray"; break;
			case ImageType::E3D:       dimensionStr = "3D";      break;
			case ImageType::Cubemap:   dimensionStr = "Cube";    break;
		}

		return fmt::format("{}sampler{}[{}]", (type.depth) ? "depth_" : "", dimensionStr, ToString(type.sampledType));
	}

	std::string ToString(const StorageType& type, const Stringifier& stringifier)
	{
		return fmt::format("storage[{}]", ToString(type.containedType, stringifier));
	}

	std::string ToString(const StructType& type, const Stringifier& stringifier)
	{
		if (stringifier.structStringifier)
			return "struct " + stringifier.structStringifier(type.structIndex);
		else
			return "struct #" + std::to_string(type.structIndex);
	}

	std::string ToString(const TextureType& type, const Stringifier& /*stringifier*/)
	{
		std::string_view dimensionStr;
		switch (type.dim)
		{
			case ImageType::E1D:       dimensionStr = "1D";      break;
			case ImageType::E1D_Array: dimensionStr = "1DArray"; break;
			case ImageType::E2D:       dimensionStr = "2D";      break;
			case ImageType::E2D_Array: dimensionStr = "2DArray"; break;
			case ImageType::E3D:       dimensionStr = "3D";      break;
			case ImageType::Cubemap:   dimensionStr = "Cube";    break;
		}

		return fmt::format("texture{}[{}]", dimensionStr, ToString(type.baseType));
	}

	std::string ToString(const Type& type, const Stringifier& stringifier)
	{
		if (stringifier.typeStringifier)
			return "type " + stringifier.typeStringifier(type.typeIndex);
		else
			return "type #" + std::to_string(type.typeIndex);
	}

	std::string ToString(const UniformType& type, const Stringifier& stringifier)
	{
		return fmt::format("uniform[{}]", ToString(type.containedType, stringifier));
	}

	std::string ToString(const VectorType& type, const Stringifier& /*stringifier*/)
	{
		return fmt::format("vec{}[{}]", type.componentCount, ToString(type.type));
	}
}
