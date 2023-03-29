set_project("NZSL")
set_xmakever("2.6.8")

----------------------- Global options -----------------------

option("erronwarn", { default = false, description = "Fails compilation if a warning occurs"})
option("fs_watcher", { default = true, description = "Compiles with filesystem watch support (uses efsw)"})
option("unitybuild", { default = false, description = "Build the library using unity build"})
option("with_nzslc", { default = true, description = "Builds the standalone command-line compiler (nzslc)"})

----------------------- Dependencies -----------------------

add_repositories("nazara-engine-repo https://github.com/NazaraEngine/xmake-repo")
add_requires("nazarautils", "fast_float", "fmt", "frozen", "ordered_map")

if has_config("fs_watcher") then
	add_requires("efsw")
end

if has_config("with_nzslc") then
	add_requires("cxxopts >=3.1.1", "nlohmann_json")
end

----------------------- Global config -----------------------

add_rules("mode.asan", "mode.tsan", "mode.ubsan", "mode.coverage", "mode.debug", "mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

add_includedirs("include", "src")
set_languages("c89", "c++17")
set_rundir("./bin/$(plat)_$(arch)_$(mode)")
set_targetdir("./bin/$(plat)_$(arch)_$(mode)")

if has_config("erronwarn") then
	set_warnings("allextra", "error")
else
	set_warnings("allextra")
end

if is_plat("mingw", "linux") then
	add_cxxflags("-Wno-weak-vtables", {tools = "clang"})
	add_cxxflags("-Wno-subobject-linkage", {tools = "gcc"})
end

if is_plat("windows") then
	add_defines("_CRT_SECURE_NO_WARNINGS")
	add_cxxflags("/bigobj", "/permissive-", "/Zc:__cplusplus", "/Zc:externConstexpr", "/Zc:inline", "/Zc:lambda", "/Zc:preprocessor", "/Zc:referenceBinding", "/Zc:strictStrings", "/Zc:throwingNew")
	add_cxflags("/w44062") -- Enable warning: switch case not handled
	add_cxflags("/wd4251") -- Disable warning: class needs to have dll-interface to be used by clients of class blah blah blah
	add_cxflags("/wd4275") -- Disable warning: DLL-interface class 'class_1' used as base for DLL-interface blah
elseif is_plat("mingw") then
	add_cxflags("-Wa,-mbig-obj")
	add_ldflags("-Wa,-mbig-obj")
	-- Always enable at least -Os optimization with MinGW to fix "string table overflow" error
	if not is_mode("release", "releasedbg") then
		set_optimize("smallest")
	end
elseif is_plat("wasm") then
	add_rules("wasm_files")
	add_cxflags("-s DISABLE_EXCEPTION_CATCHING=0")
	add_ldflags("-s DISABLE_EXCEPTION_CATCHING=0")
	if is_mode("debug") then
		-- See https://github.com/xmake-io/xmake/issues/2646
		add_ldflags("-gsource-map")
	end
end

if is_mode("debug") then
	add_rules("debug_suffix")
elseif is_mode("asan", "tsan", "ubsan") then
	set_optimize("none") -- by default xmake will optimize asan builds
elseif is_mode("coverage") then
	if not is_plat("windows") then
		add_links("gcov")
	end
elseif is_mode("releasedbg") then
	set_fpmodels("fast")
	add_vectorexts("sse", "sse2", "sse3", "ssse3")
end

if has_config("unitybuild") then
	add_defines("NAZARA_UNITY_BUILD")
	add_rules("c++.unity_build", {uniqueid = "NAZARA_UNITY_ID", batchsize = 12})
end

----------------------- Targets -----------------------

target("nzsl")
	set_kind("$(kind)")
	set_group("Libraries")
	add_defines("NZSL_BUILD")
	add_headerfiles("include/(NZSL/**.hpp)")
	add_headerfiles("include/(NZSL/**.inl)")
	add_headerfiles("src/NZSL/**.hpp", { prefixdir = "private", install = false })
	add_headerfiles("src/NZSL/**.inl", { prefixdir = "private", install = false })
	add_files("src/NZSL/**.cpp")
	add_packages("nazarautils", { public = true })
	add_packages("fast_float", "fmt", "frozen", "ordered_map")

	if has_config("fs_watcher") then
		add_packages("efsw")
		add_defines("NZSL_EFSW")
	end

	on_load(function (target)
		if target:kind() == "static" then
			target:add("defines", "NZSL_STATIC", { public = true })
		end
	end)

if has_config("with_nzslc") then
	target("nzslc")
		set_kind("binary")
		set_group("Executables")
		add_headerfiles("src/(ShaderCompiler/**.hpp)")
		add_headerfiles("src/(ShaderCompiler/**.inl)")
		add_files("src/ShaderCompiler/**.cpp")
		add_deps("nzsl")
		add_packages("cxxopts", "fmt", "nlohmann_json")
end

includes("xmake/**.lua")
includes("examples/xmake.lua")
includes("tests/xmake.lua")
