option("tests", { description = "Build unit tests", default = false })

add_repositories("nazara-engine-repo https://github.com/NazaraEngine/xmake-repo")

if has_config("tests") then
	if has_config("asan") then
		add_defines("CATCH_CONFIG_NO_WINDOWS_SEH")
		add_defines("CATCH_CONFIG_NO_POSIX_SIGNALS")
	end

	add_requires("catch2 3", "wgsl-validator", "spirv-tools", "tiny-process-library")
	add_requires("glslang", { configs = { rtti = has_config("ubsan") } }) -- ubsan requires rtti

	add_includedirs("src")

	target("UnitTests", function ()
		set_kind("binary")
		set_group("Tests")
		add_headerfiles("src/**.hpp")
		add_files("src/main.cpp", { unity_ignored = true })
		add_files("src/**.cpp")

		add_deps("nzsl")
		add_packages("catch2", "glslang", "wgsl-validator", "spirv-tools")

		if has_config("with_nzslc") then
			add_deps("nzslc", { links = {} })
			add_packages("fmt", "tiny-process-library")
		else
			remove_files("src/Tests/NzslcTests.cpp")
		end

		if has_config("with_nzsla") then
			add_deps("nzsla", { links = {} })
			add_packages("fmt", "tiny-process-library")
		else
			remove_files("src/Tests/NzslaTests.cpp")
		end

		if not has_config("with_nzsla", "with_nzslc") then
			remove_headerfiles("src/Tests/ToolTests.hpp")
			remove_files("src/Tests/ToolTests.cpp")
		end
	end)
end
