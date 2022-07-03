// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_REFLECT_HPP
#define NZSL_AST_REFLECT_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Lang/SourceLocation.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <vector>

namespace nzsl::Ast
{
	class NZSL_API ReflectVisitor : public RecursiveVisitor
	{
		public:
			struct Callbacks;

			ReflectVisitor() = default;
			ReflectVisitor(const ReflectVisitor&) = delete;
			ReflectVisitor(ReflectVisitor&&) = delete;
			~ReflectVisitor() = default;

			void Reflect(const Module& shaderModule, const Callbacks& callbacks);
			void Reflect(Statement& statement, const Callbacks& callbacks);

			ReflectVisitor& operator=(const ReflectVisitor&) = delete;
			ReflectVisitor& operator=(ReflectVisitor&&) = delete;

			struct Callbacks
			{
				std::function<void(ShaderStageType stageType, const std::string& functionName)> onEntryPointDeclaration;

				std::function<void(const DeclareAliasStatement&    aliasDecl)>    onAliasDeclaration;
				std::function<void(const DeclareConstStatement&    constDecl)>    onConstDeclaration;
				std::function<void(const DeclareExternalStatement& extDecl)>      onExternalDeclaration;
				std::function<void(const DeclareFunctionStatement& funcDecl)>     onFunctionDeclaration;
				std::function<void(const DeclareOptionStatement&   optionDecl)>   onOptionDeclaration;
				std::function<void(const DeclareStructStatement&   structDecl)>   onStructDeclaration;
				std::function<void(const DeclareVariableStatement& variableDecl)> onVariableDeclaration;

				std::function<void(const std::string& name, std::size_t aliasIndex,  const SourceLocation& sourceLocation)> onAliasIndex;
				std::function<void(const std::string& name, std::size_t constIndex,  const SourceLocation& sourceLocation)> onConstIndex;
				std::function<void(const std::string& name, std::size_t funcIndex,   const SourceLocation& sourceLocation)> onFunctionIndex;
				std::function<void(const std::string& name, std::size_t optIndex,    const SourceLocation& sourceLocation)> onOptionIndex;
				std::function<void(const std::string& name, std::size_t structIndex, const SourceLocation& sourceLocation)> onStructIndex;
				std::function<void(const std::string& name, std::size_t varIndex,    const SourceLocation& sourceLocation)> onVariableIndex;
			};

		private:
			void Visit(DeclareAliasStatement& node) override;
			void Visit(DeclareConstStatement& node) override;
			void Visit(DeclareExternalStatement& node) override;
			void Visit(DeclareFunctionStatement& node) override;
			void Visit(DeclareOptionStatement& node) override;
			void Visit(DeclareStructStatement& node) override;
			void Visit(DeclareVariableStatement& node) override;
			void Visit(ForStatement& node) override;
			void Visit(ForEachStatement& node) override;

			const Callbacks* m_callbacks;
	};
}

#include <NZSL/Ast/ReflectVisitor.inl>

#endif // NZSL_AST_REFLECT_HPP
