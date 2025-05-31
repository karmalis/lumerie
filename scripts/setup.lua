jit.on()

-- Init message

io.write("Initialising JIT:")
io.write(jit.version)
io.write("\n")

-- Init package paths
local setup_path_info = debug.getinfo(1, "S")
local setup_full_path = setup_path_info.source:sub(2)
local SCRIPT_DIR = setup_full_path:match("(.*[/\\])")
if SCRIPT_DIR == nil then SCRIPT_DIR = "" end

io.write("Initialising lua within " .. SCRIPT_DIR .. "\n")

package.path = SCRIPT_DIR .. "?.lua;" .. package.path

-- Init modules

io.write("Initialising modules\n")
local init = require("init")
print(init.modules[1])

-- Init config
io.write("Loading configuration\n")
local config = require("config")
print(config.some_config)

-- Finalise
io.write("init sequence complete\n")
