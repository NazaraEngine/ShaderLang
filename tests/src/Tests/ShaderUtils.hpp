#pragma once

#ifndef NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP
#define NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP

#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <string>

void ExpectGLSL(const nzsl::Ast::Module& shader, std::string_view expectedOutput);
void ExpectNZSL(const nzsl::Ast::Module& shader, std::string_view expectedOutput);
void ExpectSPIRV(const nzsl::Ast::Module& shader, std::string_view expectedOutput, bool outputParameter = false);

nzsl::Ast::ModulePtr SanitizeModule(const nzsl::Ast::Module& module);
nzsl::Ast::ModulePtr SanitizeModule(const nzsl::Ast::Module& module, const nzsl::Ast::SanitizeVisitor::Options& options);

#endif
