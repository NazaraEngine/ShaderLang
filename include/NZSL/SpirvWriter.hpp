// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVWRITER_HPP
#define NZSL_SPIRVWRITER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/ShaderWriter.hpp>
#include <NZSL/SpirvConstantCache.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/Module.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

namespace nzsl
{
	class SpirvSection;

	class NZSL_API SpirvWriter : public ShaderWriter
	{
		friend class SpirvAstVisitor;
		friend class SpirvBlock;
		friend class SpirvExpressionLoad;
		friend class SpirvExpressionStore;

		public:
			struct Environment;

			SpirvWriter();
			SpirvWriter(const SpirvWriter&) = delete;
			SpirvWriter(SpirvWriter&&) = delete;
			~SpirvWriter() = default;

			std::vector<std::uint32_t> Generate(const ShaderAst::Module& module, const States& states = {});

			void SetEnv(Environment environment);

			struct Environment
			{
				std::uint32_t spvMajorVersion = 1;
				std::uint32_t spvMinorVersion = 0;
			};

		private:
			struct FunctionParameter;
			struct OnlyCache {};

			std::uint32_t AllocateResultId();

			void AppendHeader();

			SpirvConstantCache::TypePtr BuildFunctionType(const ShaderAst::DeclareFunctionStatement& functionNode);

			std::uint32_t GetConstantId(const ShaderAst::ConstantValue& value) const;
			std::uint32_t GetExtendedInstructionSet(const std::string& instructionSetName) const;
			std::uint32_t GetExtVarPointerId(std::size_t varIndex) const;
			std::uint32_t GetFunctionTypeId(const ShaderAst::DeclareFunctionStatement& functionNode);
			std::uint32_t GetPointerTypeId(const ShaderAst::ExpressionType& type, SpirvStorageClass storageClass) const;
			std::uint32_t GetTypeId(const ShaderAst::ExpressionType& type) const;

			std::uint32_t RegisterConstant(const ShaderAst::ConstantValue& value);
			std::uint32_t RegisterFunctionType(const ShaderAst::DeclareFunctionStatement& functionNode);
			std::uint32_t RegisterPointerType(ShaderAst::ExpressionType type, SpirvStorageClass storageClass);
			std::uint32_t RegisterType(ShaderAst::ExpressionType type);

			static void MergeSections(std::vector<std::uint32_t>& output, const SpirvSection& from);

			struct Context
			{
				const States* states = nullptr;
			};

			struct State;

			Context m_context;
			Environment m_environment;
			State* m_currentState;
	};
}

#include <NZSL/SpirvWriter.inl>

#endif // NZSL_SPIRVWRITER_HPP
