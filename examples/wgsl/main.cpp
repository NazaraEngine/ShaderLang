#include <NZSL/WgslWriter.hpp>
#include <NZSL/Parser.hpp>
#include <iostream>

int main()
{
	auto shader = nzsl::ParseFromFile("shader.nzsl");

	nzsl::WgslWriter wgslWriter;
	auto wgslShader = wgslWriter.Generate(*shader);

	std::cout << wgslShader << std::endl;

	return 0;
}
