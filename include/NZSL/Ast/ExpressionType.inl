// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/Hash.hpp>

namespace nzsl::Ast
{
	inline auto& BaseArrayType::InnerType()
	{
		return containedType->type;
	}

	inline const auto& BaseArrayType::InnerType() const
	{
		return containedType->type;
	}

	template<typename T>
	void BaseArrayType::SetupInnerType(T&& value)
	{
		containedType = std::make_unique<ContainedType>();
		containedType->type = std::forward<T>(value);
	}

	inline bool BaseArrayType::operator!=(const BaseArrayType& rhs) const
	{
		return !operator==(rhs);
	}

	inline auto& AliasType::TargetType()
	{
		return targetType->type;
	}

	inline const auto& AliasType::TargetType() const
	{
		return targetType->type;
	}

	template<typename T>
	void AliasType::SetupTargetType(T&& value)
	{
		targetType = std::make_unique<ContainedType>();
		targetType->type = std::forward<T>(value);
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


	inline bool NamedExternalBlockType::operator==(const NamedExternalBlockType& rhs) const
	{
		return namedExternalBlockIndex == rhs.namedExternalBlockIndex;
	}

	inline bool NamedExternalBlockType::operator!=(const NamedExternalBlockType& rhs) const
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


	inline bool ImplicitArrayType::operator==(const ImplicitArrayType& /*rhs*/) const
	{
		return true;
	}

	inline bool ImplicitArrayType::operator!=(const ImplicitArrayType& rhs) const
	{
		return !operator==(rhs);
	}


	inline bool ImplicitVectorType::operator==(const ImplicitVectorType& rhs) const
	{
		return componentCount == rhs.componentCount;
	}

	inline bool ImplicitVectorType::operator!=(const ImplicitVectorType& rhs) const
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


	inline bool ModuleType::operator==(const ModuleType& rhs) const
	{
		return moduleIndex == rhs.moduleIndex;
	}

	inline bool ModuleType::operator!=(const ModuleType& rhs) const
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
		return dim == rhs.dim && sampledType == rhs.sampledType && depth == rhs.depth;
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


	inline bool TextureType::operator==(const TextureType& rhs) const
	{
		return accessPolicy == rhs.accessPolicy && format == rhs.format && dim == rhs.dim && baseType == rhs.baseType;
	}

	inline bool TextureType::operator!=(const TextureType& rhs) const
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
		return accessPolicy == rhs.accessPolicy && containedType == rhs.containedType;
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


	inline bool PushConstantType::operator==(const PushConstantType& rhs) const
	{
		return containedType == rhs.containedType;
	}

	inline bool PushConstantType::operator!=(const PushConstantType& rhs) const
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

	inline bool IsImplicitType(const ExpressionType& type)
	{
		return IsImplicitArrayType(type) || IsImplicitVectorType(type);
	}

	inline bool IsImplicitArrayType(const ExpressionType& type)
	{
		return std::holds_alternative<ImplicitArrayType>(type);
	}

	inline bool IsImplicitVectorType(const ExpressionType& type)
	{
		return std::holds_alternative<ImplicitVectorType>(type);
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

	inline bool IsModuleType(const ExpressionType& type)
	{
		return std::holds_alternative<ModuleType>(type);
	}

	inline bool IsNamedExternalBlockType(const ExpressionType& type)
	{
		return std::holds_alternative<NamedExternalBlockType>(type);
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

	inline bool IsStorageType(const ExpressionType& type)
	{
		return std::holds_alternative<StorageType>(type);
	}

	inline bool IsStructType(const ExpressionType& type)
	{
		return std::holds_alternative<StructType>(type);
	}

	inline bool IsTextureType(const ExpressionType& type)
	{
		return std::holds_alternative<TextureType>(type);
	}

	inline bool IsTypeExpression(const ExpressionType& type)
	{
		return std::holds_alternative<Type>(type);
	}

	inline bool IsUniformType(const ExpressionType& type)
	{
		return std::holds_alternative<UniformType>(type);
	}

	inline bool IsPushConstantType(const ExpressionType& type)
	{
		return std::holds_alternative<PushConstantType>(type);
	}

	inline bool IsVectorType(const ExpressionType& type)
	{
		return std::holds_alternative<VectorType>(type);
	}

	inline bool IsConstantType(const ExpressionType& exprType)
	{
		if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);
			if (arrayType.isWrapped)
				return false;

			const ExpressionType& innerType = arrayType.InnerType();
			return IsConstantType(innerType);
		}
		else
			return IsPrimitiveType(exprType) || IsVectorType(exprType);
	}

	inline bool IsExternalPointerType(const ExpressionType& exprType)
	{
		if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);

			const ExpressionType& innerType = arrayType.InnerType();
			return IsExternalPointerType(innerType);
		}
		else
			return IsSamplerType(exprType) || IsTextureType(exprType);
	}

	inline bool IsLiteralType(const ExpressionType& exprType)
	{
		if (IsArrayType(exprType))
			return IsLiteralType(std::get<ArrayType>(exprType).InnerType());

		PrimitiveType primType;
		if (IsPrimitiveType(exprType))
			primType = std::get<PrimitiveType>(exprType);
		else if (IsVectorType(exprType))
			primType = std::get<VectorType>(exprType).type;
		else
			return false;

		switch (primType)
		{
			case PrimitiveType::Boolean:
			case PrimitiveType::Float32:
			case PrimitiveType::Float64:
			case PrimitiveType::Int32:
			case PrimitiveType::UInt32:
			case PrimitiveType::String:
				return false;

			case PrimitiveType::FloatLiteral:
			case PrimitiveType::IntLiteral:
				return true;
		}

		NAZARA_UNREACHABLE();
	}

	inline bool IsStructAddressible(const ExpressionType& exprType)
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

	inline ExpressionType UnwrapExternalType(const ExpressionType& exprType)
	{
		const ExpressionType& resolvedExprType = ResolveAlias(exprType);

		if (IsStorageType(resolvedExprType))
			return std::get<StorageType>(resolvedExprType).containedType;
		else if (IsUniformType(resolvedExprType))
			return std::get<UniformType>(resolvedExprType).containedType;
		else if (IsArrayType(resolvedExprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);
			if (arrayType.isWrapped)
			{
				ArrayType unwrappedArrayType;
				unwrappedArrayType.SetupInnerType(UnwrapExternalType(arrayType.InnerType()));
				unwrappedArrayType.length = arrayType.length;

				return unwrappedArrayType;
			}
		}

		return exprType;
	}

	template<typename T>
	ExpressionType WrapExternalType(const ExpressionType& exprType)
	{
		if (IsStructType(exprType))
		{
			std::size_t innerStructIndex = std::get<StructType>(exprType).structIndex;

			T wrappedType;
			wrappedType.containedType = StructType{ innerStructIndex };

			return wrappedType;
		}
		else if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);

			ArrayType wrappedArrayType;
			wrappedArrayType.SetupInnerType(WrapExternalType<T>(arrayType.InnerType()));
			wrappedArrayType.length = arrayType.length;
			wrappedArrayType.isWrapped = true;

			return wrappedArrayType;
		}
		else if (IsDynArrayType(exprType))
		{
			const DynArrayType& arrayType = std::get<DynArrayType>(exprType);

			DynArrayType wrappedDynArrayType;
			wrappedDynArrayType.SetupInnerType(WrapExternalType<T>(arrayType.InnerType()));
			wrappedDynArrayType.isWrapped = true;

			return wrappedDynArrayType;
		}
		else
			return exprType;
	}

	inline ExpressionType WrapExternalType(const ExpressionType& exprType, const ExpressionType& referenceType)
	{
		if (IsStorageType(referenceType))
			return WrapExternalType<StorageType>(exprType);
		else if (IsUniformType(referenceType))
			return WrapExternalType<StorageType>(exprType);
		else if (IsArrayType(referenceType))
			return WrapExternalType(exprType, std::get<ArrayType>(referenceType).InnerType());
		else if (IsDynArrayType(referenceType))
			return WrapExternalType(exprType, std::get<DynArrayType>(referenceType).InnerType());
		else
			return exprType;
	}
}

namespace std
{
	template<>
	struct hash<nzsl::Ast::NoType>
	{
		std::size_t operator()(const nzsl::Ast::NoType&) const
		{
			return 0;
		}
	};

	template<>
	struct hash<nzsl::Ast::AliasType>
	{
		std::size_t operator()(const nzsl::Ast::AliasType& aliasType) const
		{
			std::size_t h = Nz::HashCombine(aliasType.aliasIndex);
			if (aliasType.targetType)
				Nz::HashCombine(h, aliasType.TargetType());

			return h;
		}
	};

	template<>
	struct hash<nzsl::Ast::ArrayType>
	{
		std::size_t operator()(const nzsl::Ast::ArrayType& arrayType) const
		{
			std::size_t h = Nz::HashCombine(arrayType.length);
			if (arrayType.containedType)
				Nz::HashCombine(h, arrayType.InnerType());

			return h;
		}
	};

	template<>
	struct hash<nzsl::Ast::DynArrayType>
	{
		std::size_t operator()(const nzsl::Ast::DynArrayType& dynArrayType) const
		{
			std::size_t h = 1;
			if (dynArrayType.containedType)
				Nz::HashCombine(h, dynArrayType.InnerType());

			return h;
		}
	};

	template<>
	struct hash<nzsl::Ast::FunctionType>
	{
		std::size_t operator()(const nzsl::Ast::FunctionType& functionType) const
		{
			return Nz::HashCombine(functionType.funcIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::ImplicitArrayType>
	{
		std::size_t operator()(const nzsl::Ast::ImplicitArrayType& /*arrayType*/) const
		{
			return 1;
		}
	};

	template<>
	struct hash<nzsl::Ast::ImplicitVectorType>
	{
		std::size_t operator()(const nzsl::Ast::ImplicitVectorType& vectorType) const
		{
			return Nz::HashCombine(vectorType.componentCount);
		}
	};

	template<>
	struct hash<nzsl::Ast::IntrinsicFunctionType>
	{
		std::size_t operator()(const nzsl::Ast::IntrinsicFunctionType& functionType) const
		{
			return Nz::HashCombine(functionType.intrinsic);
		}
	};

	template<>
	struct hash<nzsl::Ast::MatrixType>
	{
		std::size_t operator()(const nzsl::Ast::MatrixType& matrixType) const
		{
			return Nz::HashCombine(matrixType.columnCount, matrixType.rowCount, matrixType.type);
		}
	};

	template<>
	struct hash<nzsl::Ast::MethodType>
	{
		std::size_t operator()(const nzsl::Ast::MethodType& methodType) const
		{
			std::size_t h = Nz::HashCombine(methodType.methodIndex);
			if (methodType.objectType)
				Nz::HashCombine(h, methodType.objectType->type);

			return h;
		}
	};

	template<>
	struct hash<nzsl::Ast::ModuleType>
	{
		std::size_t operator()(const nzsl::Ast::ModuleType& moduleType) const
		{
			return Nz::HashCombine(moduleType.moduleIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::NamedExternalBlockType>
	{
		std::size_t operator()(const nzsl::Ast::NamedExternalBlockType& namedExternalBlockType) const
		{
			return Nz::HashCombine(namedExternalBlockType.namedExternalBlockIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::PushConstantType>
	{
		std::size_t operator()(const nzsl::Ast::PushConstantType& pushConstantType) const
		{
			return Nz::HashCombine(pushConstantType.containedType.structIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::SamplerType>
	{
		std::size_t operator()(const nzsl::Ast::SamplerType& samplerType) const
		{
			return Nz::HashCombine(samplerType.depth, samplerType.dim, samplerType.sampledType);
		}
	};

	template<>
	struct hash<nzsl::Ast::StorageType>
	{
		std::size_t operator()(const nzsl::Ast::StorageType& storageType) const
		{
			return Nz::HashCombine(storageType.accessPolicy, storageType.containedType.structIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::StructType>
	{
		std::size_t operator()(const nzsl::Ast::StructType& structType) const
		{
			return Nz::HashCombine(structType.structIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::TextureType>
	{
		std::size_t operator()(const nzsl::Ast::TextureType& textureType) const
		{
			return Nz::HashCombine(textureType.accessPolicy, textureType.baseType, textureType.dim, textureType.format);
		}
	};

	template<>
	struct hash<nzsl::Ast::Type>
	{
		std::size_t operator()(const nzsl::Ast::Type& type) const
		{
			return Nz::HashCombine(type.typeIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::UniformType>
	{
		std::size_t operator()(const nzsl::Ast::UniformType& uniformType) const
		{
			return Nz::HashCombine(uniformType.containedType.structIndex);
		}
	};

	template<>
	struct hash<nzsl::Ast::VectorType>
	{
		std::size_t operator()(const nzsl::Ast::VectorType& vectorType) const
		{
			return Nz::HashCombine(vectorType.componentCount, vectorType.type);
		}
	};
}
