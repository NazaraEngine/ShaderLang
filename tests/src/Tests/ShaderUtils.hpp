#pragma once

#ifndef NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP
#define NAZARA_UNITTESTS_SHADER_SHADERUTILS_HPP

#include <NZSL/GlslWriter.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/Transformations/BindingResolverTransformer.hpp>
#include <NZSL/Ast/Transformations/ConstantRemovalTransformer.hpp>
#include <NZSL/Ast/Transformations/ResolveTransformer.hpp>
#include <NZSL/Ast/Transformations/LiteralTransformer.hpp>
#include <spirv-tools/libspirv.hpp>
#include <filesystem>

struct ResolveOptions
{
	static const nzsl::Ast::BindingResolverTransformer::Options defaultBindingResolverOptions;
	static const nzsl::Ast::ResolveTransformer::Options defaultIdentifierResolveOptions;
	static const nzsl::Ast::LiteralTransformer::Options defaultLiteralOptions;

	std::unordered_map<nzsl::Ast::OptionHash, nzsl::Ast::ConstantValue> optionValues;
	bool partialCompilation = false;
	const nzsl::Ast::BindingResolverTransformer::Options* bindingResolverOptions = &defaultBindingResolverOptions;
	const nzsl::Ast::ConstantRemovalTransformer::Options* constantRemovalOptions = nullptr;
	const nzsl::Ast::ResolveTransformer::Options* identifierResolverOptions = &defaultIdentifierResolveOptions;
	const nzsl::Ast::LiteralTransformer::Options* literalOptions = nullptr;
};

void ExpectGLSL(nzsl::ShaderStageType stageType, nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::BackendParameters& options = {}, const nzsl::GlslWriter::Environment& env = {}, bool testShaderCompilation = true);
void ExpectGLSL(nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::BackendParameters& options = {}, const nzsl::GlslWriter::Environment& env = {}, bool testShaderCompilation = true);
void ExpectNZSL(const nzsl::Ast::Module& shader, std::string_view expectedOutput);
void ExpectSPIRV(nzsl::Ast::Module& shader, std::string_view expectedOutput, const nzsl::BackendParameters& options = {}, const nzsl::SpirvWriter::Environment& env = {}, bool outputParameter = false, const spvtools::ValidatorOptions& validatorOptions = {});
void ExpectWGSL(const nzsl::Ast::Module& shader, std::string_view expectedOutput);

std::filesystem::path GetResourceDir();

void ResolveModule(nzsl::Ast::Module& module, const ResolveOptions& = {});

#endif
