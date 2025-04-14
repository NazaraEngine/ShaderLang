#include <NZSL/WgslWriter.hpp>
#include <NZSL/Parser.hpp>
#include <iostream>

int main()
{
	auto shader = nzsl::ParseFromFile("shader.nzsl");

	nzsl::WgslWriter wgslWriter;
	auto wgslShader = wgslWriter.Generate(*shader);

	std::cout << wgslShader << std::endl;

	std::cout << "\n=============================================================\n" << R"(
struct Outputs
{
    @location(0) color: vec4<f32>;
}

@fragment
fn main() -> Outputs
{
    var output: Outputs;
    output.color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    return output;
}
	)" << std::endl;

	return 0;
}
