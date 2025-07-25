// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRV_SPIRVASTVISITOR_HPP
#define NZSL_SPIRV_SPIRVASTVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/ExpressionVisitorExcept.hpp>
#include <NZSL/Ast/StatementVisitorExcept.hpp>
#include <NZSL/SpirV/SpirvBlock.hpp>
#include <NZSL/SpirV/SpirvConstantCache.hpp>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace nzsl
{
	class SpirvWriter;

	class NZSL_API SpirvAstVisitor : public Ast::ExpressionVisitorExcept, public Ast::StatementVisitorExcept
	{
		public:
			struct EntryPoint;
			struct FuncData;

			inline SpirvAstVisitor(SpirvWriter& writer, SpirvSection& instructions, std::function<FuncData&(std::size_t)> functionRetriever);
			SpirvAstVisitor(const SpirvAstVisitor&) = delete;
			SpirvAstVisitor(SpirvAstVisitor&&) = delete;
			~SpirvAstVisitor() = default;

			std::uint32_t AllocateResultId();

			std::uint32_t EvaluateExpression(Ast::Expression& expr);
			std::uint32_t EvaluatePointer(Ast::Expression& expr);

			const SpirvVariable& GetVariable(std::size_t varIndex) const;

			using ExpressionVisitorExcept::Visit;
			using StatementVisitorExcept::Visit;

			void Visit(Ast::AccessFieldExpression& node) override;
			void Visit(Ast::AccessIndexExpression& node) override;
			void Visit(Ast::AssignExpression& node) override;
			void Visit(Ast::BinaryExpression& node) override;
			void Visit(Ast::BranchStatement& node) override;
			void Visit(Ast::BreakStatement& node) override;
			void Visit(Ast::CallFunctionExpression& node) override;
			void Visit(Ast::CastExpression& node) override;
			void Visit(Ast::ConstantExpression& node) override;
			void Visit(Ast::ConstantValueExpression& node) override;
			void Visit(Ast::ContinueStatement& node) override;
			void Visit(Ast::DeclareConstStatement& node) override;
			void Visit(Ast::DeclareExternalStatement& node) override;
			void Visit(Ast::DeclareFunctionStatement& node) override;
			void Visit(Ast::DeclareOptionStatement& node) override;
			void Visit(Ast::DeclareStructStatement& node) override;
			void Visit(Ast::DeclareVariableStatement& node) override;
			void Visit(Ast::DiscardStatement& node) override;
			void Visit(Ast::ExpressionStatement& node) override;
			void Visit(Ast::IntrinsicExpression& node) override;
			void Visit(Ast::MultiStatement& node) override;
			void Visit(Ast::NoOpStatement& node) override;
			void Visit(Ast::ReturnStatement& node) override;
			void Visit(Ast::ScopedStatement& node) override;
			void Visit(Ast::SwizzleExpression& node) override;
			void Visit(Ast::UnaryExpression& node) override;
			void Visit(Ast::VariableValueExpression& node) override;
			void Visit(Ast::WhileStatement& node) override;

			SpirvAstVisitor& operator=(const SpirvAstVisitor&) = delete;
			SpirvAstVisitor& operator=(SpirvAstVisitor&&) = delete;

			// Should be private but are referenced in SpirvGenData.hpp
			void BuildArraySizeIntrinsic(const Ast::IntrinsicExpression& node);
			void BuildSelectIntrinsic(const Ast::IntrinsicExpression& node);
			static SpirvGlslStd450Op SelectAbs(const Ast::IntrinsicExpression& node);
			static SpirvGlslStd450Op SelectClamp(const Ast::IntrinsicExpression& node);
			static SpirvGlslStd450Op SelectLerp(const Ast::IntrinsicExpression& node);
			static SpirvGlslStd450Op SelectMaxMin(const Ast::IntrinsicExpression& node);
			static SpirvGlslStd450Op SelectSign(const Ast::IntrinsicExpression& node);

			struct EntryPoint
			{
				struct ExecutionMode
				{
					SpirvExecutionMode mode;
					std::vector<std::uint32_t> params;
				};

				struct Input
				{
					std::uint32_t memberIndexConstantId;
					std::uint32_t memberPointerId;
					std::uint32_t varId;
				};

				struct Output
				{
					std::int32_t memberIndex;
					std::uint32_t typeId;
					std::uint32_t varId;
				};

				struct InputStruct
				{
					std::uint32_t pointerId;
					std::uint32_t typeId;
					SpirvConstantCache::TypePtr type;
				};

				ShaderStageType stageType;
				std::optional<InputStruct> inputStruct;
				std::optional<std::uint32_t> outputStructTypeId;
				std::vector<ExecutionMode> executionModes;
				std::vector<Input> inputs;
				std::vector<Output> outputs;
			};

			struct FuncData
			{
				std::optional<EntryPoint> entryPointData;

				struct FuncCall
				{
					std::size_t firstVarIndex;
				};

				struct Parameter
				{
					std::uint32_t pointerTypeId;
					std::uint32_t typeId;
					SpirvConstantCache::TypePtr typePtr;
				};

				struct Variable
				{
					std::uint32_t typeId;
					std::uint32_t varId;
					SpirvConstantCache::TypePtr typePtr;
					SourceLocation sourceLocation;
				};

				std::size_t funcIndex;
				std::string name;
				std::unordered_map<std::size_t, std::size_t> varIndexToVarId;
				std::vector<FuncCall> funcCalls;
				std::vector<Parameter> parameters;
				std::vector<Variable> variables;
				std::uint32_t funcId;
				std::uint32_t funcTypeId;
				std::uint32_t returnTypeId;
			};

		private:
			void HandleSourceLocation(const SourceLocation& sourceLocation);
			void HandleStatementList(const std::vector<Ast::StatementPtr>& statements);

			void PushResultId(std::uint32_t value);
			std::uint32_t PopResultId();

			inline void RegisterVariable(std::size_t varIndex, SpirvConstantCache::TypePtr typePtr, std::uint32_t typeId, std::uint32_t pointerId, SpirvStorageClass storageClass);

			void ResetSourceLocation();

			std::function<FuncData& (std::size_t)> m_functionRetriever;
			std::optional<std::uint32_t> m_breakTarget;
			std::optional<std::uint32_t> m_continueTarget;
			std::size_t m_funcCallIndex;
			std::unordered_map<std::size_t, SpirvVariable> m_variables;
			std::vector<std::size_t> m_scopeSizes;
			std::vector<std::unique_ptr<SpirvBlock>> m_functionBlocks;
			std::vector<std::uint32_t> m_resultIds;
			FuncData* m_currentFunc;
			SpirvBlock* m_currentBlock;
			SpirvSection& m_instructions;
			SpirvWriter& m_writer;
			SourceLocation m_lastLocation;
			bool m_isEvaluatingPointer;
	};
}

#include <NZSL/SpirV/SpirvAstVisitor.inl>

#endif // NZSL_SPIRV_SPIRVASTVISITOR_HPP
