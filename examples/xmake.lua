option("examples")
	set_default(false)
	set_showmenu(true)
	set_description("Build examples")
option_end()

if has_config("examples") then
    includes("*/xmake.lua")
end
