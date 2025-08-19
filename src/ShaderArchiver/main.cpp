#include <ShaderArchiver/Archiver.hpp>
#include <fmt/format.h>

int main(int argc, char* argv[])
{
	try
	{
		cxxopts::Options cmdOptions = nzsla::Archiver::BuildOptions();

		auto options = cmdOptions.parse(argc, argv);
		if (options.count("version") > 0)
		{
			fmt::print("nzsla version {}.{}.{} using nzsl {}.{}.{}{}\n", 
				nzsla::Archiver::MajorVersion, nzsla::Archiver::MinorVersion, nzsla::Archiver::PatchVersion,
				NZSL_VERSION_MAJOR, NZSL_VERSION_MINOR, NZSL_VERSION_PATCH, NZSL_VERSION_SUFFIX);

			return EXIT_SUCCESS;
		}

		if (options.count("help") > 0)
		{
			fmt::print("{}\n", cmdOptions.help());
			return EXIT_SUCCESS;
		}

		nzsla::Archiver archiver(options);

		try
		{
			archiver.HandleParameters();
			archiver.Process();

			return EXIT_SUCCESS;
		}
		catch (const cxxopts::exceptions::exception& e)
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
