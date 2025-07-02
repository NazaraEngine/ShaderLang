#pragma once

#ifndef NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP
#define NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP

#include <NZSL/GlslWriter.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/Transformations/IdentifierTypeResolverTransformer.hpp>
#include <NZSL/Ast/Transformations/ImportResolverTransformer.hpp>
#include <spirv-tools/libspirv.hpp>
#include <filesystem>

void ExpectGLSL(nzsl::ShaderStageType stageType, nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::ShaderWriter::States& options = {}, const nzsl::GlslWriter::Environment& env = {}, bool testShaderCompilation = true);
void ExpectGLSL(nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::ShaderWriter::States& options = {}, const nzsl::GlslWriter::Environment& env = {}, bool testShaderCompilation = true);
void ExpectNZSL(nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::ShaderWriter::States& options = {});
void ExpectSPIRV(nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::ShaderWriter::States& options = {}, const nzsl::SpirvWriter::Environment& env = {}, bool outputParameter = false, const spvtools::ValidatorOptions& validatorOptions = {});

std::filesystem::path GetResourceDir();

void ResolveModule(nzsl::Ast::Module& module);
void ResolveModule(nzsl::Ast::Module& module, const nzsl::Ast::IdentifierTypeResolverTransformer::Options& resolverOptions, const nzsl::Ast::ImportResolverTransformer::Options* importOptions = nullptr);
void ResolveModule(nzsl::Ast::Module& module, nzsl::Ast::Transformer::Context& context, const nzsl::Ast::IdentifierTypeResolverTransformer::Options& resolverOptions = {}, const nzsl::Ast::ImportResolverTransformer::Options* importOptions = nullptr);

#endif
