// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/Compare.hpp>
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
	
	ArrayType::ArrayType(const ArrayType& array) :
	length(array.length)
	{
		assert(array.containedType);
		containedType = std::make_unique<ContainedType>(*array.containedType);
	}

	ArrayType& ArrayType::operator=(const ArrayType& array)
	{
		assert(array.containedType);

		containedType = std::make_unique<ContainedType>(*array.containedType);
		length = array.length;

		return *this;
	}

	bool ArrayType::operator==(const ArrayType& rhs) const
	{
		assert(containedType);
		assert(rhs.containedType);

		if (length != rhs.length)
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

	std::string ToString(const AliasType& type, const Stringifier& stringifier)
	{
		std::string str = "alias ";
		if (stringifier.aliasStringifier)
			return fmt::format("alias {} -> {}", stringifier.aliasStringifier(type.aliasIndex), ToString(type.targetType->type));
		else
			return fmt::format("alias #{} -> {}", type.aliasIndex, ToString(type.targetType->type));
	}

	std::string ToString(const ArrayType& type, const Stringifier& stringifier)
	{
		if (type.length > 0)
			return fmt::format("array[{}, {}]", ToString(type.containedType->type, stringifier), type.length);
		else
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
			case Ast::PrimitiveType::Int32:   return "i32";
			case Ast::PrimitiveType::UInt32:  return "u32";
			case Ast::PrimitiveType::String:  return "string";
		}

		return "<unhandled primitive type>";
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

		return fmt::format("sampler{}[{}]", dimensionStr, ToString(type.sampledType));
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
