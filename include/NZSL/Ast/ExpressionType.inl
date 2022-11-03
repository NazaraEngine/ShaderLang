// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ExpressionType.hpp>
#include <Nazara/Utils/Algorithm.hpp>

namespace nzsl::Ast
{
	inline bool BaseArrayType::operator!=(const BaseArrayType& rhs) const
	{
		return !operator==(rhs);
	}

	inline bool AliasType::operator!=(const AliasType& rhs) const
	{
		return !operator==(rhs);
	}

	inline bool ArrayType::operator==(const ArrayType& rhs) const
	{
		if (length != rhs.length)
			return false;

		if (!BaseArrayType::operator==(rhs))
			return false;

		return true;
	}

	inline bool ArrayType::operator!=(const ArrayType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool DynArrayType::operator==(const DynArrayType& rhs) const
	{
		return BaseArrayType::operator==(rhs);
	}

	inline bool DynArrayType::operator!=(const DynArrayType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool FunctionType::operator==(const FunctionType& rhs) const
	{
		return funcIndex == rhs.funcIndex;
	}

	inline bool FunctionType::operator!=(const FunctionType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool IntrinsicFunctionType::operator==(const IntrinsicFunctionType& rhs) const
	{
		return intrinsic == rhs.intrinsic;
	}

	inline bool IntrinsicFunctionType::operator!=(const IntrinsicFunctionType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool MatrixType::operator==(const MatrixType& rhs) const
	{
		return columnCount == rhs.columnCount && rowCount == rhs.rowCount && type == rhs.type;
	}

	inline bool MatrixType::operator!=(const MatrixType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool MethodType::operator!=(const MethodType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool NoType::operator==(const NoType& /*rhs*/) const
	{
		return true;
	}

	inline bool NoType::operator!=(const NoType& /*rhs*/) const
	{
		return false;
	}


	inline bool SamplerType::operator==(const SamplerType& rhs) const
	{
		return dim == rhs.dim && sampledType == rhs.sampledType;
	}

	inline bool SamplerType::operator!=(const SamplerType& rhs) const
	{
		return !operator==(rhs);
	}
	
	inline bool StructType::operator==(const StructType& rhs) const
	{
		return structIndex == rhs.structIndex;
	}

	inline bool StructType::operator!=(const StructType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool Type::operator==(const Type& rhs) const
	{
		return typeIndex == rhs.typeIndex;
	}

	inline bool Type::operator!=(const Type& rhs) const
	{
		return !operator==(rhs);
	}

	inline bool VectorType::operator==(const VectorType& rhs) const
	{
		return componentCount == rhs.componentCount && type == rhs.type;
	}

	inline bool VectorType::operator!=(const VectorType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool StorageType::operator==(const StorageType& rhs) const
	{
		return containedType == rhs.containedType;
	}

	inline bool StorageType::operator!=(const StorageType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool UniformType::operator==(const UniformType& rhs) const
	{
		return containedType == rhs.containedType;
	}

	inline bool UniformType::operator!=(const UniformType& rhs) const
	{
		return !operator==(rhs);
	}

	
	inline bool IsAliasType(const ExpressionType& type)
	{
		return std::holds_alternative<AliasType>(type);
	}

	inline bool IsArrayType(const ExpressionType& type)
	{
		return std::holds_alternative<ArrayType>(type);
	}

	inline bool IsDynArrayType(const ExpressionType& type)
	{
		return std::holds_alternative<DynArrayType>(type);
	}

	inline bool IsFunctionType(const ExpressionType& type)
	{
		return std::holds_alternative<FunctionType>(type);
	}

	inline bool IsIntrinsicFunctionType(const ExpressionType& type)
	{
		return std::holds_alternative<IntrinsicFunctionType>(type);
	}

	inline bool IsMatrixType(const ExpressionType& type)
	{
		return std::holds_alternative<MatrixType>(type);
	}

	inline bool IsMethodType(const ExpressionType& type)
	{
		return std::holds_alternative<MethodType>(type);
	}

	inline bool IsNoType(const ExpressionType& type)
	{
		return std::holds_alternative<NoType>(type);
	}

	inline bool IsPrimitiveType(const ExpressionType& type)
	{
		return std::holds_alternative<PrimitiveType>(type);
	}

	inline bool IsSamplerType(const ExpressionType& type)
	{
		return std::holds_alternative<SamplerType>(type);
	}

	bool IsStorageType(const ExpressionType& type)
	{
		return std::holds_alternative<StorageType>(type);
	}

	bool IsStructType(const ExpressionType& type)
	{
		return std::holds_alternative<StructType>(type);
	}

	bool IsTypeExpression(const ExpressionType& type)
	{
		return std::holds_alternative<Type>(type);
	}

	bool IsUniformType(const ExpressionType& type)
	{
		return std::holds_alternative<UniformType>(type);
	}

	bool IsVectorType(const ExpressionType& type)
	{
		return std::holds_alternative<VectorType>(type);
	}

	bool IsStructAddressible(const ExpressionType& exprType)
	{
		return ResolveStructIndex(exprType) != std::numeric_limits<std::size_t>::max();
	}

	inline const ExpressionType& ResolveAlias(const ExpressionType& exprType)
	{
		if (IsAliasType(exprType))
		{
			const AliasType& alias = std::get<AliasType>(exprType);
			return alias.targetType->type;
		}
		else
			return exprType;
	}

}

