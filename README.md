[![build status](https://github.com/NazaraEngine/ShaderLang/actions/workflows/build.yml/badge.svg)](https://github.com/NazaraEngine/ShaderLang/actions/workflows/build.yml)  
[![codecov](https://codecov.io/gh/NazaraEngine/ShaderLang/branch/main/graph/badge.svg?token=VE71FIB616)](https://codecov.io/gh/NazaraEngine/ShaderLang)

# Nazara Shading Language (NZSL)

NZSL is a shader language inspired by Rust and C++ which compiles to GLSL or SPIR-V (without depending on SPIRV-Cross).

### Why a new shader language?

I first designed NZSL for my [game engine](https://github.com/NazaraEngine/NazaraEngine) (hence the name) to help supporting Vulkan and OpenGL (ES) renderers with a unique shader language (without requiring #ifdef garbage).

I grew tired of the C-like syntax of both HLSL and GLSL, and though we can do a lot better and give it more features (like compile-time options, a modern import/module system instead of 40 year olds #include, etc.).

Some people got interested in the language to use in their own projects without requiring the game engine, so here it is!

## What does it look like?

Simple example:
```nzsl
[nzsl_version("1.1")]
module TextureBlit;

import VertOut, VertexShader from Engine.FullscreenVertex;

external
{
    [binding(0)] texture: sampler2D[f32]
}

struct FragOut
{
    [location(0)] color: vec4[f32]
}

[entry(frag)]
fn main(input: VertOut) -> FragOut
{
    let texColor = texture.Sample(input.uv);

    let output: FragOut;
    output.color = texColor;

    return output;
}
```

## How to use

You can find precompiled binaries in the [releases](https://github.com/NazaraEngine/ShaderLang/releases).

NZSL is designed to be embedded in a game engine / game / graphics application that uses GLSL / SPIR-V for its shaders.

You can use it to generate GLSL, GLSL ES and SPIR-V in two non-exclusive ways:

1) Using the offline NZSL compiler (nzslc) ahead of time, in a way similar to glslang or glslc today.
2) Use NZSL as a library in your application to compile shaders in a dynamic way, just as they're needed (which can be used to benefit from supported extensions to improve generation).

### Offline compilation

There are two binary tools you can use:
- **nzslc**: shader compiler, for compiling nzsl files to binary nzsl or directly to GLSL/SPIR-V.
- **nzsla**: shader archiver, store and compress all your compiled shaders in a single file.

**nzslc example usage:**

- Validating shader: `nzslc file.nzsl`
- Compile a shader to GLSL: `nzsl --compile=glsl file.nzsl`
- Compile a shader to SPIR-V: `nzsl --compile=spv file.nzsl`
- Compile a shader using modules to both GLSL and SPIR-V header includable version: `nzsl --module module_file.nzsl --module module_folder/ --compile=glsl-header,spv-header file.nzsl`

Run `nzslc -h` to see all supported options.

**nzsla example usage:**

- Create an archive: `nzsla --archive -o shaders.nzsla shader1.nzsl shader2.nzslb`
- Create a compressed archive (lz4hc by default): `nzsla --archive --compress -o shaders.nzsla shader1.nzsl shader2.nzslb`
- View the content of an archive: `nzsla shaders.nzsla`

Run `nzsla -h` to see all supported options.

### Use it as a library

**Example usage:**

```cpp
#include <NZSL/Parser.hpp>
#include <NZSL/GlslWriter.hpp>
#include <NZSL/SpirvWriter.hpp>

int main()
{
    nzsl::Ast::ModulePtr shaderAst = nzsl::ParseFromFile("pbr.nzsl");

    nzsl::SpirvWriter spirvWriter;
    std::vector<std::uint32_t> spirv = spirvWriter.Generate(shaderAst);
    // spirv contains SPIR-V bytecode that can be directly given to Vulkan

    nzsl::GlslWriter glslWriter;
    nzsl::GlslWriter::Output output = glslWriter.Generate(shaderAst);
    // output.code contains GLSL that can directly be used by OpenGL
}
```

The library contains a lot of options to customize the generation process (target SPIR-V/GLSL version, GLSL ES, gl_Position.y flipping, gl_Position.z remapping to match Vulkan semantics, supported OpenGL extensions, etc.).

## Integration

You can find precompiled binaries in the [releases](https://github.com/NazaraEngine/ShaderLang/releases).

You can easily integrate NZSL as a library in your project if you're using [xmake](https://xmake.io) as a build system (try it, it's amazing!) with:

```lua
add_requires("nzsl")
```

Simply add the `nzsl` package to your target and xmake will download/compile the latest version.

If you're using CMake, you can also check out [xrepo-cmake](https://github.com/xmake-io/xrepo-cmake) to integrate it.

# Commonly asked questions

## Where can I find the language specification?

At this early point, there's no BNF notation or syntax document, but it's something I'd like to describe on the project [wiki](https://github.com/NazaraEngine/ShaderLang/wiki) soon.

## Why does it look so similar to [WGSL](https://www.w3.org/TR/WGSL/)?

The language syntax is mainly based on C++ and Rust, the 
resemblance with WGSL is accidental as I discovered it after the [first working version of NZSL](https://www.reddit.com/r/vulkan/comments/mpeglj/finally_managed_to_make_my_own_shading_language/).

## Why not just use SPIR-V and SPIRV-Cross?

[SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) is interesting but it requires you to generate SPIR-V offline first, and thus use GLSL/HLSL which I grew out of love after a few years.

At one of my previous working place we were using huge HLSL-derived shaders with thousands of lines relying on #include and #pragma once, and it's very impractical and a pain to debug. The more translation layers you add, the more information you lose.

NZSL is designed to be small, fast and easy to debug, for example NZSL to GLSL retains a lot of the source code information which could be lost during SSA (SPIR-V) translation, even with debug symbols enabled.

## Is there a DXIL/WGSL backend?

Not yet, as I don't target Direct3D or WebGPU yet.

DXIL is not very different from SPIR-V and WGSL looks a lot like NZSL so it should be quite easy to add, though.

See [this issue](https://github.com/NazaraEngine/ShaderLang/issues/13) for WGSL.

## Are there limitations?

Unfortunately yes, NZSL is in its early stage and is currently only capable of regular operations in compute, fragment and vertex stages, not all intrinsics are supported, and more.

The reason isn't that it's complicated to add, but that I didn't need it yet. Most feature can be added quite fast so do not hesitate to [open an issue](https://github.com/NazaraEngine/ShaderLang/issues) and/or check the [roadmap](https://github.com/NazaraEngine/ShaderLang/projects/1).

## I don't like the syntax

That's unfortunate, however I'm open to suggestions to improve NZSL syntax, even for breaking changes (one feature of NZSL modules is to allow breaking changes without breaking all shaders), so do not hesitate to [open an issue](https://github.com/NazaraEngine/ShaderLang/issues).

## I want to contribute

Welcome aboard! Do not hesitate to check the [issues](https://github.com/NazaraEngine/ShaderLang/issues) for "good first issues" which should be quite easy to do.

You can also check the [roadmap](https://github.com/NazaraEngine/ShaderLang/projects/1).

Simply fork the project, make some changes and open a [pull request](https://github.com/NazaraEngine/ShaderLang/pulls) once you're ready to merge your work!
