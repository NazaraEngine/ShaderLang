#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/GlslWriter.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/ShaderLangErrors.hpp>
#include <NZSL/ShaderLangLexer.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <NZSL/Ast/ReflectVisitor.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <cxxopts.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

constexpr std::uint32_t MajorVersion = 0;
constexpr std::uint32_t MinorVersion = 1;
constexpr std::uint32_t PatchVersion = 0;

enum class LogFormat
{
	Classic,
	VisualStudio
};

std::vector<std::uint8_t> ReadFileContent(const std::filesystem::path& filePath)
{
	std::ifstream inputFile(filePath, std::ios::in | std::ios::binary);
	if (!inputFile)
		throw std::runtime_error("failed to open " + filePath.generic_u8string());

	inputFile.seekg(0, std::ios::end);

	std::streamsize length = inputFile.tellg();

	inputFile.seekg(0, std::ios::beg);

	std::vector<std::uint8_t> content(length);
	if (length > 0 && !inputFile.read(reinterpret_cast<char*>(&content[0]), length))
		throw std::runtime_error("failed to read " + filePath.generic_u8string());

	return content;
}

std::string ReadSourceFileContent(const std::filesystem::path& filePath)
{
	std::vector<std::uint8_t> content = ReadFileContent(filePath);
	return std::string(reinterpret_cast<const char*>(&content[0]), content.size());
}

nzsl::Ast::ModulePtr Sanitize(const cxxopts::ParseResult& options, const nzsl::Ast::Module& module)
{
	nzsl::Ast::SanitizeVisitor::Options sanitizeOptions;
	sanitizeOptions.allowPartialSanitization = options.count("partial") > 0;

	if (options.count("module") > 0)
	{
		std::shared_ptr<nzsl::FilesystemModuleResolver> resolver = std::make_shared<nzsl::FilesystemModuleResolver>();

		for (const std::string& modulePath : options["module"].as<std::vector<std::string>>())
		{
			std::filesystem::path path = std::filesystem::u8path(modulePath);
			if (std::filesystem::is_regular_file(path))
				resolver->RegisterModule(path);
			else if (std::filesystem::is_directory(path))
				resolver->RegisterModuleDirectory(path);
			else
				throw std::runtime_error(modulePath + " is not a path nor a directory");
		}

		sanitizeOptions.moduleResolver = std::move(resolver);
	}

	return nzsl::Ast::Sanitize(module, sanitizeOptions);
}

void WriteFileContent(const std::filesystem::path& filePath, const void* data, std::size_t size)
{
	std::ofstream outputFile(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!outputFile)
		throw std::runtime_error(fmt::format("failed to open {}, reason: {}", filePath.generic_u8string(), std::strerror(errno)));

	if (!outputFile.write(static_cast<const char*>(data), size))
		throw std::runtime_error(fmt::format("failed to write {}, reason: {}", filePath.generic_u8string(), std::strerror(errno)));
}

void OutputFile(const cxxopts::ParseResult& options, std::filesystem::path filePath, const void* data, std::size_t size)
{
	if (options.count("header-file") > 0)
	{
		filePath.replace_extension(filePath.extension().generic_u8string() + ".h");

		const std::uint8_t* ptr = static_cast<const std::uint8_t*>(data);

		std::stringstream ss;

		bool first = true;
		for (std::size_t i = 0; i < size; ++i)
		{
			if (!first)
				ss << ',';

			ss << +ptr[i];

			first = false;
		}

		std::string headerFile = std::move(ss).str();
		WriteFileContent(filePath, headerFile.data(), headerFile.size());
	}
	else
		WriteFileContent(filePath, data, size);
}

void CompileToGLSL(const cxxopts::ParseResult& options, std::filesystem::path outputPath, const nzsl::Ast::Module& module)
{
	nzsl::ShaderWriter::States states;
	states.optimize = (options.count("optimize") > 0);

	nzsl::GlslWriter::Environment env;
	if (options.count("gl-es") > 0)
		env.glES = options["gl-es"].as<bool>();

	env.flipYPosition = (options.count("gl-flipy") > 0);
	env.remapZPosition = (options.count("gl-remapz") > 0);

	if (options.count("gl-version") > 0)
	{
		std::uint32_t minVersion = (env.glES) ? 300 : 330; //< OpenGL ES 3.0 and OpenGL 3.3 are the last versions of OpenGL
		std::uint32_t maxVersion = (env.glES) ? 320 : 460; //< OpenGL ES 3.2 and OpenGL 4.6 are the last versions of OpenGL

		std::uint32_t version = options["gl-version"].as<std::uint32_t>();
		if (version < minVersion || version > maxVersion)
			throw std::runtime_error(fmt::format("invalid GLSL version (must be between {} and {})", minVersion, maxVersion));

		env.glMajorVersion = version / 100;
		env.glMinorVersion = (version % 100) / 10;
	}

	nzsl::GlslWriter::BindingMapping bindingMapping;
	if (options.count("gl-uniformbinding") > 0)
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
			OutputFile(options, std::move(bindingOutputPath), bindingStr.data(), bindingStr.size());
		}
	}

	nzsl::GlslWriter writer;
	writer.SetEnv(env);

	std::optional<nzsl::ShaderStageType> entryType;
	if (options.count("gl-entry") > 0)
	{
		std::string entryName = options["gl-entry"].as<std::string>();
		if (entryName == "frag")
			entryType = nzsl::ShaderStageType::Fragment;
		else if (entryName == "vert")
			entryType = nzsl::ShaderStageType::Vertex;
		else
			throw std::runtime_error("unrecognized gl-entry " + entryName);
	}

	std::string glsl = writer.Generate(entryType, module, bindingMapping, states);
	
	outputPath.replace_extension("glsl");
	OutputFile(options, std::move(outputPath), glsl.data(), glsl.size());
}

void CompileToNZSL(const cxxopts::ParseResult& options, std::filesystem::path outputPath, const nzsl::Ast::Module& module)
{
	nzsl::ShaderWriter::States states;
	states.optimize = (options.count("optimize") > 0);

	nzsl::LangWriter nzslWriter;
	std::string nzsl = nzslWriter.Generate(module, states);

	outputPath.replace_extension("nzsl");
	OutputFile(options, std::move(outputPath), nzsl.data(), nzsl.size());
}

void CompileToNZSLB(const cxxopts::ParseResult& options, std::filesystem::path outputPath, nzsl::Ast::ModulePtr& module)
{
	nzsl::Serializer serializer;
	nzsl::Ast::SerializeShader(serializer, module);

	const std::vector<std::uint8_t>& data = serializer.GetData();

	outputPath.replace_extension("nzslb");
	OutputFile(options, std::move(outputPath), data.data(), data.size());
}

void CompileToSPV(const cxxopts::ParseResult& options, std::filesystem::path outputPath, const nzsl::Ast::Module& module)
{
	nzsl::ShaderWriter::States states;
	states.optimize = (options.count("optimize") > 0);

	nzsl::SpirvWriter::Environment env;
	if (options.count("spv-version"))
	{
		constexpr std::uint32_t maxVersion = nzsl::SpirvMajorVersion * 100 + nzsl::SpirvMinorVersion * 10;

		std::uint32_t version = options["spv-version"].as<std::uint32_t>();
		if (version < 100 || version > maxVersion)
			throw std::runtime_error(fmt::format("invalid SPIRV version (must be between 100 and {})", maxVersion));

		env.spvMajorVersion = version / 100;
		env.spvMinorVersion = (version % 100) / 10;
	}

	nzsl::SpirvWriter writer;
	writer.SetEnv(env);

	std::vector<std::uint32_t> spirv = writer.Generate(module, states);
	std::size_t size = spirv.size() * sizeof(std::uint32_t);

	outputPath.replace_extension("spv");
	OutputFile(options, std::move(outputPath), spirv.data(), size);
}

int main(int argc, char* argv[])
{
	cxxopts::Options cmdOptions("nzslc", "Tool for validating and compiling NZSL shaders");
	cmdOptions.add_options()
		("c,compile", "Compile input shader", cxxopts::value<std::vector<std::string>>()->implicit_value("nzslb"))
		("output-nzsl", "Output shader as NZSL to stdout")
		("header-file", "Generate an includable header file")
		("log-format", "Set log format (classic, vs)", cxxopts::value<std::string>())
		("i,input", "Input file(s)", cxxopts::value<std::string>())
		("o,output", "Output path", cxxopts::value<std::string>()->default_value("."), "path")
		("s,show", "Show informations about the shader (default)")
		("v,verbose", "Verbose output")
		("h,help", "Print usage")
		("version", "Print version");

	cmdOptions.add_options("compilation")
		("m,module", "Module file or directory", cxxopts::value<std::vector<std::string>>())
		("optimize", "Optimize shader code")
		("p,partial", "Allow partial compilation");

	cmdOptions.add_options("glsl output")
		("gl-es", "Generate GLSL ES instead of GLSL", cxxopts::value<bool>())
		("gl-version", "OpenGL version (310 being 3.1)", cxxopts::value<std::uint32_t>(), "version")
		("gl-entry", "Shader entry point (required for files having multiple entry points)", cxxopts::value<std::string>(), "[frag|vert]")
		("gl-flipy", "Add code to conditionally flip gl_Position Y value")
		("gl-remapz", "Add code to remap gl_Position Z value from [0;1] to [-1;1]")
		("gl-uniformbinding", "Add binding support (generates a .binding.json mapping file)");

	cmdOptions.add_options("spirv output")
		("spv-version", "SPIRV version (110 being 1.1)", cxxopts::value<std::uint32_t>(), "version");

	cmdOptions.parse_positional("input");
	cmdOptions.positional_help("shader path");

	try
	{
		auto options = cmdOptions.parse(argc, argv);
		if (options.count("version") > 0)
		{
			fmt::print("nzslc version {}.{}.{} using nzsl {}.{}.{}\n", 
				MajorVersion, MinorVersion, PatchVersion, 
				NZSL_VERSION_MAJOR, NZSL_VERSION_MINOR, NZSL_VERSION_PATCH);

			return EXIT_SUCCESS;
		}

		if (options.count("help") > 0)
		{
			fmt::print("{}\n", cmdOptions.help());
			return EXIT_SUCCESS;
		}

		if (options.count("input") == 0)
		{
			fmt::print(stderr, "no input file\n{}\n", cmdOptions.help());
			return EXIT_SUCCESS;
		}

		LogFormat logFormat = LogFormat::Classic;
		if (options.count("log-format") != 0)
		{
			const std::string& formatStr = options["log-format"].as<std::string>();
			if (formatStr == "vs")
				logFormat = LogFormat::VisualStudio;
			else if (formatStr != "classic")
			{
				fmt::print(stderr, "{} is not a file\n", formatStr);
				return EXIT_FAILURE;
			}
		}

		std::filesystem::path inputFilePath = options["input"].as<std::string>();

		if(!std::filesystem::exists(inputFilePath))
		{
			fmt::print(stderr, "{} does not exist\n", inputFilePath.generic_u8string());
			return EXIT_FAILURE;
		}

		if (!std::filesystem::is_regular_file(inputFilePath))
		{
			fmt::print(stderr, "{} is not a file\n", inputFilePath.generic_u8string());
			return EXIT_FAILURE;
		}

		std::filesystem::path outputPath;
		if (options.count("output") > 0)
		{
			outputPath = options["output"].as<std::string>();

			if (!std::filesystem::is_directory(outputPath) && !std::filesystem::create_directories(outputPath))
			{
				fmt::print(stderr, "failed to create {} directory\n", outputPath.generic_u8string());
				return EXIT_FAILURE;
			}
		}

		try
		{
			nzsl::Ast::ModulePtr shaderModule;
			if (inputFilePath.extension() == ".nzsl")
			{
				std::string sourceContent = ReadSourceFileContent(inputFilePath);

				std::vector<nzsl::Token> tokens = nzsl::Tokenize(sourceContent, inputFilePath.generic_u8string());

				shaderModule = nzsl::Parse(tokens);
			}
			else if (inputFilePath.extension() == ".nzslb")
			{
				std::vector<std::uint8_t> sourceContent = ReadFileContent(inputFilePath);
				nzsl::Unserializer unserializer(sourceContent.data(), sourceContent.size());

				shaderModule = nzsl::Ast::UnserializeShader(unserializer);
			}
			else
			{
				fmt::print("{} has unknown extension\n", inputFilePath.generic_u8string());
				return EXIT_FAILURE;
			}

			// Validate shader
			nzsl::Ast::ModulePtr sanitizedModule = Sanitize(options, *shaderModule);

			if (options.count("compile") > 0)
			{
				for (const std::string& outputType : options["compile"].as<std::vector<std::string>>())
				{
					// if no output path has been provided, output in the same folder as the input file
					std::filesystem::path outputFilePath = outputPath;
					if (outputFilePath.empty())
						outputFilePath = inputFilePath.parent_path();

					outputFilePath /= inputFilePath.filename();

					if (outputType == "nzsl")
						CompileToNZSL(options, std::move(outputFilePath), *sanitizedModule);
					else if (outputType == "nzslb")
						CompileToNZSLB(options, std::move(outputFilePath), sanitizedModule);
					else if (outputType == "spv")
						CompileToSPV(options, std::move(outputFilePath), *sanitizedModule);
					else if (outputType == "glsl")
						CompileToGLSL(options, std::move(outputFilePath), *sanitizedModule);
					else
					{
						fmt::print("Unknown format {}, ignoring\n");
						continue;
					}
				}
			}

			if (options.count("output-nzsl") > 0)
			{
				nzsl::LangWriter nzslWriter;
				fmt::print("{}", nzslWriter.Generate(*sanitizedModule));
			}
			
			return EXIT_SUCCESS;
		}
		catch (const nzsl::Error& error)
		{
			const nzsl::SourceLocation& errorLocation = error.GetSourceLocation();
			if (errorLocation.IsValid())
			{
				if (logFormat == LogFormat::Classic)
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
				else if (logFormat == LogFormat::VisualStudio)
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

			return EXIT_FAILURE;
		}
	}
	catch (const cxxopts::OptionException& e)
	{
		fmt::print(stderr, "{}\n{}\n", e.what(), cmdOptions.help());
		return EXIT_FAILURE;
	}
	catch (const std::exception& e)
	{
		fmt::print(stderr, "{}\n", e.what());
		return EXIT_FAILURE;
	}
}
