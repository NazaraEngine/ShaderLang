-- Copies every other file along with generated .html (see https://github.com/xmake-io/xmake/issues/2645)
rule("wasm_files")
    on_load("wasm", function (target)
        if target:kind() == "binary" then
            target:add("installfiles", (target:targetfile():gsub("%.html", ".js")), { prefixdir = "bin" })
            target:add("installfiles", (target:targetfile():gsub("%.html", ".mem")), { prefixdir = "bin" })
            target:add("installfiles", (target:targetfile():gsub("%.html", ".mjs")), { prefixdir = "bin" })
            target:add("installfiles", (target:targetfile():gsub("%.html", ".wasm")), { prefixdir = "bin" })
		end
	end)
