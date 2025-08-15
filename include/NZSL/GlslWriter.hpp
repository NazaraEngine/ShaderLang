// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_GLSLWRITER_HPP
#define NZSL_GLSLWRITER_HPP

#include <NZSL/BackendParameters.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionVisitorExcept.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/StatementVisitorExcept.hpp>
#include <NZSL/Ast/TransformerExecutor.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace nzsl
{
	class NZSL_API GlslWriter : Ast::ExpressionVisitorExcept, Ast::StatementVisitorExcept
	{
		public:
			using ExtSupportCallback = std::function<bool(std::string_view name)>;
			struct Environment;
			struct Output;
			struct Parameters;

			inline GlslWriter();
			GlslWriter(const GlslWriter&) = delete;
			GlslWriter(GlslWriter&&) = delete;
			~GlslWriter() = default;

			inline Output Generate(Ast::Module& module, const BackendParameters& parameters = {}, const Parameters& glslParameters = {});
			Output Generate(std::optional<ShaderStageType> shaderStage, Ast::Module& module, const BackendParameters& parameters = {}, const Parameters& glslParameters = {});

			void SetEnv(Environment environment);

			struct Environment
			{
				ExtSupportCallback extCallback;
				unsigned int glMajorVersion = 3;
				unsigned int glMinorVersion = 0;
				bool glES = true;
				bool flipYPosition = false;
				bool remapZPosition = false;
				bool allowDrawParametersUniformsFallback = false;
			};

			struct Parameters
			{
				std::optional<unsigned int> pushConstantBinding;
				std::unordered_map<std::uint64_t /* set | binding */, unsigned int /*glBinding*/> bindingMapping;
			};

			struct Output
			{
				std::string code;
				std::unordered_map<std::string, unsigned int> explicitTextureBinding;
				std::unordered_map<std::string, unsigned int> explicitUniformBlockBinding;
				bool usesDrawParameterBaseInstanceUniform;
				bool usesDrawParameterBaseVertexUniform;
				bool usesDrawParameterDrawIndexUniform;
			};

			static std::string_view GetDrawParameterBaseInstanceUniformName();
			static std::string_view GetDrawParameterBaseVertexUniformName();
			static std::string_view GetDrawParameterDrawIndexUniformName();
			static std::string_view GetFlipYUniformName();
			static void RegisterPasses(Ast::TransformerExecutor& executor);

		private:
			void Append(const Ast::AliasType& aliasType);
			void Append(const Ast::ArrayType& type);
			void Append(Ast::BuiltinEntry builtin);
			void Append(const Ast::DynArrayType& type);
			void Append(const Ast::ExpressionType& type);
			void Append(const Ast::ExpressionValue<Ast::ExpressionType>& type);
			void Append(const Ast::FunctionType& functionType);
			void Append(const Ast::ImplicitVectorType& type);
			void Append(Ast::InterpolationQualifier interpolation);
			void Append(const Ast::IntrinsicFunctionType& intrinsicFunctionType);
			void Append(const Ast::MatrixType& matrixType);
			void Append(const Ast::MethodType& methodType);
			void Append(const Ast::ModuleType& methodType);
			void Append(Ast::MemoryLayout layout);
			void Append(const Ast::NamedExternalBlockType& namedExternalBlockType);
			void Append(Ast::NoType);
			void Append(Ast::PrimitiveType type);
			void Append(const Ast::PushConstantType& pushConstantType);
			void Append(const Ast::SamplerType& samplerType);
			void Append(const Ast::StorageType& storageType);
			void Append(const Ast::StructType& structType);
			void Append(const Ast::TextureType& textureType);
			void Append(const Ast::Type& type);
			void Append(const Ast::UniformType& uniformType);
			void Append(const Ast::VectorType& vecType);
			template<typename T> void Append(const T& param);
			template<typename T1, typename T2, typename... Args> void Append(const T1& firstParam, const T2& secondParam, Args&&... params);
			template<typename... Args> void Append(const std::variant<Args...>& param);
			void AppendArray(const Ast::ExpressionType& varType, const std::string& varName = {});
			void AppendComment(std::string_view section);
			void AppendCommentSection(std::string_view section);
			void AppendFunctionDeclaration(const Ast::DeclareFunctionStatement& node, const std::string& nameOverride, bool forward = false);
			void AppendHeader();
			void AppendLine(std::string_view txt = {});
			template<typename... Args> void AppendLine(Args&&... params);
			void AppendModuleComments(const Ast::Module& module);
			void AppendStatementList(std::vector<Ast::StatementPtr>& statements);
			template<typename T> void AppendValue(const T& value);
			void AppendVariableDeclaration(const Ast::ExpressionType& varType, const std::string& varName);

			void EnterScope();
			void LeaveScope(bool skipLine = true);

			void HandleEntryPoint(Ast::DeclareFunctionStatement& node);
			void HandleInOut();
			void HandleSourceLocation(const SourceLocation& sourceLocation, DebugLevel requiredLevel);

			void RegisterConstant(std::size_t constIndex, std::string constName);
			void RegisterStruct(std::size_t structIndex, Ast::StructDescription* desc, std::string structName);
			void RegisterVariable(std::size_t varIndex, std::string varName);

			void ScopeVisit(Ast::Statement& node);

			void Visit(Ast::ExpressionPtr& expr, bool encloseIfRequired = false);

			using ExpressionVisitorExcept::Visit;
			void Visit(Ast::AccessFieldExpression& node) override;
			void Visit(Ast::AccessIdentifierExpression& node) override;
			void Visit(Ast::AccessIndexExpression& node) override;
			void Visit(Ast::AliasValueExpression& node) override;
			void Visit(Ast::AssignExpression& node) override;
			void Visit(Ast::BinaryExpression& node) override;
			void Visit(Ast::CallFunctionExpression& node) override;
			void Visit(Ast::CastExpression& node) override;
			void Visit(Ast::ConstantExpression& node) override;
			void Visit(Ast::ConstantArrayValueExpression& node) override;
			void Visit(Ast::ConstantValueExpression& node) override;
			void Visit(Ast::FunctionExpression& node) override;
			void Visit(Ast::IntrinsicExpression& node) override;
			void Visit(Ast::SwizzleExpression& node) override;
			void Visit(Ast::VariableValueExpression& node) override;
			void Visit(Ast::UnaryExpression& node) override;

			using StatementVisitorExcept::Visit;
			void Visit(Ast::BranchStatement& node) override;
			void Visit(Ast::BreakStatement& node) override;
			void Visit(Ast::ContinueStatement& node) override;
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

			struct State;

			Environment m_environment;
			State* m_currentState;
	};
}

#include <NZSL/GlslWriter.inl>

#endif // NZSL_GLSLWRITER_HPP
