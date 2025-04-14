target("wgsl-example")
	set_group("Examples")
	add_deps("nzsl")
	set_rundir(".")

	add_files("main.cpp")
	add_headerfiles("shader.nzsl", { install = false })
	add_installfiles("shader.nzsl", {prefixdir = "bin"})
