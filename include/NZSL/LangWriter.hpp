// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LANGWRITER_HPP
#define NZSL_LANGWRITER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/ShaderWriter.hpp>
#include <NZSL/Ast/ExpressionVisitorExcept.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/StatementVisitorExcept.hpp>
#include <string>

namespace nzsl
{
	class NZSL_API LangWriter : public ShaderWriter, public Ast::ExpressionVisitorExcept, public Ast::StatementVisitorExcept
	{
		public:
			struct Environment;

			inline LangWriter();
			LangWriter(const LangWriter&) = delete;
			LangWriter(LangWriter&&) = delete;
			~LangWriter() = default;

			std::string Generate(const Ast::Module& module, const States& states = {});

			void SetEnv(Environment environment);

			struct Environment
			{
			};

		private:
			struct PreVisitor;
			friend PreVisitor;

			// Attributes
			struct AutoBindingAttribute;
			struct AuthorAttribute;
			struct BindingAttribute;
			struct BuiltinAttribute;
			struct CondAttribute;
			struct DepthWriteAttribute;
			struct DescriptionAttribute;
			struct EarlyFragmentTestsAttribute;
			struct EntryAttribute;
			struct FeatureAttribute;
			struct InterpAttribute;
			struct LangVersionAttribute;
			struct LayoutAttribute;
			struct LicenseAttribute;
			struct LocationAttribute;
			struct SetAttribute;
			struct TagAttribute;
			struct UnrollAttribute;
			struct WorkgroupAttribute;

			void Append(const Ast::AliasType& type);
			void Append(const Ast::ArrayType& type);
			void Append(const Ast::DynArrayType& type);
			void Append(const Ast::ExpressionType& type);
			void Append(const Ast::ExpressionValue<Ast::ExpressionType>& type);
			void Append(const Ast::FunctionType& functionType);
			void Append(const Ast::IntrinsicFunctionType& intrinsicFunctionType);
			void Append(const Ast::MatrixType& matrixType);
			void Append(const Ast::MethodType& methodType);
			void Append(const Ast::ModuleType& moduleType);
			void Append(const Ast::NamedExternalBlockType& namedExternalBlockType);
			void Append(Ast::NoType);
			void Append(Ast::PrimitiveType type);
			void Append(const Ast::PushConstantType& pushConstantType);
			void Append(const Ast::SamplerType& samplerType);
			void Append(const Ast::StorageType& storageType);
			void Append(const Ast::StructType& structType);
			void Append(const Ast::TextureType& samplerType);
			void Append(const Ast::Type& type);
			void Append(const Ast::UniformType& uniformType);
			void Append(const Ast::VectorType& vecType);
			template<typename T> void Append(const T& param);
			template<typename T1, typename T2, typename... Args> void Append(const T1& firstParam, const T2& secondParam, Args&&... params);
			template<typename... Args> void AppendAttributes(bool appendLine, Args&&... params);
			template<typename T> void AppendAttributesInternal(bool& first, const T& param);
			template<typename T1, typename T2, typename... Rest> void AppendAttributesInternal(bool& first, const T1& firstParam, const T2& secondParam, Rest&&... params);
			void AppendAttribute(AutoBindingAttribute attribute);
			void AppendAttribute(AuthorAttribute attribute);
			void AppendAttribute(BindingAttribute attribute);
			void AppendAttribute(BuiltinAttribute attribute);
			void AppendAttribute(CondAttribute attribute);
			void AppendAttribute(DepthWriteAttribute attribute);
			void AppendAttribute(DescriptionAttribute attribute);
			void AppendAttribute(EarlyFragmentTestsAttribute attribute);
			void AppendAttribute(EntryAttribute attribute);
			void AppendAttribute(FeatureAttribute attribute);
			void AppendAttribute(InterpAttribute attribute);
			void AppendAttribute(LangVersionAttribute attribute);
			void AppendAttribute(LayoutAttribute attribute);
			void AppendAttribute(LicenseAttribute attribute);
			void AppendAttribute(LocationAttribute attribute);
			void AppendAttribute(SetAttribute attribute);
			void AppendAttribute(TagAttribute attribute);
			void AppendAttribute(UnrollAttribute attribute);
			void AppendAttribute(WorkgroupAttribute attribute);
			void AppendComment(std::string_view section);
			void AppendCommentSection(std::string_view section);
			void AppendHeader();
			template<typename T> void AppendIdentifier(const T& map, std::size_t id);
			void AppendLine(std::string_view txt = {});
			template<typename... Args> void AppendLine(Args&&... params);
			void AppendModuleAttributes(const Ast::Module::Metadata& metadata);
			void AppendStatementList(std::vector<Ast::StatementPtr>& statements);
			template<typename T> void AppendValue(const T& value);

			void EnterScope();
			void LeaveScope(bool skipLine = true);

			void RegisterAlias(std::size_t aliasIndex, std::string aliasName);
			void RegisterConstant(std::size_t constantIndex, std::string constantName);
			void RegisterFunction(std::size_t funcIndex, std::string functionName);
			void RegisterModule(std::size_t moduleIndex, std::string moduleName);
			void RegisterStruct(std::size_t structIndex, std::string structName);
			void RegisterVariable(std::size_t varIndex, std::string varName);

			void ScopeVisit(Ast::Statement& node);

			void Visit(Ast::ExpressionPtr& expr, bool encloseIfRequired = false);

			using ExpressionVisitorExcept::Visit;
			void Visit(Ast::AccessIdentifierExpression& node) override;
			void Visit(Ast::AccessIndexExpression& node) override;
			void Visit(Ast::AliasValueExpression& node) override;
			void Visit(Ast::AssignExpression& node) override;
			void Visit(Ast::BinaryExpression& node) override;
			void Visit(Ast::CallFunctionExpression& node) override;
			void Visit(Ast::CastExpression& node) override;
			void Visit(Ast::ConditionalExpression& node) override;
			void Visit(Ast::ConstantArrayValueExpression& node) override;
			void Visit(Ast::ConstantValueExpression& node) override;
			void Visit(Ast::ConstantExpression& node) override;
			void Visit(Ast::FunctionExpression& node) override;
			void Visit(Ast::IdentifierExpression& node) override;
			void Visit(Ast::IntrinsicExpression& node) override;
			void Visit(Ast::ModuleExpression& node) override;
			void Visit(Ast::NamedExternalBlockExpression& node) override;
			void Visit(Ast::StructTypeExpression& node) override;
			void Visit(Ast::SwizzleExpression& node) override;
			void Visit(Ast::VariableValueExpression& node) override;
			void Visit(Ast::UnaryExpression& node) override;

			using StatementVisitorExcept::Visit;
			void Visit(Ast::BranchStatement& node) override;
			void Visit(Ast::BreakStatement& node) override;
			void Visit(Ast::ConditionalStatement& node) override;
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
			void Visit(Ast::ForStatement& node) override;
			void Visit(Ast::ForEachStatement& node) override;
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

#include <NZSL/LangWriter.inl>

#endif // NZSL_LANGWRITER_HPP
