// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/TypeTransformer.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackVector.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/Types.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <fmt/format.h>
#include <unordered_map>

namespace nzsl::Ast
{
	void TypeTransformer::RegisterBuiltin()
	{
		// Primitive types
		RegisterType("bool", PrimitiveType::Boolean, std::nullopt, {});
		RegisterType("f32",  PrimitiveType::Float32, std::nullopt, {});
		RegisterType("i32",  PrimitiveType::Int32,   std::nullopt, {});
		RegisterType("u32",  PrimitiveType::UInt32,  std::nullopt, {});

		if (IsFeatureEnabled(ModuleFeature::Float64))
			RegisterType("f64", PrimitiveType::Float64, std::nullopt, {});

		// Partial types

		// Array
		RegisterType("array", PartialType {
			{ TypeParameterCategory::FullType }, { TypeParameterCategory::ConstantValue },
			[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
			{
				assert(parameterCount >= 1 && parameterCount <= 2);
				
				assert(std::holds_alternative<ExpressionType>(parameters[0]));
				const ExpressionType& exprType = std::get<ExpressionType>(parameters[0]);

				std::uint32_t lengthValue;
				if (parameterCount >= 2)
				{
					assert(std::holds_alternative<ConstantValue>(parameters[1]));
					const ConstantValue& length = std::get<ConstantValue>(parameters[1]);

					if (std::holds_alternative<std::int32_t>(length))
					{
						std::int32_t value = std::get<std::int32_t>(length);
						if (value <= 0)
							throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(value) };

						lengthValue = Nz::SafeCast<std::uint32_t>(value);
					}
					else if (std::holds_alternative<std::uint32_t>(length))
					{
						lengthValue = std::get<std::uint32_t>(length);
						if (lengthValue == 0)
							throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(lengthValue) };
					}
					else
						throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(GetConstantType(length)) };
				}
				else
					lengthValue = 0;
				
				ArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = exprType;
				arrayType.length = lengthValue;

				return arrayType;
			}
		}, std::nullopt, {});

		// Dynamic array
		RegisterType("dyn_array", PartialType {
			{ TypeParameterCategory::FullType }, {},
			[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
			{
				assert(parameterCount == 1);
				assert(std::holds_alternative<ExpressionType>(parameters[0]));

				const ExpressionType& exprType = std::get<ExpressionType>(parameters[0]);

				DynArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = exprType;

				return arrayType;
			}
		}, std::nullopt, {});

		// matX | matAxB
		for (std::size_t columnCount = 2; columnCount <= 4; ++columnCount)
		{
			for (std::size_t rowCount = 2; rowCount <= 4; ++rowCount)
			{
				std::string name;
				if (columnCount == rowCount)
					name = fmt::format("mat{}", columnCount);
				else
					name = fmt::format("mat{}x{}", columnCount, rowCount);

				RegisterType(std::move(name), PartialType{
					{ TypeParameterCategory::PrimitiveType }, {},
					[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
					{
						assert(parameterCount == 1);
						assert(std::holds_alternative<ExpressionType>(*parameters));

						const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
						assert(IsPrimitiveType(exprType));

						PrimitiveType primitiveType = std::get<PrimitiveType>(exprType);
						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64)
							throw CompilerMatrixExpectedFloatError{ sourceLocation, Ast::ToString(exprType) };

						return MatrixType {
							columnCount, rowCount, primitiveType
						};
					}
				}, std::nullopt, {});
			}
		}

		// vecX
		for (std::size_t componentCount = 2; componentCount <= 4; ++componentCount)
		{
			RegisterType("vec" + std::to_string(componentCount), PartialType {
				{ TypeParameterCategory::PrimitiveType }, {},
				[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
				{
					assert(parameterCount == 1);
					assert(std::holds_alternative<ExpressionType>(*parameters));

					const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
					assert(IsPrimitiveType(exprType));

					return VectorType {
						componentCount, std::get<PrimitiveType>(exprType)
					};
				}
			}, std::nullopt, {});
		}

		// samplers
		struct SamplerInfo
		{
			std::string_view typeName;
			ImageType imageType;
			std::optional<ModuleFeature> requiredFeature;
			bool depthSampler;
		};

		constexpr std::array<SamplerInfo, 11> samplerInfos = {
			{
				// Regular samplers
				{
					"sampler1D",
					ImageType::E1D,
					ModuleFeature::Texture1D,
					false
				},
				{
					"sampler1D_array",
					ImageType::E1D_Array,
					ModuleFeature::Texture1D,
					false
				},
				{
					"sampler2D",
					ImageType::E2D,
					std::nullopt,
					false
				},
				{
					"sampler2D_array",
					ImageType::E2D_Array,
					std::nullopt,
					false
				},
				{
					"sampler3D",
					ImageType::E3D,
					std::nullopt,
					false
				},
				{
					"sampler_cube",
					ImageType::Cubemap,
					std::nullopt,
					false
				},
				// Depth samplers
				{
					"depth_sampler1D",
					ImageType::E1D,
					ModuleFeature::Texture1D,
					true
				},
				{
					"depth_sampler1D_array",
					ImageType::E1D_Array,
					ModuleFeature::Texture1D,
					true
				},
				{
					"depth_sampler2D",
					ImageType::E2D,
					std::nullopt,
					true
				},
				{
					"depth_sampler2D_array",
					ImageType::E2D_Array,
					std::nullopt,
					true
				},
				{
					"depth_sampler_cube",
					ImageType::Cubemap,
					std::nullopt,
					true
				}
			}
		};

		for (const SamplerInfo& sampler : samplerInfos)
		{
			if (sampler.requiredFeature.has_value() && !IsFeatureEnabled(*sampler.requiredFeature))
				continue;

			RegisterType(std::string(sampler.typeName), PartialType {
				{ TypeParameterCategory::PrimitiveType }, {},
				[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
				{
					assert(parameterCount == 1);
					assert(std::holds_alternative<ExpressionType>(*parameters));

					const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
					assert(IsPrimitiveType(exprType));

					PrimitiveType primitiveType = std::get<PrimitiveType>(exprType);

					// TODO: Add support for integer samplers
					if (primitiveType != PrimitiveType::Float32)
						throw CompilerSamplerUnexpectedTypeError{ sourceLocation, Ast::ToString(primitiveType) };

					return SamplerType {
						sampler.imageType, primitiveType, sampler.depthSampler
					};
				}
			}, std::nullopt, {});
		}

		// texture
		struct TextureInfo
		{
			std::string_view typeName;
			ImageType imageType;
			std::optional<ModuleFeature> requiredFeature;
		};

		constexpr std::array<TextureInfo, 6> textureInfos = {
			{
				{
					"texture1D",
					ImageType::E1D,
					ModuleFeature::Texture1D
				},
				{
					"texture1D_array",
					ImageType::E1D_Array,
					ModuleFeature::Texture1D
				},
				{
					"texture2D",
					ImageType::E2D,
					std::nullopt
				},
				{
					"texture2D_array",
					ImageType::E2D_Array,
					std::nullopt
				},
				{
					"texture3D",
					ImageType::E3D,
					std::nullopt
				},
				{
					"texture_cube",
					ImageType::Cubemap,
					std::nullopt
				}
			}
		};

		for (const TextureInfo& texture : textureInfos)
		{
			if (texture.requiredFeature.has_value() && !IsFeatureEnabled(*texture.requiredFeature))
				continue;

			RegisterType(std::string(texture.typeName), PartialType {
				{ TypeParameterCategory::PrimitiveType, TypeParameterCategory::ConstantValue }, { TypeParameterCategory::ConstantValue },
				[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
				{
					assert(std::holds_alternative<ExpressionType>(parameters[0]));
					const ExpressionType& exprType = std::get<ExpressionType>(parameters[0]);
					assert(IsPrimitiveType(exprType));

					PrimitiveType primitiveType = std::get<PrimitiveType>(exprType);

					// TODO: Add support for integer textures
					if (primitiveType != PrimitiveType::Float32)
						throw CompilerTextureUnexpectedTypeError{ sourceLocation, Ast::ToString(primitiveType) };

					assert(std::holds_alternative<ConstantValue>(parameters[1]));
					const ConstantValue& accessValue = std::get<ConstantValue>(parameters[1]);
					if (!std::holds_alternative<std::uint32_t>(accessValue))
						throw CompilerTextureUnexpectedAccessError{ sourceLocation, "<TODO>" };

					AccessPolicy access = static_cast<AccessPolicy>(std::get<std::uint32_t>(accessValue));

					std::optional<ImageFormat> formatOpt;
					if (parameterCount >= 3)
					{
						assert(std::holds_alternative<ConstantValue>(parameters[2]));
						const ConstantValue& formatValue = std::get<ConstantValue>(parameters[2]);
						if (!std::holds_alternative<std::uint32_t>(formatValue))
							throw CompilerTextureUnexpectedFormatError{ sourceLocation, "<TODO>" };

						ImageFormat format = static_cast<ImageFormat>(std::get<std::uint32_t>(formatValue));
						if (format != ImageFormat::RGBA8) //< TODO: Add support for more formats
							throw CompilerTextureUnexpectedFormatError{ sourceLocation, "<TODO>" };

						formatOpt = format;
					}

					return TextureType {
						access, formatOpt.value_or(ImageFormat::Unknown), texture.imageType, primitiveType
					};
				}
			}, std::nullopt, {});
		}

		// storage
		RegisterType("storage", PartialType {
			{ TypeParameterCategory::StructType }, { TypeParameterCategory::ConstantValue },
			[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
			{
				assert(parameterCount >= 1);
				assert(std::holds_alternative<ExpressionType>(*parameters));

				const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
				assert(IsStructType(exprType));

				AccessPolicy access = AccessPolicy::ReadWrite;
				if (parameterCount > 1)
				{
					assert(std::holds_alternative<ConstantValue>(parameters[1]));
					const ConstantValue& accessValue = std::get<ConstantValue>(parameters[1]);
					if (!std::holds_alternative<std::uint32_t>(accessValue))
						throw CompilerStorageUnexpectedAccessError{ sourceLocation, "<TODO>" };

					access = static_cast<AccessPolicy>(std::get<std::uint32_t>(accessValue));
				}

				StructType structType = std::get<StructType>(exprType);
				return StorageType {
					access,
					structType
				};
			}
		}, std::nullopt, {});
		
		// uniform
		RegisterType("uniform", PartialType {
			{ TypeParameterCategory::StructType }, {},
			[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
			{
				assert(parameterCount == 1);
				assert(std::holds_alternative<ExpressionType>(*parameters));

				const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
				assert(IsStructType(exprType));

				StructType structType = std::get<StructType>(exprType);
				return UniformType {
					structType
				};
			}
		}, std::nullopt, {});

		// push constant
		RegisterType("push_constant", PartialType {
			{ TypeParameterCategory::StructType }, {},
			[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
			{
				assert(parameterCount == 1);
				assert(std::holds_alternative<ExpressionType>(*parameters));

				const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
				assert(IsStructType(exprType));

				StructType structType = std::get<StructType>(exprType);
				return PushConstantType {
					structType
				};
			}
		}, std::nullopt, {});

		// Intrinsics
		for (const auto& [intrinsic, data] : LangData::s_intrinsicData)
		{
			if (!data.functionName.empty())
				RegisterIntrinsic(std::string(data.functionName), intrinsic);
		}

		// Constants
		RegisterConstant("readonly", Nz::SafeCast<std::uint32_t>(AccessPolicy::ReadOnly), std::nullopt, {});
		RegisterConstant("readwrite", Nz::SafeCast<std::uint32_t>(AccessPolicy::ReadWrite), std::nullopt, {});
		RegisterConstant("writeonly", Nz::SafeCast<std::uint32_t>(AccessPolicy::WriteOnly), std::nullopt, {});

		// TODO: Register more image formats
		RegisterConstant("rgba8", Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA8), std::nullopt, {});
	}
}
