#pragma once

#ifndef NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP
#define NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP

#include <NZSL/GlslWriter.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <filesystem>
#include <string>

void ExpectGLSL(const nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::ShaderWriter::States& options = {}, const nzsl::GlslWriter::Environment& env = {}, bool testShaderCompilation = true);
void ExpectNZSL(const nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::ShaderWriter::States& options = {});
void ExpectSPIRV(const nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::ShaderWriter::States& options = {}, const nzsl::SpirvWriter::Environment& env = {}, bool outputParameter = false);

std::filesystem::path GetResourceDir();

nzsl::Ast::ModulePtr SanitizeModule(const nzsl::Ast::Module& module);
nzsl::Ast::ModulePtr SanitizeModule(const nzsl::Ast::Module& module, const nzsl::Ast::SanitizeVisitor::Options& options);

#endif
