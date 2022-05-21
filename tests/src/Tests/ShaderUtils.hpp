#pragma once

#ifndef NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP
#define NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP

#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <string>

void ExpectGLSL(const nzsl::ShaderAst::Module& shader, std::string_view expectedOutput);
void ExpectNZSL(const nzsl::ShaderAst::Module& shader, std::string_view expectedOutput);
void ExpectSPIRV(const nzsl::ShaderAst::Module& shader, std::string_view expectedOutput, bool outputParameter = false);

nzsl::ShaderAst::ModulePtr SanitizeModule(const nzsl::ShaderAst::Module& module);
nzsl::ShaderAst::ModulePtr SanitizeModule(const nzsl::ShaderAst::Module& module, const nzsl::ShaderAst::SanitizeVisitor::Options& options);

#endif
