// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSLC_COMPILER_HPP
#define NZSLC_COMPILER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Errors.hpp>
#include <NZSL/Ast/Module.hpp>
#include <cxxopts.hpp>
#include <filesystem>
#include <type_traits>
#include <vector>

namespace nzslc
{
	class Compiler
	{
		public:
			static constexpr std::uint32_t MajorVersion = 0;
			static constexpr std::uint32_t MinorVersion = 1;
			static constexpr std::uint32_t PatchVersion = 0;

			Compiler(cxxopts::ParseResult& options);
			Compiler(const Compiler&) = delete;
			Compiler(Compiler&&) = delete;
			~Compiler() = default;

			void HandleParameters();

			void PrintError(const nzsl::Error& error) const;

			void Process();

			Compiler& operator=(const Compiler&) = delete;
			Compiler& operator=(Compiler&&) = delete;

			static cxxopts::Options BuildOptions();

			enum class LogFormat
			{
				Classic,
				VisualStudio
			};

		private:
			void Compile();
			void CompileToGLSL(std::filesystem::path outputPath, const nzsl::Ast::Module& module);
			void CompileToNZSL(std::filesystem::path outputPath, const nzsl::Ast::Module& module);
			void CompileToNZSLB(std::filesystem::path outputPath, nzsl::Ast::ModulePtr& module);
			void CompileToSPV(std::filesystem::path outputPath, const nzsl::Ast::Module& module, bool textual);
			void PrintTime();
			void OutputFile(std::filesystem::path filePath, const void* data, std::size_t size);
			void OutputToStdout(std::string_view str);
			void ReadInput();
			void Sanitize();
			template<typename F, typename... Args> auto Step(std::enable_if_t<!std::is_member_function_pointer_v<F>, std::string_view> stepName, F&& func, Args&&... args) -> decltype(std::invoke(func, std::forward<Args>(args)...));
			template<typename F, typename... Args> auto Step(std::enable_if_t<std::is_member_function_pointer_v<F>, std::string_view> stepName, F&& func, Args&&... args) -> decltype(std::invoke(func, this, std::forward<Args>(args)...));
			template<typename F> auto StepInternal(std::string_view stepName, F&& func) -> decltype(func());
			nzsl::Ast::ModulePtr Unserialize(const std::uint8_t* data, std::size_t size);

			static nzsl::Ast::ModulePtr Parse(std::string_view sourceContent, const std::string& filePath);
			static std::vector<std::uint8_t> ReadFileContent(const std::filesystem::path& filePath);
			static std::string ReadSourceFileContent(const std::filesystem::path& filePath);
			static std::string ToHeader(const void* data, std::size_t size);
			static void WriteFileContent(const std::filesystem::path& filePath, const void* data, std::size_t size);

			struct StepTime
			{
				std::size_t childrenCount;
				std::string name;
				long long time;
			};

			std::filesystem::path m_inputFilePath;
			std::filesystem::path m_outputPath;
			std::vector<StepTime> m_steps;
			LogFormat m_logFormat;
			nzsl::Ast::ModulePtr m_shaderModule;
			cxxopts::ParseResult& m_options;
			bool m_profiling;
			bool m_outputHeader;
			bool m_outputToStdout;
			bool m_verbose;
			unsigned int m_iterationCount;
	};
}

#include <ShaderCompiler/Compiler.inl>

#endif // NZSLC_COMPILER_HPP
