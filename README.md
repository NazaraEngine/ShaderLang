Platform | Build Status
------------ | -------------
Windows | [![Windows build status](https://github.com/NazaraEngine/ShaderLang/actions/workflows/windows-build.yml/badge.svg)](https://github.com/NazaraEngine/ShaderLang/actions/workflows/windows-build.yml)
MSYS2 (MinGW64) | [![Windows build status](https://github.com/NazaraEngine/ShaderLang/actions/workflows/msys2-build.yml/badge.svg)](https://github.com/NazaraEngine/ShaderLang/actions/workflows/msys2-build.yml)
Linux | [![Linux build status](https://github.com/NazaraEngine/ShaderLang/actions/workflows/linux-build.yml/badge.svg)](https://github.com/NazaraEngine/ShaderLang/actions/workflows/linux-build.yml)
macOS | [![macOS build status](https://github.com/NazaraEngine/ShaderLang/actions/workflows/macos-build.yml/badge.svg)](https://github.com/NazaraEngine/ShaderLang/actions/workflows/macos-build.yml)
Coverage | [![codecov](https://codecov.io/gh/NazaraEngine/ShaderLang/branch/main/graph/badge.svg?token=VE71FIB616)](https://codecov.io/gh/NazaraEngine/ShaderLang)

# Nazara Shading Language (NZSL)

NZSL is a shader language inspired by Rust and C++ which compiles to GLSL or SPIRV (without depending on SPIRV-Cross).

Simple example:
```nzsl
[nzsl_version("1.0")]
module TextureBlit;

external
{
    [binding(0)] texture: sampler2D[f32]
}

struct VertIn
{
    [location(0)] position: vec2[f32],
    [location(1)] uv: vec2[f32]
}

struct VertOut
{
    [builtin(position)] position: vec4[f32],
    [location(0)] uv: vec2[f32]
}

[entry(vert)]
fn main(vertIn: VertIn) -> VertOut
{
    let output: VertOut;
    output.position = vec4[f32](vertIn.position, 0.0, 1.0);
    output.uv = vertIn.uv;

    return output;
}

struct FragOut
{
    [location(0)] color: vec4[f32]
}

[entry(frag)]
fn main(input: VertOut) -> FragOut
{
    let output: FragOut;
    output.color = texture.Sample(input.uv);

    return output;
}
```
