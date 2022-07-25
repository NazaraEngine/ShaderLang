add_requires("sfml", { configs = { audio = false, network = false }})

target("sfml-mandelbrot")
	set_group("Examples")
	add_files("main.cpp")
	add_packages("sfml")
	add_deps("nzsl")
	set_rundir(".")
