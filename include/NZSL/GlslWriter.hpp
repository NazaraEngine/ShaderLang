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
	class NZSL_API GlslWriter : public ShaderWriter, public Ast::AstExpressionVisitorExcept, public Ast::AstStatementVisitorExcept
	{
		public:
			using BindingMapping = std::unordered_map<std::uint64_t /* set | binding */, unsigned /*glBinding*/>;
			struct Environment;
			using ExtSupportCallback = std::function<bool(const std::string_view& name)>;

			inline GlslWriter();
			GlslWriter(const GlslWriter&) = delete;
			GlslWriter(GlslWriter&&) = delete;
			~GlslWriter() = default;

			inline std::string Generate(const Ast::Module& module, const BindingMapping& bindingMapping = {}, const States& states = {});
			std::string Generate(std::optional<ShaderStageType> shaderStage, const Ast::Module& module, const BindingMapping& bindingMapping = {}, const States& states = {});

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
			static Ast::SanitizeVisitor::Options GetSanitizeOptions();

		private:
			void Append(const Ast::AliasType& aliasType);
			void Append(const Ast::ArrayType& type);
			void Append(Ast::BuiltinEntry builtin);
			void Append(const Ast::ExpressionType& type);
			void Append(const Ast::ExpressionValue<Ast::ExpressionType>& type);
			void Append(const Ast::FunctionType& functionType);
			void Append(const Ast::IntrinsicFunctionType& intrinsicFunctionType);
			void Append(const Ast::MatrixType& matrixType);
			void Append(const Ast::MethodType& methodType);
			void Append(Ast::MemoryLayout layout);
			void Append(Ast::NoType);
			void Append(Ast::PrimitiveType type);
			void Append(const Ast::SamplerType& samplerType);
			void Append(const Ast::StructType& structType);
			void Append(const Ast::Type& type);
			void Append(const Ast::UniformType& uniformType);
			void Append(const Ast::VectorType& vecType);
			template<typename T> void Append(const T& param);
			template<typename T1, typename T2, typename... Args> void Append(const T1& firstParam, const T2& secondParam, Args&&... params);
			void AppendComment(const std::string& section);
			void AppendCommentSection(const std::string& section);
			void AppendFunctionDeclaration(const Ast::DeclareFunctionStatement& node, const std::string& nameOverride, bool forward = false);
			void AppendHeader();
			void AppendLine(const std::string& txt = {});
			template<typename... Args> void AppendLine(Args&&... params);
			void AppendStatementList(std::vector<Ast::StatementPtr>& statements);
			void AppendVariableDeclaration(const Ast::ExpressionType& varType, const std::string& varName);

			void EnterScope();
			void LeaveScope(bool skipLine = true);

			void HandleEntryPoint(Ast::DeclareFunctionStatement& node);
			void HandleInOut();

			void RegisterStruct(std::size_t structIndex, Ast::StructDescription* desc, std::string structName);
			void RegisterVariable(std::size_t varIndex, std::string varName);

			void ScopeVisit(Ast::Statement& node);

			void Visit(Ast::ExpressionPtr& expr, bool encloseIfRequired = false);

			void Visit(Ast::AccessIdentifierExpression& node) override;
			void Visit(Ast::AccessIndexExpression& node) override;
			void Visit(Ast::AliasValueExpression& node) override;
			void Visit(Ast::AssignExpression& node) override;
			void Visit(Ast::BinaryExpression& node) override;
			void Visit(Ast::CallFunctionExpression& node) override;
			void Visit(Ast::CastExpression& node) override;
			void Visit(Ast::ConstantValueExpression& node) override;
			void Visit(Ast::FunctionExpression& node) override;
			void Visit(Ast::IntrinsicExpression& node) override;
			void Visit(Ast::SwizzleExpression& node) override;
			void Visit(Ast::VariableValueExpression& node) override;
			void Visit(Ast::UnaryExpression& node) override;

			void Visit(Ast::BranchStatement& node) override;
			void Visit(Ast::DeclareAliasStatement& node) override;
			void Visit(Ast::DeclareConstStatement& node) override;
			void Visit(Ast::DeclareExternalStatement& node) override;
			void Visit(Ast::DeclareFunctionStatement& node) override;
			void Visit(Ast::DeclareOptionStatement& node) override;
			void Visit(Ast::DeclareStructStatement& node) override;
			void Visit(Ast::DeclareVariableStatement& node) override;
			void Visit(Ast::DiscardStatement& node) override;
			void Visit(Ast::ExpressionStatement& node) override;
			void Visit(Ast::ImportStatement& node) override;
			void Visit(Ast::MultiStatement& node) override;
			void Visit(Ast::NoOpStatement& node) override;
			void Visit(Ast::ReturnStatement& node) override;
			void Visit(Ast::ScopedStatement& node) override;
			void Visit(Ast::WhileStatement& node) override;

			static bool HasExplicitBinding(Ast::StatementPtr& shader);
			static bool HasExplicitLocation(Ast::StatementPtr& shader);

			struct State;

			Environment m_environment;
			State* m_currentState;
	};
}

#include <NZSL/GlslWriter.inl>

#endif // NZSL_GLSLWRITER_HPP
