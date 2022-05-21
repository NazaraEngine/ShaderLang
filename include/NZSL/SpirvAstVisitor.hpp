// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRVASTVISITOR_HPP
#define NZSL_SPIRVASTVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/SpirvBlock.hpp>
#include <NZSL/Ast/AstExpressionVisitorExcept.hpp>
#include <NZSL/Ast/AstStatementVisitorExcept.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <unordered_map>
#include <vector>

namespace nzsl
{
	class SpirvWriter;

	class NZSL_API SpirvAstVisitor : public ShaderAst::AstExpressionVisitorExcept, public ShaderAst::AstStatementVisitorExcept
	{
		public:
			struct EntryPoint;
			struct FuncData;
			struct Variable;

			inline SpirvAstVisitor(SpirvWriter& writer, SpirvSection& instructions, std::unordered_map<std::size_t, FuncData>& funcData);
			SpirvAstVisitor(const SpirvAstVisitor&) = delete;
			SpirvAstVisitor(SpirvAstVisitor&&) = delete;
			~SpirvAstVisitor() = default;

			std::uint32_t AllocateResultId();

			std::uint32_t EvaluateExpression(ShaderAst::ExpressionPtr& expr);

			const Variable& GetVariable(std::size_t varIndex) const;

			using AstExpressionVisitorExcept::Visit;
			using AstStatementVisitorExcept::Visit;

			void Visit(ShaderAst::AccessIndexExpression& node) override;
			void Visit(ShaderAst::AssignExpression& node) override;
			void Visit(ShaderAst::BinaryExpression& node) override;
			void Visit(ShaderAst::BranchStatement& node) override;
			void Visit(ShaderAst::CallFunctionExpression& node) override;
			void Visit(ShaderAst::CastExpression& node) override;
			void Visit(ShaderAst::ConstantValueExpression& node) override;
			void Visit(ShaderAst::DeclareExternalStatement& node) override;
			void Visit(ShaderAst::DeclareFunctionStatement& node) override;
			void Visit(ShaderAst::DeclareOptionStatement& node) override;
			void Visit(ShaderAst::DeclareStructStatement& node) override;
			void Visit(ShaderAst::DeclareVariableStatement& node) override;
			void Visit(ShaderAst::DiscardStatement& node) override;
			void Visit(ShaderAst::ExpressionStatement& node) override;
			void Visit(ShaderAst::IntrinsicExpression& node) override;
			void Visit(ShaderAst::MultiStatement& node) override;
			void Visit(ShaderAst::NoOpStatement& node) override;
			void Visit(ShaderAst::ReturnStatement& node) override;
			void Visit(ShaderAst::ScopedStatement& node) override;
			void Visit(ShaderAst::SwizzleExpression& node) override;
			void Visit(ShaderAst::UnaryExpression& node) override;
			void Visit(ShaderAst::VariableValueExpression& node) override;
			void Visit(ShaderAst::WhileStatement& node) override;

			SpirvAstVisitor& operator=(const SpirvAstVisitor&) = delete;
			SpirvAstVisitor& operator=(SpirvAstVisitor&&) = delete;

			struct EntryPoint
			{
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
				};

				ShaderStageType stageType;
				std::optional<InputStruct> inputStruct;
				std::optional<std::uint32_t> outputStructTypeId;
				std::vector<Input> inputs;
				std::vector<Output> outputs;
				std::vector<SpirvExecutionMode> executionModes;
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
				};

				struct Variable
				{
					std::uint32_t typeId;
					std::uint32_t varId;
				};

				std::size_t funcIndex;
				std::string name;
				std::vector<FuncCall> funcCalls;
				std::vector<Parameter> parameters;
				std::vector<Variable> variables;
				std::unordered_map<std::size_t, std::size_t> varIndexToVarId;
				std::uint32_t funcId;
				std::uint32_t funcTypeId;
				std::uint32_t returnTypeId;
			};

			struct Variable
			{
				SpirvStorageClass storage;
				std::uint32_t pointerId;
				std::uint32_t pointedTypeId;
			};

		private:
			void HandleStatementList(const std::vector<ShaderAst::StatementPtr>& statements);

			void PushResultId(std::uint32_t value);
			std::uint32_t PopResultId();

			inline void RegisterExternalVariable(std::size_t varIndex, const ShaderAst::ExpressionType& type);
			inline void RegisterStruct(std::size_t structIndex, ShaderAst::StructDescription* structDesc);
			inline void RegisterVariable(std::size_t varIndex, std::uint32_t typeId, std::uint32_t pointerId, SpirvStorageClass storageClass);

			std::size_t m_extVarIndex;
			std::size_t m_funcCallIndex;
			std::size_t m_funcIndex;
			std::unordered_map<std::size_t, FuncData>& m_funcData;
			std::unordered_map<std::size_t, ShaderAst::StructDescription*> m_structs;
			std::unordered_map<std::size_t, Variable> m_variables;
			std::vector<std::size_t> m_scopeSizes;
			std::vector<std::unique_ptr<SpirvBlock>> m_functionBlocks;
			std::vector<std::uint32_t> m_resultIds;
			SpirvBlock* m_currentBlock;
			SpirvSection& m_instructions;
			SpirvWriter& m_writer;
	};
}

#include <NZSL/SpirvAstVisitor.inl>

#endif // NZSL_SPIRVASTVISITOR_HPP
