#!/usr/bin/env lua

local MODULE_NAME = "GDB_BRIDGE"
print("<6>[  " .. MODULE_NAME .. "  ] Generating inline execution parameters for remote GDB targets...")

local target_port = "1234"
local symbols_obj = "build/vmlinux"

local gdb_script_content = [[
# Auto-generated GDB sequence for BlueOS debugging tracking
file ]] .. symbols_obj .. [[

target remote localhost:]] .. target_port .. [[

set disassembly-flavor intel
layout src
breakpoint main
continue
]]

local f = io.open(".gdbinit", "w")
if f then
    f:write(gdb_script_content)
    f:close()
    print("<6>[  " .. MODULE_NAME .. "  ] .gdbinit session locked. Launch 'gdb' to hook active execution frames.")
else
    print("<3>[  " .. MODULE_NAME .. "  ] Error writing initialization configuration profiles.")
end