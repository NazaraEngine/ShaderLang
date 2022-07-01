// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <ShaderCompiler/Compiler.hpp>
#include <Nazara/Utils/CallOnExit.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/GlslWriter.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/Errors.hpp>
#include <NZSL/Lexer.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/SpirvPrinter.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <NZSL/Ast/ReflectVisitor.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <cassert>
#include <chrono>
#include <fstream>
#include <stdexcept>

namespace nzslc
{
	namespace
	{
		inline bool EndsWith(std::string_view str, std::string_view s)
		{
			if (s.size() > str.size())
				return false;

			return str.compare(str.size() - s.size(), s.size(), s.data()) == 0;
		}
	}
	
	Compiler::Compiler(cxxopts::ParseResult& options) :
	m_logFormat(LogFormat::Classic),
	m_options(options),
	m_profiling(false),
	m_outputToStdout(false),
	m_verbose(false),
	m_iterationCount(1)
	{
	}

	void Compiler::HandleParameters()
	{
		if (m_options.count("log-format") != 0)
		{
			const std::string& formatStr = m_options["log-format"].as<std::string>();
			if (formatStr == "vs")
				m_logFormat = LogFormat::VisualStudio;
			else if (formatStr != "classic")
				throw cxxopts::OptionException(fmt::format("{} is not a valid error format", formatStr));
		}

		if (m_options.count("input") == 0)
			throw cxxopts::OptionException("no input file");

		m_inputFilePath = m_options["input"].as<std::string>();
		if (!std::filesystem::is_regular_file(m_inputFilePath))
			throw std::runtime_error(fmt::format("{} is not a file", m_inputFilePath.generic_u8string()));

		if (m_options.count("output") > 0)
		{
			const std::string& outputPath = m_options["output"].as<std::string>();

			if (outputPath == "@stdout")
				m_outputToStdout = true;
			else
			{
				m_outputPath = m_options["output"].as<std::string>();

				if (!std::filesystem::is_directory(m_outputPath) && !std::filesystem::create_directories(m_outputPath))
					throw std::runtime_error(fmt::format("failed to create {} directory", m_outputPath.generic_u8string()));
			}
		}

		if (m_options.count("benchmark-iteration") > 0)
		{
			m_profiling = true;
			m_iterationCount = m_options["benchmark-iteration"].as<unsigned int>();
			if (m_iterationCount == 0)
				throw cxxopts::OptionException("iteration count must be over zero");
		}

		if (m_options.count("measure") > 0)
			m_profiling = m_options["measure"].as<bool>();

		m_verbose = m_options.count("verbose") > 0;
	}
	
	void Compiler::PrintError(const nzsl::Error& error) const
	{
		const nzsl::SourceLocation& errorLocation = error.GetSourceLocation();
		if (errorLocation.IsValid())
		{
			if (m_logFormat == LogFormat::Classic)
			{
				fmt::print(stderr, (fmt::emphasis::bold | fg(fmt::color::red)), "{}\n", error.what());

				try
				{
					// Retrieve line
					std::string sourceContent = ReadSourceFileContent(*errorLocation.file);

					std::size_t lineStartOffset = 0;
					if (errorLocation.startLine > 1)
					{
						lineStartOffset = sourceContent.find('\n') + 1;
						for (std::size_t i = 0; i < errorLocation.startLine - 2; ++i) //< remember startLine is 1-based
						{
							lineStartOffset = sourceContent.find('\n', lineStartOffset);
							if (lineStartOffset == std::string::npos)
								throw std::runtime_error("file content doesn't match original source");

							++lineStartOffset;
						}
					}
					std::size_t lineEndOffset = sourceContent.find('\n', lineStartOffset);

					std::string errorLine = sourceContent.substr(lineStartOffset, lineEndOffset - lineStartOffset);

					// handle tabs
					std::uint32_t startColumn = errorLocation.startColumn - 1;
					std::size_t startPos = 0;
					while ((startPos = errorLine.find('\t', startPos)) != std::string::npos)
					{
						if (startPos < startColumn)
							startColumn += 3;

						errorLine.replace(startPos, 1, "    ");
						startPos += 4;
					}

					std::size_t columnSize;
					if (errorLocation.startLine == errorLocation.endLine)
						columnSize = errorLocation.endColumn - errorLocation.startColumn + 1;
					else
						columnSize = 1;

					std::string lineStr = std::to_string(errorLocation.startLine);

					fmt::print(stderr, " {} | {}\n", lineStr, errorLine);
					fmt::print(stderr, " {} | {}", std::string(lineStr.size(), ' '), std::string(startColumn, ' '));
					fmt::print(stderr, fg(fmt::color::green), "{}\n", std::string(columnSize, '^'));
				}
				catch (const std::exception& e)
				{
					fmt::print(stderr, "failed to print error line: {}\n", e.what());
				}
			}
			else if (m_logFormat == LogFormat::VisualStudio)
			{
				// VS requires absolute path
				std::filesystem::path fullPath;
				if (errorLocation.file)
					fullPath = std::filesystem::absolute(*errorLocation.file);

				fmt::print(stderr, "{}({},{}): error {}: {}\n", fullPath.generic_u8string(), errorLocation.startLine, errorLocation.startColumn, ToString(error.GetErrorType()), error.GetErrorMessage());
			}
		}
		else
			fmt::print(stderr, (fmt::emphasis::bold | fg(fmt::color::red)), "{}\n", error.what());
	}

	void Compiler::Process()
	{
		using namespace std::literals;

		Step("Full processing"sv, [&]
		{
			Step("Read input file"sv, &Compiler::ReadInput);
			Step("Processing"sv, &Compiler::Sanitize);

			if (m_options.count("compile") > 0)
				Step("Compiling"sv, &Compiler::Compile);
		});

		if (m_profiling)
			PrintTime();
	}

	cxxopts::Options Compiler::BuildOptions()
	{
		cxxopts::Options options("nzslc", "Tool for validating and compiling NZSL shaders");

		options.add_options()
			("benchmark-iteration", "Benchmark each step of the compilation repeat over a huge number of time, implies --measure", cxxopts::value<unsigned int>()->implicit_value("1000"))
			("measure", "Measure time taken for every step of the compilation process", cxxopts::value<bool>()->default_value("false"))
			("log-format", "Set log format (classic, vs)", cxxopts::value<std::string>())
			("i,input", "Input file(s)", cxxopts::value<std::string>())
			("o,output", "Output path (use @stdout to output on stdout)", cxxopts::value<std::string>()->default_value("."), "path")
			("s,show", "Show informations about the shader (default)")
			("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
			("h,help", "Print usage")
			("version", "Print version");

		options.add_options("compilation")
			("c,compile", R"(Compile input shader to the following format. Possible values are:
- glsl : GLSL (GLSL ES if --gl-es is set)
- nzsl : textual NZSL
- nzslb : binary NZSL
- spv : binary SPIR-V
- spv-dis : textual SPIR-V

Multiple values can be specified using commas (ex: --compile=glsl,nzslb).
You can also specify -header as a suffix (ex: --compile=glsl-header) to generate an includable header file.
)", cxxopts::value<std::vector<std::string>>()->implicit_value("nzslb"))
			("m,module", "Module file or directory", cxxopts::value<std::vector<std::string>>())
			("optimize", "Optimize shader code")
			("p,partial", "Allow partial compilation");

		options.add_options("glsl output")
			("gl-es", "Generate GLSL ES instead of GLSL", cxxopts::value<bool>()->default_value("false"))
			("gl-version", "OpenGL version (310 being 3.1)", cxxopts::value<std::uint32_t>(), "version")
			("gl-entry", "Shader entry point (required for files having multiple entry points)", cxxopts::value<std::string>(), "[frag|vert]")
			("gl-flipy", "Add code to conditionally flip gl_Position Y value")
			("gl-remapz", "Add code to remap gl_Position Z value from [0;1] to [-1;1]")
			("gl-bindingmap", "Add binding support (generates a .binding.json mapping file)");

		options.add_options("spirv output")
			("spv-version", "SPIR-V version (110 being 1.1)", cxxopts::value<std::uint32_t>(), "version");

		options.parse_positional("input");
		options.positional_help("shader path");

		return options;
	}

	void Compiler::Compile()
	{
		using namespace std::literals;

		// if no output path has been provided, output in the same folder as the input file
		std::filesystem::path outputFilePath = m_outputPath;
		if (outputFilePath.empty())
			outputFilePath = m_inputFilePath.parent_path();

		outputFilePath /= m_inputFilePath.filename();

		const std::vector<std::string>& options = m_options["compile"].as<std::vector<std::string>>();
		for (std::string_view outputType : options)
		{
			// TODO: Don't compile multiple times unnecessary (ex: glsl and glsl-header)
			if (m_outputToStdout && options.size() > 1)
				fmt::print("-- {}\n", outputType);

			m_outputHeader = EndsWith(outputType, "-header");
			if (m_outputHeader)
				outputType.remove_suffix(7);

			if (outputType == "nzsl")
				Step("Compile to NZSL", &Compiler::CompileToNZSL, outputFilePath, *m_shaderModule);
			else if (outputType == "nzslb")
				Step("Compile to NZSLB", &Compiler::CompileToNZSLB, outputFilePath, m_shaderModule);
			else if (outputType == "spv")
				Step("Compile to SPIR-V", &Compiler::CompileToSPV, outputFilePath, *m_shaderModule, false);
			else if (outputType == "spv-dis")
				Step("Compile to textual SPIR-V", &Compiler::CompileToSPV, outputFilePath, *m_shaderModule, true);
			else if (outputType == "glsl")
				Step("Compile to GLSL", &Compiler::CompileToGLSL, outputFilePath, *m_shaderModule);
			else
			{
				fmt::print("Unknown format {}, ignoring\n", outputType);
				continue;
			}
		}
	}

	void Compiler::CompileToGLSL(std::filesystem::path outputPath, const nzsl::Ast::Module& module)
	{
		nzsl::ShaderWriter::States states;
		states.optimize = (m_options.count("optimize") > 0);

		nzsl::GlslWriter::Environment env;
		if (m_options.count("gl-es") > 0)
			env.glES = m_options["gl-es"].as<bool>();

		env.flipYPosition = (m_options.count("gl-flipy") > 0);
		env.remapZPosition = (m_options.count("gl-remapz") > 0);

		if (m_options.count("gl-version") > 0)
		{
			std::uint32_t minVersion = (env.glES) ? 300 : 330; //< OpenGL ES 3.0 and OpenGL 3.3 are the last versions of OpenGL
			std::uint32_t maxVersion = (env.glES) ? 320 : 460; //< OpenGL ES 3.2 and OpenGL 4.6 are the last versions of OpenGL

			std::uint32_t version = m_options["gl-version"].as<std::uint32_t>();
			if (version < minVersion || version > maxVersion)
				throw std::runtime_error(fmt::format("invalid GLSL version (must be between {} and {})", minVersion, maxVersion));

			env.glMajorVersion = version / 100;
			env.glMinorVersion = (version % 100) / 10;
		}

		nzsl::GlslWriter::BindingMapping bindingMapping;
		if (m_options.count("gl-bindingmap") > 0)
		{
			unsigned int glslBinding = 0;
			nlohmann::json bindingArray;

			nzsl::Ast::ReflectVisitor::Callbacks callbacks;
			callbacks.onExternalDeclaration = [&](const nzsl::Ast::DeclareExternalStatement& extDecl)
			{
				std::uint32_t extSet = 0;
				if (extDecl.bindingSet.HasValue())
				{
					if (!extDecl.bindingSet.IsResultingValue())
						throw std::runtime_error(fmt::format("external block on line {} has unresolved binding set", extDecl.sourceLocation.startLine));

					extSet = extDecl.bindingSet.GetResultingValue();
				}

				for (auto& extVar : extDecl.externalVars)
				{
					std::uint64_t bindingSet = extSet;
					if (extVar.bindingSet.HasValue())
					{
						if (!extVar.bindingSet.IsResultingValue())
							throw std::runtime_error(fmt::format("external var on line {} has unresolved binding set", extVar.sourceLocation.startLine));

						extSet = extVar.bindingSet.GetResultingValue();
					}

					std::uint64_t bindingIndex;
					if (!extVar.bindingIndex.IsResultingValue())
						throw std::runtime_error(fmt::format("external var on line {} has unresolved binding index", extVar.sourceLocation.startLine));

					bindingIndex = extVar.bindingIndex.GetResultingValue();

					std::uint64_t binding = bindingSet << 32 | bindingIndex;
					bindingMapping.emplace(binding, glslBinding);

					nlohmann::json& bindingDoc = bindingArray.emplace_back();
					bindingDoc["set"] = bindingSet;
					bindingDoc["binding"] = bindingIndex;
					bindingDoc["glsl_binding"] = glslBinding;

					glslBinding++;
				}
			};

			nzsl::Ast::ReflectVisitor reflectVisitor;
			reflectVisitor.Reflect(module, callbacks);

			if (!bindingArray.empty())
			{
				nlohmann::json finalDoc;
				finalDoc["bindings"] = std::move(bindingArray);

				std::string bindingStr = finalDoc.dump(4);

				std::filesystem::path bindingOutputPath = outputPath;
				bindingOutputPath.replace_extension("glsl.binding.json");
				OutputFile(std::move(bindingOutputPath), bindingStr.data(), bindingStr.size());
			}
		}

		nzsl::GlslWriter writer;
		writer.SetEnv(env);

		std::optional<nzsl::ShaderStageType> entryType;
		if (m_options.count("gl-entry") > 0)
		{
			std::string entryName = m_options["gl-entry"].as<std::string>();
			if (entryName == "frag")
				entryType = nzsl::ShaderStageType::Fragment;
			else if (entryName == "vert")
				entryType = nzsl::ShaderStageType::Vertex;
			else
				throw std::runtime_error("unrecognized gl-entry " + entryName);
		}

		nzsl::GlslWriter::Output output = writer.Generate(entryType, module, bindingMapping, states);

		if (m_outputToStdout)
		{
			OutputToStdout(output.code);
			return;
		}

		outputPath.replace_extension("glsl");
		OutputFile(std::move(outputPath), output.code.data(), output.code.size());
	}

	void Compiler::CompileToNZSL(std::filesystem::path outputPath, const nzsl::Ast::Module& module)
	{
		nzsl::ShaderWriter::States states;
		states.optimize = (m_options.count("optimize") > 0);

		nzsl::LangWriter nzslWriter;
		std::string nzsl = nzslWriter.Generate(module, states);

		if (m_outputToStdout)
		{
			OutputToStdout(nzsl);
			return;
		}

		outputPath.replace_extension("nzsl");
		OutputFile(std::move(outputPath), nzsl.data(), nzsl.size());
	}

	void Compiler::CompileToNZSLB(std::filesystem::path outputPath, nzsl::Ast::ModulePtr& module)
	{
		nzsl::Serializer serializer;
		nzsl::Ast::SerializeShader(serializer, module);

		const std::vector<std::uint8_t>& data = serializer.GetData();

		if (m_outputToStdout)
		{
			if (m_outputHeader)
				OutputToStdout(std::string_view(reinterpret_cast<const char*>(&data[0]), data.size()));
			else
				fmt::print("NZSLB is a binary format and cannot be printed to stdout\n");

			return;
		}

		outputPath.replace_extension("nzslb");
		OutputFile(std::move(outputPath), data.data(), data.size());
	}

	void Compiler::CompileToSPV(std::filesystem::path outputPath, const nzsl::Ast::Module& module, bool textual)
	{
		nzsl::ShaderWriter::States states;
		states.optimize = (m_options.count("optimize") > 0);

		nzsl::SpirvWriter::Environment env;
		if (m_options.count("spv-version"))
		{
			constexpr std::uint32_t maxVersion = nzsl::SpirvMajorVersion * 100 + nzsl::SpirvMinorVersion * 10;

			std::uint32_t version = m_options["spv-version"].as<std::uint32_t>();
			if (version < 100 || version > maxVersion)
				throw std::runtime_error(fmt::format("invalid SPIR-V version (must be between 100 and {})", maxVersion));

			env.spvMajorVersion = version / 100;
			env.spvMinorVersion = (version % 100) / 10;
		}

		nzsl::SpirvWriter writer;
		writer.SetEnv(env);

		std::vector<std::uint32_t> spirv = writer.Generate(module, states);
		std::size_t size = spirv.size() * sizeof(std::uint32_t);

		if (textual)
		{
			nzsl::SpirvPrinter printer;
			std::string spirvTxt = printer.Print(spirv);

			if (m_outputToStdout)
			{
				OutputToStdout(spirvTxt);
				return;
			}

			outputPath.replace_extension("spv.txt");
			OutputFile(std::move(outputPath), spirvTxt.data(), spirvTxt.size());
		}
		else
		{
			if (m_outputToStdout)
			{
				if (m_outputHeader)
					OutputToStdout(std::string_view(reinterpret_cast<const char*>(&spirv[0]), spirv.size() * sizeof(std::uint32_t)));
				else
					fmt::print("SPIR-V is a binary format and cannot be printed to stdout\n");

				return;
			}

			outputPath.replace_extension("spv");
			OutputFile(std::move(outputPath), spirv.data(), size);
		}
	}

	void Compiler::PrintTime()
	{
		long long fullTime = std::max(m_steps[0].time, 1LL); //< prevent a divison by zero

		struct StepParent
		{
			std::size_t parentStep;
			std::size_t childRemaining;
		};

		std::vector<StepParent> parents;
		for (std::size_t i = 0; i < m_steps.size(); ++i)
		{
			if (!parents.empty())
			{
				StepParent& remainingChild = parents.back();
				assert(remainingChild.childRemaining > 0);
				remainingChild.childRemaining--;

				if (remainingChild.childRemaining == 0)
					parents.pop_back();
			}

			auto& step = m_steps[i];
			fmt::print("{}- {}: {}", std::string(parents.size() * 2, ' '), step.name, fmt::format(fg(fmt::color::dark_golden_rod), "{}us", step.time / m_iterationCount), 0);
			if (!parents.empty())
			{
				std::size_t parentIndex = parents.back().parentStep;

				long long globalTime = std::min(100 * step.time / fullTime, 100LL);
				if (parentIndex != 0)
				{
					long long parentTime = std::max(m_steps[parentIndex].time, 1LL); //< prevent a divison by zero
					long long localTime = std::min(100 * step.time / parentTime, 100LL);
					fmt::print(" (global: {}% - local: {}%)", globalTime, localTime);
				}
				else
					fmt::print(" (global: {}%)", globalTime);
			}
			fmt::print("\n");

			if (step.childrenCount > 0)
			{
				auto& parent = parents.emplace_back();
				parent.childRemaining = step.childrenCount + 1;
				parent.parentStep = i;
			}
		}
	}

	void Compiler::OutputToStdout(std::string_view str)
	{
		if (m_outputHeader)
			fmt::print("{}", ToHeader(str.data(), str.size()));
		else
			fmt::print("{}", str);
	}

	void Compiler::OutputFile(std::filesystem::path filePath, const void* data, std::size_t size)
	{
		if (m_outputHeader)
		{
			std::string headerFile = ToHeader(data, size);

			filePath.replace_extension(filePath.extension().generic_u8string() + ".h");
			WriteFileContent(filePath, headerFile.data(), headerFile.size());
		}
		else
			WriteFileContent(filePath, data, size);

		if (m_verbose)
			fmt::print("Generated file {}\n", std::filesystem::absolute(filePath).generic_u8string());
	}

	void Compiler::ReadInput()
	{
		using namespace std::literals;

		std::filesystem::path extension = m_inputFilePath.extension();

		if (extension == ".nzsl")
		{
			std::string sourceContent = Step("File reading"sv, &Compiler::ReadSourceFileContent, m_inputFilePath);
			m_shaderModule = Step("Parse input"sv, &Compiler::Parse, sourceContent, m_inputFilePath.generic_u8string());
		}
		else if (extension == ".nzslb")
		{
			std::vector<std::uint8_t> sourceContent = Step("File reading"sv, &Compiler::ReadFileContent, m_inputFilePath);
			m_shaderModule = Step("Unserialize input"sv, &Compiler::Unserialize, sourceContent.data(), sourceContent.size());
		}
		else
			throw std::runtime_error(fmt::format("{} has unknown extension \"{}\"", m_inputFilePath.filename().generic_u8string(), extension.generic_u8string()));
	}

	void Compiler::Sanitize()
	{
		using namespace std::literals;

		nzsl::Ast::SanitizeVisitor::Options sanitizeOptions;
		sanitizeOptions.allowPartialSanitization = m_options.count("partial") > 0;

		if (m_options.count("module") > 0)
		{
			std::shared_ptr<nzsl::FilesystemModuleResolver> resolver = std::make_shared<nzsl::FilesystemModuleResolver>();

			for (const std::string& modulePath : m_options["module"].as<std::vector<std::string>>())
			{
				std::filesystem::path path = std::filesystem::u8path(modulePath);
				if (std::filesystem::is_regular_file(path))
				{
					std::string stepName = "Register module " + path.generic_u8string();
					Step(stepName, [&] { resolver->RegisterModule(path); });
				}
				else if (std::filesystem::is_directory(path))
				{
					std::string stepName = "Register module directory " + path.generic_u8string();
					Step(stepName, [&] { resolver->RegisterModuleDirectory(path); });
				}
				else
					throw std::runtime_error(modulePath + " is not a path nor a directory");
			}

			sanitizeOptions.moduleResolver = std::move(resolver);
		}

		m_shaderModule = Step("AST processing"sv, [&] { return nzsl::Ast::Sanitize(*m_shaderModule, sanitizeOptions); });
	}

	template<typename F, typename... Args>
	auto Compiler::Step(std::enable_if_t<!std::is_member_function_pointer_v<F>, std::string_view> stepName, F&& func, Args&&... args) -> decltype(std::invoke(func, std::forward<Args>(args)...))
	{
		return StepInternal(stepName, [&] { return std::invoke(func, std::forward<Args>(args)...); });
	}

	template<typename F, typename... Args>
	auto Compiler::Step(std::enable_if_t<std::is_member_function_pointer_v<F>, std::string_view> stepName, F&& func, Args&&... args) -> decltype(std::invoke(func, this, std::forward<Args>(args)...))
	{
		return StepInternal(stepName, [&] { return std::invoke(func, this, std::forward<Args>(args)...); });
	}

	template<typename F>
	auto Compiler::StepInternal(std::string_view stepName, F&& func) -> decltype(func())
	{
		if (!m_profiling)
			return func();

		std::size_t stepIndex = m_steps.size();

		bool wasVerbose = m_verbose;

		// Disable substeps benchmarking (and verbose messages) when benchmarking a step
		m_profiling = false;
		m_verbose = false;
		{
			auto& step = m_steps.emplace_back();
			step.name = stepName;

			auto start = std::chrono::steady_clock::now();

			for (unsigned int i = 0; i < m_iterationCount; ++i)
				func();

			auto end = std::chrono::steady_clock::now();

			step.time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		}
		m_profiling = true;
		m_verbose = wasVerbose;
		
		Nz::CallOnExit onExit([&]
		{
			auto& step = m_steps[stepIndex];
			step.childrenCount = m_steps.size() - stepIndex - 1;
		});

		return func();
	}

	nzsl::Ast::ModulePtr Compiler::Unserialize(const std::uint8_t* data, std::size_t size)
	{
		nzsl::Unserializer unserializer(data, size);
		return nzsl::Ast::UnserializeShader(unserializer);
	}

	nzsl::Ast::ModulePtr Compiler::Parse(std::string_view sourceContent, const std::string& filePath)
	{
		std::vector<nzsl::Token> tokens = nzsl::Tokenize(sourceContent, filePath);
		return nzsl::Parse(tokens);
	}

	std::vector<std::uint8_t> Compiler::ReadFileContent(const std::filesystem::path& filePath)
	{
		std::ifstream inputFile(filePath, std::ios::in | std::ios::binary);
		if (!inputFile)
			throw std::runtime_error("failed to open " + filePath.generic_u8string());

		inputFile.seekg(0, std::ios::end);

		std::streamsize length = inputFile.tellg();

		inputFile.seekg(0, std::ios::beg);

		std::vector<std::uint8_t> content(Nz::SafeCast<std::size_t>(length));
		if (length > 0 && !inputFile.read(reinterpret_cast<char*>(&content[0]), length))
			throw std::runtime_error("failed to read " + filePath.generic_u8string());

		return content;
	}

	std::string Compiler::ReadSourceFileContent(const std::filesystem::path& filePath)
	{
		std::vector<std::uint8_t> content = ReadFileContent(filePath);
		return std::string(reinterpret_cast<const char*>(&content[0]), content.size());
	}

	std::string Compiler::ToHeader(const void* data, std::size_t size)
	{
		const std::uint8_t* ptr = static_cast<const std::uint8_t*>(data);

		std::stringstream ss;

		bool first = true;
		for (std::size_t i = 0; i < size; ++i)
		{
			if (!first)
			{
				ss << ',';
				if (i % 30 == 0)
					ss << '\n';
			}

			first = false;

			ss << +ptr[i];
		}
		
		ss << '\n';

		return std::move(ss).str();
	}

	void Compiler::WriteFileContent(const std::filesystem::path& filePath, const void* data, std::size_t size)
	{
		std::ofstream outputFile(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outputFile)
			throw std::runtime_error(fmt::format("failed to open {}, reason: {}", filePath.generic_u8string(), std::strerror(errno)));

		if (!outputFile.write(static_cast<const char*>(data), size))
			throw std::runtime_error(fmt::format("failed to write {}, reason: {}", filePath.generic_u8string(), std::strerror(errno)));
	}
}
