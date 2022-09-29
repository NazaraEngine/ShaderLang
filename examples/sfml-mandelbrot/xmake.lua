if is_plat("wasm") then
	return
end

add_requires("sfml", { configs = { audio = false, network = false }})

target("sfml-mandelbrot")
	set_group("Examples")
	add_packages("sfml")
	add_deps("nzsl")
	set_rundir(".")

	add_files("main.cpp")
	add_headerfiles("mandelbrot.nzsl", { install = false })
	add_installfiles("mandelbrot.nzsl", {prefixdir = "bin"})
	add_installfiles("palette.png", {prefixdir = "bin"})
