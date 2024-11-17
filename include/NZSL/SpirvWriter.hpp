// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVWRITER_HPP
#define NZSL_SPIRVWRITER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/ShaderWriter.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <NZSL/SpirV/SpirvConstantCache.hpp>
#include <NZSL/SpirV/SpirvVariable.hpp>
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
		class PreVisitor;
		friend PreVisitor;

		public:
			struct Environment;

			SpirvWriter();
			SpirvWriter(const SpirvWriter&) = delete;
			SpirvWriter(SpirvWriter&&) = delete;
			~SpirvWriter() = default;

			std::vector<std::uint32_t> Generate(const Ast::Module& module, const States& states = {});

			const SpirvVariable& GetConstantVariable(std::size_t constIndex) const;

			bool IsVersionGreaterOrEqual(std::uint32_t spvMajor, std::uint32_t spvMinor) const;

			void SetEnv(Environment environment);

			struct Environment
			{
				std::uint32_t spvMajorVersion = 1;
				std::uint32_t spvMinorVersion = 0;
			};
			
			static std::pair<std::uint32_t, std::uint32_t> GetMaximumSupportedVersion(std::uint32_t vkMajorVersion, std::uint32_t vkMinorVersion);
			static Ast::SanitizeVisitor::Options GetSanitizeOptions();

		private:
			struct FunctionParameter;
			struct OnlyCache {};

			std::uint32_t AllocateResultId();

			void AppendHeader();

			SpirvConstantCache::TypePtr BuildFunctionType(const Ast::DeclareFunctionStatement& functionNode);

			std::uint32_t GetArrayConstantId(const Ast::ConstantArrayValue& values) const;
			std::uint32_t GetSingleConstantId(const Ast::ConstantSingleValue& value) const;
			std::uint32_t GetExtendedInstructionSet(const std::string& instructionSetName) const;
			const SpirvVariable& GetExtVar(std::size_t varIndex) const;
			std::uint32_t GetFunctionTypeId(const Ast::DeclareFunctionStatement& functionNode);
			std::uint32_t GetPointerTypeId(const Ast::ExpressionType& type, SpirvStorageClass storageClass) const;
			std::uint32_t GetSourceFileId(const std::shared_ptr<const std::string>& filepathPtr);
			std::uint32_t GetTypeId(const Ast::ExpressionType& type) const;

			bool HasDebugInfo(DebugLevel debugInfo) const;

			std::uint32_t RegisterArrayConstant(const Ast::ConstantArrayValue& value);
			std::uint32_t RegisterFunctionType(const Ast::DeclareFunctionStatement& functionNode);
			std::uint32_t RegisterPointerType(Ast::ExpressionType type, SpirvStorageClass storageClass);
			std::uint32_t RegisterSingleConstant(const Ast::ConstantSingleValue& value);
			std::uint32_t RegisterType(Ast::ExpressionType type);

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
