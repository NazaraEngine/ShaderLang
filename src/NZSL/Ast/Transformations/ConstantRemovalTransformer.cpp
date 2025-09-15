// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ConstantRemovalTransformer.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template<typename T>
		struct GetVectorInnerType
		{
			static constexpr bool IsVector = false;

			using type = T; //< fallback
		};

		template<typename T>
		struct GetVectorInnerType<std::vector<T>>
		{
			static constexpr bool IsVector = true;

			using type = T;
		};
	}

	bool ConstantRemovalTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		m_constantSingleValues.clear();
		m_optionValues.clear();

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto ConstantRemovalTransformer::Transform(IdentifierValueExpression&& identifierValueExpr) -> ExpressionTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (identifierValueExpr.identifierType != IdentifierType::Constant)
			return VisitChildren{};

		if (auto constIt = m_constantSingleValues.find(identifierValueExpr.identifierIndex); constIt != m_constantSingleValues.end())
		{
			return std::visit([&](auto&& arg) -> ExpressionTransformation
			{
				return ReplaceExpression{ ShaderBuilder::ConstantValue(arg, identifierValueExpr.sourceLocation) };
			}, constIt->second);
		}
		else if (auto optIt = m_optionValues.find(identifierValueExpr.identifierIndex); optIt != m_optionValues.end())
		{
			return std::visit([&](auto&& arg) -> ExpressionTransformation
			{
				using T = std::decay_t<decltype(arg)>;

				using VectorInner = GetVectorInnerType<T>;

				if constexpr (VectorInner::IsVector)
					return DontVisitChildren{};
				else
					return ReplaceExpression{ ShaderBuilder::ConstantValue(arg, identifierValueExpr.sourceLocation) };
			}, optIt->second);
		}

		return DontVisitChildren{};
	}

	auto ConstantRemovalTransformer::Transform(IntrinsicExpression&& intrinsicExpr) -> ExpressionTransformation
	{
		if (!m_options->removeConstArraySize)
			return VisitChildren{};

		HandleChildren(intrinsicExpr);

		if (intrinsicExpr.intrinsic == IntrinsicType::ArraySize)
		{
			assert(!intrinsicExpr.parameters.empty());
			const ExpressionType* paramType = GetExpressionType(*intrinsicExpr.parameters.front());
			if (!paramType)
				return DontVisitChildren{};

			if (IsArrayType(*paramType))
			{
				const ArrayType& arrayType = std::get<ArrayType>(*paramType);
				return ReplaceExpression{ ShaderBuilder::ConstantValue(arrayType.length, intrinsicExpr.sourceLocation) };
			}
		}

		return DontVisitChildren{};
	}

	auto ConstantRemovalTransformer::Transform(TypeConstantExpression&& typeConstantExpr) -> ExpressionTransformation
	{
		if (!m_options->removeTypeConstant)
			return VisitChildren{};

		HandleChildren(typeConstantExpr);
		return ReplaceExpression{ ShaderBuilder::ConstantValue(ComputeTypeConstant(typeConstantExpr.type, typeConstantExpr.typeConstant)) };
	}

	auto ConstantRemovalTransformer::Transform(DeclareConstStatement&& declConst) -> StatementTransformation
	{
		if (!declConst.constIndex)
			return VisitChildren{}; //< option has not been resolved yet

		HandleChildren(declConst);

		const auto& constantData = m_context->constants.Retrieve(*declConst.constIndex, declConst.sourceLocation);
		if (!constantData.value)
			return DontVisitChildren{};

		if (IsArrayConstant(*constantData.value))
		{
			if (m_options->replaceExpressionWithValue && declConst.expression->GetType() != NodeType::ConstantArrayValueExpression)
				declConst.expression = ShaderBuilder::ConstantArrayValue(ToArrayConstantValue(*constantData.value));

			return DontVisitChildren{}; //< keep const arrays
		}
		else
		{
			if (m_options->replaceExpressionWithValue && declConst.expression->GetType() != NodeType::ConstantValueExpression)
				declConst.expression = ShaderBuilder::ConstantValue(ToSingleConstantValue(*constantData.value));
		}

		m_constantSingleValues.emplace(*declConst.constIndex, ToSingleConstantValue(*constantData.value));

		if (m_options->removeConstantDeclaration)
			return RemoveStatement{};
		else
			return DontVisitChildren{};
	}

	auto ConstantRemovalTransformer::Transform(DeclareOptionStatement&& declOption) -> StatementTransformation
	{
		if (!declOption.optIndex)
			return VisitChildren{}; //< option has not been resolved yet

		HandleChildren(declOption);

		OptionHash optionHash = HashOption(declOption.optName.data());
		auto optionValueIt = m_context->optionValues.find(optionHash);
		if (optionValueIt != m_context->optionValues.end())
			m_optionValues.emplace(*declOption.optIndex, optionValueIt->second);
		else
		{
			if (m_context->partialCompilation)
				return DontVisitChildren{}; //< option has no value in partial compilation, don't remove

			if (!declOption.defaultValue)
				throw CompilerMissingOptionValueError{ declOption.sourceLocation, declOption.optName };

			const auto& constantData = m_context->constants.Retrieve(*declOption.optIndex, declOption.sourceLocation);
			if (IsSingleConstant(*constantData.value))
				m_constantSingleValues.emplace(*declOption.optIndex, ToSingleConstantValue(*constantData.value));
		}

		if (m_options->removeOptionDeclaration)
			return RemoveStatement{};
		else
			return DontVisitChildren{};
	}
}
