#include <ShaderCompiler/Compiler.hpp>
#include <fmt/format.h>

int main(int argc, char* argv[])
{
	try
	{
		cxxopts::Options cmdOptions = nzslc::Compiler::BuildOptions();

		auto options = cmdOptions.parse(argc, argv);
		if (options.count("version") > 0)
		{
			fmt::print("nzslc version {}.{}.{} using nzsl {}.{}.{}\n", 
				nzslc::Compiler::MajorVersion, nzslc::Compiler::MinorVersion, nzslc::Compiler::PatchVersion,
				NZSL_VERSION_MAJOR, NZSL_VERSION_MINOR, NZSL_VERSION_PATCH);

			return EXIT_SUCCESS;
		}

		if (options.count("help") > 0)
		{
			fmt::print("{}\n", cmdOptions.help());
			return EXIT_SUCCESS;
		}

		nzslc::Compiler compiler(options);

		try
		{
			compiler.HandleParameters();
			compiler.Process();

			return EXIT_SUCCESS;
		}
		catch (const nzsl::Error& error)
		{
			compiler.PrintError(error);
			return EXIT_FAILURE;
		}
		catch (const cxxopts::OptionException& e)
		{
			fmt::print(stderr, "{}\n{}\n", e.what(), cmdOptions.help());
			return EXIT_FAILURE;
		}
	}
	catch (const std::exception& e)
	{
		fmt::print(stderr, "{}\n", e.what());
		return EXIT_FAILURE;
	}
}
