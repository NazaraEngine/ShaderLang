-- Copies every other file along with generated .html (see https://github.com/xmake-io/xmake/issues/2645)
rule("wasm_files")
    on_load("wasm", function (target)
        if target:kind() == "binary" then
            target:add("installfiles", target:targetfile():gsub("%.html", ".js"))
            target:add("installfiles", target:targetfile():gsub("%.html", ".mem"))
            target:add("installfiles", target:targetfile():gsub("%.html", ".mjs"))
            target:add("installfiles", target:targetfile():gsub("%.html", ".wasm"))
		end
	end)
