// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_GLSLWRITER_HPP
#define NZSL_GLSLWRITER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/ShaderWriter.hpp>
#include <NZSL/Ast/AstExpressionVisitorExcept.hpp>
#include <NZSL/Ast/AstStatementVisitorExcept.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace nzsl
{
	class NZSL_API GlslWriter : public ShaderWriter, public ShaderAst::AstExpressionVisitorExcept, public ShaderAst::AstStatementVisitorExcept
	{
		public:
			using BindingMapping = std::unordered_map<std::uint64_t /* set | binding */, unsigned /*glBinding*/>;
			struct Environment;
			using ExtSupportCallback = std::function<bool(const std::string_view& name)>;

			inline GlslWriter();
			GlslWriter(const GlslWriter&) = delete;
			GlslWriter(GlslWriter&&) = delete;
			~GlslWriter() = default;

			inline std::string Generate(const ShaderAst::Module& module, const BindingMapping& bindingMapping = {}, const States& states = {});
			std::string Generate(std::optional<ShaderStageType> shaderStage, const ShaderAst::Module& module, const BindingMapping& bindingMapping = {}, const States& states = {});

			void SetEnv(Environment environment);

			struct Environment
			{
				ExtSupportCallback extCallback;
				unsigned int glMajorVersion = 3;
				unsigned int glMinorVersion = 0;
				bool glES = true;
				bool flipYPosition = false;
				bool remapZPosition = false;
			};

			static const char* GetFlipYUniformName();
			static ShaderAst::SanitizeVisitor::Options GetSanitizeOptions();

		private:
			void Append(const ShaderAst::AliasType& aliasType);
			void Append(const ShaderAst::ArrayType& type);
			void Append(ShaderAst::BuiltinEntry builtin);
			void Append(const ShaderAst::ExpressionType& type);
			void Append(const ShaderAst::ExpressionValue<ShaderAst::ExpressionType>& type);
			void Append(const ShaderAst::FunctionType& functionType);
			void Append(const ShaderAst::IntrinsicFunctionType& intrinsicFunctionType);
			void Append(const ShaderAst::MatrixType& matrixType);
			void Append(const ShaderAst::MethodType& methodType);
			void Append(ShaderAst::MemoryLayout layout);
			void Append(ShaderAst::NoType);
			void Append(ShaderAst::PrimitiveType type);
			void Append(const ShaderAst::SamplerType& samplerType);
			void Append(const ShaderAst::StructType& structType);
			void Append(const ShaderAst::Type& type);
			void Append(const ShaderAst::UniformType& uniformType);
			void Append(const ShaderAst::VectorType& vecType);
			template<typename T> void Append(const T& param);
			template<typename T1, typename T2, typename... Args> void Append(const T1& firstParam, const T2& secondParam, Args&&... params);
			void AppendComment(const std::string& section);
			void AppendCommentSection(const std::string& section);
			void AppendFunctionDeclaration(const ShaderAst::DeclareFunctionStatement& node, const std::string& nameOverride, bool forward = false);
			void AppendHeader();
			void AppendLine(const std::string& txt = {});
			template<typename... Args> void AppendLine(Args&&... params);
			void AppendStatementList(std::vector<ShaderAst::StatementPtr>& statements);
			void AppendVariableDeclaration(const ShaderAst::ExpressionType& varType, const std::string& varName);

			void EnterScope();
			void LeaveScope(bool skipLine = true);

			void HandleEntryPoint(ShaderAst::DeclareFunctionStatement& node);
			void HandleInOut();

			void RegisterStruct(std::size_t structIndex, ShaderAst::StructDescription* desc, std::string structName);
			void RegisterVariable(std::size_t varIndex, std::string varName);

			void ScopeVisit(ShaderAst::Statement& node);

			void Visit(ShaderAst::ExpressionPtr& expr, bool encloseIfRequired = false);

			void Visit(ShaderAst::AccessIdentifierExpression& node) override;
			void Visit(ShaderAst::AccessIndexExpression& node) override;
			void Visit(ShaderAst::AliasValueExpression& node) override;
			void Visit(ShaderAst::AssignExpression& node) override;
			void Visit(ShaderAst::BinaryExpression& node) override;
			void Visit(ShaderAst::CallFunctionExpression& node) override;
			void Visit(ShaderAst::CastExpression& node) override;
			void Visit(ShaderAst::ConstantValueExpression& node) override;
			void Visit(ShaderAst::FunctionExpression& node) override;
			void Visit(ShaderAst::IntrinsicExpression& node) override;
			void Visit(ShaderAst::SwizzleExpression& node) override;
			void Visit(ShaderAst::VariableValueExpression& node) override;
			void Visit(ShaderAst::UnaryExpression& node) override;

			void Visit(ShaderAst::BranchStatement& node) override;
			void Visit(ShaderAst::DeclareAliasStatement& node) override;
			void Visit(ShaderAst::DeclareConstStatement& node) override;
			void Visit(ShaderAst::DeclareExternalStatement& node) override;
			void Visit(ShaderAst::DeclareFunctionStatement& node) override;
			void Visit(ShaderAst::DeclareOptionStatement& node) override;
			void Visit(ShaderAst::DeclareStructStatement& node) override;
			void Visit(ShaderAst::DeclareVariableStatement& node) override;
			void Visit(ShaderAst::DiscardStatement& node) override;
			void Visit(ShaderAst::ExpressionStatement& node) override;
			void Visit(ShaderAst::ImportStatement& node) override;
			void Visit(ShaderAst::MultiStatement& node) override;
			void Visit(ShaderAst::NoOpStatement& node) override;
			void Visit(ShaderAst::ReturnStatement& node) override;
			void Visit(ShaderAst::ScopedStatement& node) override;
			void Visit(ShaderAst::WhileStatement& node) override;

			static bool HasExplicitBinding(ShaderAst::StatementPtr& shader);
			static bool HasExplicitLocation(ShaderAst::StatementPtr& shader);

			struct State;

			Environment m_environment;
			State* m_currentState;
	};
}

#include <NZSL/GlslWriter.inl>

#endif // NZSL_GLSLWRITER_HPP
