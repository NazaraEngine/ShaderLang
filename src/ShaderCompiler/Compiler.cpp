// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <ShaderCompiler/Compiler.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <NZSL/BackendParameters.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/GlslWriter.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lexer.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/SpirV/SpirvPrinter.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/ReflectVisitor.hpp>
#include <NZSL/Ast/Transformations/ResolveTransformer.hpp>
#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
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

		constexpr auto s_debugLevels = frozen::make_unordered_map<frozen::string, nzsl::DebugLevel>({
			{ "full",    nzsl::DebugLevel::Full },
			{ "minimal", nzsl::DebugLevel::Minimal },
			{ "regular", nzsl::DebugLevel::Regular },
			{ "none",    nzsl::DebugLevel::None }
		});
	}

	Compiler::Compiler(cxxopts::ParseResult& options) :
	m_parentStep(Nz::MaxValue()),
	m_logFormat(LogFormat::Classic),
	m_options(options),
	m_isProfiling(false),
	m_outputToStdout(false),
	m_skipOutput(false),
	m_skipUnchangedOutput(false),
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
				throw cxxopts::exceptions::specification(fmt::format("{} is not a valid error format", formatStr));
		}

		if (m_options.count("input") == 0)
			throw cxxopts::exceptions::specification("no input file");

		m_inputFilePath = Nz::Utf8Path(m_options["input"].as<std::string>());
		if (!std::filesystem::is_regular_file(m_inputFilePath))
			throw std::runtime_error(fmt::format("{} is not a file", Nz::PathToString(m_inputFilePath)));

		if (m_options.count("output") > 0)
		{
			const std::string& outputPath = m_options["output"].as<std::string>();

			if (outputPath == "@stdout")
				m_outputToStdout = true;
			else if (outputPath == "@null")
				m_skipOutput = true;
			else
			{
				m_outputPath = Nz::Utf8Path(outputPath);

				if (!std::filesystem::is_directory(m_outputPath) && !std::filesystem::create_directories(m_outputPath))
					throw std::runtime_error(fmt::format("failed to create {} directory", Nz::PathToString(m_outputPath)));
			}
		}

		if (m_options.count("benchmark-iteration") > 0)
		{
			m_isProfiling = true;
			m_iterationCount = m_options["benchmark-iteration"].as<unsigned int>();
			if (m_iterationCount == 0)
				throw cxxopts::exceptions::specification("iteration count must be over zero");
		}

		if (m_options.count("measure") > 0)
			m_isProfiling = m_options["measure"].as<bool>();

		m_skipUnchangedOutput = m_options.count("skip-unchanged") > 0;
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

				fmt::print(stderr, "{}({},{}): error {}: {}\n", Nz::PathToString(fullPath), errorLocation.startLine, errorLocation.startColumn, ToString(error.GetErrorType()), error.GetErrorMessage());
			}
		}
		else
			fmt::print(stderr, (fmt::emphasis::bold | fg(fmt::color::red)), "{}\n", error.what());
	}

	void Compiler::Process()
	{
		using namespace std::literals;

		for (unsigned int i = 0; i < m_iterationCount; ++i)
		{
			Step("Full processing"sv, __LINE__, [&]
			{
				Step("Read input file"sv, __LINE__, &Compiler::ReadInput);
				Step("Processing"sv, __LINE__, &Compiler::Resolve);

				if (m_options.count("compile") > 0)
					Step("Compiling"sv, __LINE__, &Compiler::Compile);
			});
		}

		if (m_isProfiling)
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
			("d,debug-level", "Debug level to generate", cxxopts::value<std::string>(), "[none|minimal|regular|full]")
			("m,module", "Module file or directory", cxxopts::value<std::vector<std::string>>())
			("optimize", "Optimize shader code")
			("p,partial", "Allow partial compilation")
			("skip-unchanged", "After compilation, compare the output with the current output file and skip writing if the content is the same", cxxopts::value<bool>()->default_value("false"));

		options.add_options("glsl output")
			("gl-es", "Generate GLSL ES instead of GLSL", cxxopts::value<bool>()->default_value("false"))
			("gl-version", "OpenGL version (310 being 3.1)", cxxopts::value<std::uint32_t>(), "version")
			("gl-flipy", "Add code to conditionally flip gl_Position Y value")
			("gl-remapz", "Add code to remap gl_Position Z value from [0;1] to [-1;1]")
			("gl-bindingmap", "Add binding support (generates a .binding.json mapping file)");

		options.add_options("spirv output")
			("spv-version", "SPIR-V version (110 being 1.1)", cxxopts::value<std::uint32_t>(), "version");

		options.parse_positional("input");
		options.positional_help("shader path");

		return options;
	}

	nzsl::BackendParameters Compiler::BuildWriterOptions()
	{
		nzsl::BackendParameters parameters;

		if (m_options.count("optimize"))
			parameters.backendPasses |= nzsl::BackendPass::Optimize | nzsl::BackendPass::RemoveDeadCode;

		if (m_options.count("debug-level"))
		{
			const std::string& debugLevelStr = m_options["debug-level"].as<std::string>();

			auto it = s_debugLevels.find(frozen::string(debugLevelStr));
			if (it == s_debugLevels.end())
				throw cxxopts::exceptions::specification("invalid debug-level " + debugLevelStr);

			parameters.debugLevel = it->second;
		}

		return parameters;
	}

	void Compiler::Compile()
	{
		using namespace std::literals;

		// if no output path has been provided, output in the same folder as the input file
		std::filesystem::path outputFilePath = m_outputPath;
		if (outputFilePath.empty())
			outputFilePath = m_inputFilePath.parent_path();

		outputFilePath /= m_inputFilePath.filename();

		const std::vector<std::string>& compileOptions = m_options["compile"].as<std::vector<std::string>>();
		for (std::size_t i = 0; i < compileOptions.size(); ++i)
		{
			std::string_view outputType = compileOptions[i];

			// TODO: Don't compile multiple times unnecessary (ex: glsl and glsl-header)
			if (!m_skipOutput && m_outputToStdout && compileOptions.size() > 1)
				fmt::print("-- {}\n", outputType);

			m_outputHeader = EndsWith(outputType, "-header");
			if (m_outputHeader)
				outputType.remove_suffix(7);

			// Clone the AST if multiple outputs are required (to avoid the transformations from the first compilation to go to the second)
			nzsl::Ast::Module* targetModule;
			nzsl::Ast::ModulePtr cloneModule;
			if (i != compileOptions.size() - 1)
			{
				cloneModule = nzsl::Ast::Clone(*m_shaderModule);
				targetModule = cloneModule.get();
			}
			else
				targetModule = m_shaderModule.get();

			if (outputType == "nzsl")
				Step("Compile to NZSL", __LINE__, &Compiler::CompileToNZSL, outputFilePath, *targetModule);
			else if (outputType == "nzslb")
				Step("Compile to NZSLB", __LINE__, &Compiler::CompileToNZSLB, outputFilePath, *targetModule);
			else if (outputType == "spv")
				Step("Compile to SPIR-V", __LINE__, &Compiler::CompileToSPV, outputFilePath, *targetModule, false);
			else if (outputType == "spv-dis")
				Step("Compile to textual SPIR-V", __LINE__, &Compiler::CompileToSPV, outputFilePath, *targetModule, true);
			else if (outputType == "glsl")
				Step("Compile to GLSL", __LINE__, &Compiler::CompileToGLSL, outputFilePath, *targetModule);
			else
			{
				fmt::print("Unknown format {}, ignoring\n", outputType);
				continue;
			}
		}
	}

	void Compiler::CompileToGLSL(std::filesystem::path outputPath, nzsl::Ast::Module& module)
	{
		nzsl::GlslWriter::Environment env;
		if (m_options.count("gl-es") > 0)
			env.glES = m_options["gl-es"].as<bool>();

		env.flipYPosition = (m_options.count("gl-flipy") > 0);
		env.remapZPosition = (m_options.count("gl-remapz") > 0);

		if (m_options.count("gl-version") > 0)
		{
			std::uint32_t minVersion = (env.glES) ? 300 : 330; //< OpenGL ES 3.0 and OpenGL 3.3 are the lowest supported versions of OpenGL
			std::uint32_t maxVersion = (env.glES) ? 320 : 460; //< OpenGL ES 3.2 and OpenGL 4.6 are the highest supported versions of OpenGL

			std::uint32_t version = m_options["gl-version"].as<std::uint32_t>();
			if (version < minVersion || version > maxVersion)
				throw std::runtime_error(fmt::format("invalid GLSL version (must be between {} and {})", minVersion, maxVersion));

			env.glMajorVersion = version / 100;
			env.glMinorVersion = (version % 100) / 10;
		}

		nzsl::GlslWriter::Parameters parameters;
		if (m_options.count("gl-bindingmap") > 0)
		{
			unsigned int glslBinding = 0;
			nlohmann::ordered_json bindingArray;

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
					if (IsPushConstantType(extVar.type.GetResultingValue()))
					{
						parameters.pushConstantBinding = glslBinding++;
						continue;
					}

					std::uint64_t bindingSet = extSet;
					if (extVar.bindingSet.HasValue())
					{
						if (!extVar.bindingSet.IsResultingValue())
							throw std::runtime_error(fmt::format("external var on line {} has unresolved binding set", extVar.sourceLocation.startLine));

						bindingSet = extVar.bindingSet.GetResultingValue();
					}

					std::uint64_t bindingIndex;
					if (!extVar.bindingIndex.IsResultingValue())
						throw std::runtime_error(fmt::format("external var on line {} has unresolved binding index", extVar.sourceLocation.startLine));

					bindingIndex = extVar.bindingIndex.GetResultingValue();

					std::uint64_t binding = bindingSet << 32 | bindingIndex;
					parameters.bindingMapping.emplace(binding, glslBinding);

					nlohmann::ordered_json& bindingDoc = bindingArray.emplace_back();
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
				nlohmann::ordered_json finalDoc;
				finalDoc["bindings"] = std::move(bindingArray);

				if (parameters.pushConstantBinding.has_value())
					finalDoc["push_constant_binding"] = *parameters.pushConstantBinding;

				std::string bindingStr = finalDoc.dump(4);

				std::filesystem::path bindingOutputPath = outputPath;
				bindingOutputPath.replace_extension("glsl.binding.json");
				OutputFile(std::move(bindingOutputPath), bindingStr.data(), bindingStr.size(), true);
			}
		}

		nzsl::GlslWriter writer;
		writer.SetEnv(env);

		nzsl::ShaderStageTypeFlags entryTypes;
		nzsl::Ast::ReflectVisitor::Callbacks callbacks;
		callbacks.onEntryPointDeclaration = [&](nzsl::ShaderStageType shaderStage, const std::string& /*functionName*/)
		{
			entryTypes |= shaderStage;
		};

		nzsl::Ast::ReflectVisitor reflectVisitor;
		reflectVisitor.Reflect(module, callbacks);

		if (entryTypes == 0)
			throw std::runtime_error("shader has no entry function!");

		nzsl::BackendParameters backendParameters = BuildWriterOptions();

		std::size_t remainingEntryType = entryTypes.size();
		nzsl::Ast::ModulePtr clonedModule;
		nzsl::Ast::Module* targetModule = &module;

		bool first = true;
		for (nzsl::ShaderStageType entryType : entryTypes)
		{
			remainingEntryType--;

			// If dead code removal is enabled, we have to clone AST before generation if we have more than one entry type
			if (backendParameters.backendPasses.Test(nzsl::BackendPass::RemoveDeadCode) && remainingEntryType > 0)
			{
				clonedModule = nzsl::Ast::Clone(module);
				targetModule = clonedModule.get();
			}

			nzsl::GlslWriter::Output output = writer.Generate(entryType, *targetModule, backendParameters, parameters);
			if (m_skipOutput)
				continue;

			if (m_outputToStdout)
			{
				const char* stageName = nullptr;
				switch (entryType)
				{
					case nzsl::ShaderStageType::Compute:  stageName = "Compute"; break;
					case nzsl::ShaderStageType::Fragment: stageName = "Fragment"; break;
					case nzsl::ShaderStageType::Vertex:   stageName = "Vertex"; break;
				}
				fmt::print("{}{}:\n", (first) ? "" : "\n", stageName);
				OutputToStdout(output.code);
				continue;
			}

			std::filesystem::path filePath = outputPath;
			switch (entryType)
			{
				case nzsl::ShaderStageType::Compute:  filePath.replace_extension("comp.glsl"); break;
				case nzsl::ShaderStageType::Fragment: filePath.replace_extension("frag.glsl"); break;
				case nzsl::ShaderStageType::Vertex:   filePath.replace_extension("vert.glsl"); break;
			}

			OutputFile(std::move(filePath), output.code.data(), output.code.size());
			first = false;
		}
	}

	void Compiler::CompileToNZSL(std::filesystem::path outputPath, const nzsl::Ast::Module& module)
	{
		nzsl::LangWriter nzslWriter;
		std::string nzsl = nzslWriter.Generate(module);
		if (m_skipOutput)
			return;

		if (m_outputToStdout)
		{
			OutputToStdout(nzsl);
			return;
		}

		outputPath.replace_extension("nzsl");
		OutputFile(std::move(outputPath), nzsl.data(), nzsl.size());
	}

	void Compiler::CompileToNZSLB(std::filesystem::path outputPath, const nzsl::Ast::Module& module)
	{
		nzsl::Serializer serializer;
		nzsl::Ast::SerializeShader(serializer, module);
		if (m_skipOutput)
			return;

		const std::vector<std::uint8_t>& data = serializer.GetData();

		if (m_outputToStdout)
		{
			if (m_outputHeader)
				OutputToStdout(std::string_view(reinterpret_cast<const char*>(&data[0]), data.size()));
			else
				throw std::runtime_error("NZSLB is a binary format and cannot be printed to stdout");

			return;
		}

		outputPath.replace_extension("nzslb");
		OutputFile(std::move(outputPath), data.data(), data.size());
	}

	void Compiler::CompileToSPV(std::filesystem::path outputPath, nzsl::Ast::Module& module, bool textual)
	{
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

		nzsl::BackendParameters states = BuildWriterOptions();

		std::vector<std::uint32_t> spirv = writer.Generate(module, states);
		std::size_t size = spirv.size() * sizeof(std::uint32_t);

		if (textual)
		{
			nzsl::SpirvPrinter printer;
			std::string spirvTxt = printer.Print(spirv);
			if (m_skipOutput)
				return;

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
			if (m_skipOutput)
				return;

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

	nzsl::Ast::ModulePtr Compiler::Deserialize(const std::uint8_t* data, std::size_t size)
	{
		nzsl::Deserializer deserializer(data, size);
		return nzsl::Ast::DeserializeShader(deserializer);
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

	void Compiler::OutputFile(std::filesystem::path filePath, const void* data, std::size_t size, bool disallowHeader)
	{
		std::string headerFile;
		if (m_outputHeader && !disallowHeader)
		{
			headerFile = ToHeader(data, size);
			filePath.replace_extension(Nz::PathToString(filePath.extension()) + ".h");
			data = headerFile.data();
			size = headerFile.size();
		}

		bool written = WriteFileContent(filePath, data, size);
		if (m_verbose)
			fmt::print("{} file {}\n", (written) ? "Generated" : "Skipped", Nz::PathToString(std::filesystem::absolute(filePath)));
	}

	void Compiler::OutputToStdout(std::string_view str)
	{
		if (m_outputHeader)
			fmt::print("{}", ToHeader(str.data(), str.size()));
		else
			fmt::print("{}", str);
	}

	void Compiler::ReadInput()
	{
		using namespace std::literals;

		std::filesystem::path extension = m_inputFilePath.extension();

		if (extension == ".nzsl")
		{
			std::string sourceContent = Step("File reading"sv, __LINE__, &Compiler::ReadSourceFileContent, m_inputFilePath);
			std::string sourceFile = Nz::PathToString(m_inputFilePath);
			std::vector<nzsl::Token> tokens = Step("Tokenizing", __LINE__, [&] { return nzsl::Tokenize(sourceContent, sourceFile); });
			m_shaderModule = Step("Parsing"sv, __LINE__, [&] { return nzsl::Parse(tokens); });
		}
		else if (extension == ".nzslb")
		{
			std::vector<std::uint8_t> sourceContent = Step("File reading"sv, __LINE__, &Compiler::ReadFileContent, m_inputFilePath);
			m_shaderModule = Step("Deserialize input"sv, __LINE__, &Compiler::Deserialize, sourceContent.data(), sourceContent.size());
		}
		else
			throw std::runtime_error(fmt::format("{} has unknown extension \"{}\"", Nz::PathToString(m_inputFilePath.filename()), Nz::PathToString(extension)));
	}

	void Compiler::Resolve()
	{
		using namespace std::literals;

		nzsl::Ast::TransformerContext context;
		context.partialCompilation = m_options.count("partial") > 0;

		nzsl::Ast::ResolveTransformer::Options resolverOpt;

		if (m_options.count("module") > 0)
		{
			std::shared_ptr<nzsl::FilesystemModuleResolver> resolver = std::make_shared<nzsl::FilesystemModuleResolver>();

			for (const std::string& modulePath : m_options["module"].as<std::vector<std::string>>())
			{
				std::filesystem::path path = Nz::Utf8Path(modulePath);
				if (std::filesystem::is_regular_file(path))
				{
					std::string stepName = "Register module " + Nz::PathToString(path);
					Step(stepName, __LINE__, [&] { resolver->RegisterFile(path); });
				}
				else if (std::filesystem::is_directory(path))
				{
					std::string stepName = "Register module directory " + Nz::PathToString(path);
					Step(stepName, __LINE__, [&] { resolver->RegisterDirectory(path); });
				}
				else
					throw std::runtime_error(modulePath + " is not a path nor a directory");
			}

			resolverOpt.moduleResolver = std::move(resolver);
		}

		nzsl::Ast::ResolveTransformer resolver;
		nzsl::Ast::ValidationTransformer validation;

		Step("AST processing"sv, __LINE__, [&] { resolver.Transform(*m_shaderModule, context, resolverOpt); });
		Step("AST validation"sv, __LINE__, [&] { validation.Transform(*m_shaderModule, context); });
	}

	template<typename F, typename... Args>
	auto Compiler::Step(std::enable_if_t<!std::is_member_function_pointer_v<F>, std::string_view> stepName, std::size_t uniqueIndex, F&& func, Args&&... args) -> decltype(std::invoke(func, std::forward<Args>(args)...))
	{
		return StepInternal(stepName, uniqueIndex, [&] { return std::invoke(func, std::forward<Args>(args)...); });
	}

	template<typename F, typename... Args>
	auto Compiler::Step(std::enable_if_t<std::is_member_function_pointer_v<F>, std::string_view> stepName, std::size_t uniqueIndex, F&& func, Args&&... args) -> decltype(std::invoke(func, this, std::forward<Args>(args)...))
	{
		return StepInternal(stepName, uniqueIndex, [&] { return std::invoke(func, this, std::forward<Args>(args)...); });
	}

	template<typename F>
	auto Compiler::StepInternal(std::string_view stepName, std::size_t uniqueIndex, F&& func) -> decltype(func())
	{
		if (!m_isProfiling)
			return func();

		std::size_t stepIndex;
		if (auto it = m_stepIndices.find(uniqueIndex); it == m_stepIndices.end())
		{
			stepIndex = m_steps.size();
			auto& step = m_steps.emplace_back();
			step.name = stepName;
			step.time = 0;

			if (m_parentStep != Nz::MaxValue<std::size_t>())
				m_steps[m_parentStep].childrenCount++;

			m_stepIndices.emplace(uniqueIndex, stepIndex);
		}
		else
			stepIndex = it->second;

		std::size_t prevParentStep = m_parentStep;
		bool wasVerbose = m_verbose;

		// Disable verbose messages when benchmarking a step
		m_parentStep = stepIndex;
		m_verbose = false;

		auto start = std::chrono::steady_clock::now();

		NAZARA_DEFER({
			auto end = std::chrono::steady_clock::now();

			m_steps[stepIndex].time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

			m_parentStep = prevParentStep;
			m_verbose = wasVerbose;
		});

		return func();
	}

	std::vector<std::uint8_t> Compiler::ReadFileContent(const std::filesystem::path& filePath)
	{
		std::uintmax_t fileSize = std::filesystem::file_size(filePath);

		std::ifstream inputFile(filePath, std::ios::in | std::ios::binary);
		if (!inputFile)
			throw std::runtime_error("failed to open " + Nz::PathToString(filePath));

		std::vector<std::uint8_t> content(Nz::SafeCast<std::size_t>(fileSize));
		if (fileSize > 0 && !inputFile.read(reinterpret_cast<char*>(&content[0]), fileSize))
			throw std::runtime_error("failed to read " + Nz::PathToString(filePath));

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

	bool Compiler::WriteFileContent(const std::filesystem::path& filePath, const void* data, std::size_t size)
	{
		if (m_skipUnchangedOutput)
		{
			std::ifstream inputFile(filePath, std::ios::in | std::ios::binary);
			if (inputFile)
			{
				std::size_t fileSize = Nz::SafeCaster(std::filesystem::file_size(filePath));
				if (fileSize == size)
				{
					if (size == 0)
						return false;

					// Compare file content
					std::vector<char> content(fileSize);
					if (inputFile.read(&content[0], fileSize) && std::memcmp(&content[0], data, size) == 0)
						return false;
				}
			}
		}

		std::ofstream outputFile(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outputFile)
			throw std::runtime_error(fmt::format("failed to open {}, reason: {}", Nz::PathToString(filePath), std::strerror(errno)));

		if (!outputFile.write(static_cast<const char*>(data), size))
			throw std::runtime_error(fmt::format("failed to write {}, reason: {}", Nz::PathToString(filePath), std::strerror(errno)));

		return true;
	}
}
