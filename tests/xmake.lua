option("tests")
	set_default(false)
	set_showmenu(true)
	set_description("Build unit tests")

option_end()

if has_config("tests") then
	if is_mode("asan") then
		add_defines("CATCH_CONFIG_NO_WINDOWS_SEH")
		add_defines("CATCH_CONFIG_NO_POSIX_SIGNALS")
	end

	add_requires("catch2", "spirv-tools")
	add_requires("tiny-process-library", { debug = is_mode("debug") })
	add_requires("glslang", { configs = { rtti = is_mode("ubsan") } }) -- ubsan requires rtti

	add_includedirs("src")

	target("UnitTests")
		set_kind("binary")
		set_group("Tests")
		add_headerfiles("src/**.hpp")
		add_files("src/main.cpp", { unity_ignored = true })
		add_files("src/**.cpp")

		add_deps("nzsl")
		add_packages("catch2", "fmt", "glslang", "spirv-tools", "tiny-process-library")

		if not has_config("with_nzslc") then
			remove_files("src/Tests/NzslcTests.cpp")
		end
end
