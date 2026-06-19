#!/usr/bin/env lua

local M = {}
local MODULE_NAME = "KCONFIG_STREAM"

function M.process_config(filepath)
    print("<6>[  " .. MODULE_NAME .. "  ] Parsing target configuration token streams: " .. filepath)
    local config_elements = {}
    
    local file = io.open(filepath, "r")
    if not file then return nil end
    
    for line in file:lines() do
        -- Strip spaces and bypass comments
        line = line:match("^%s*(.-)%s*$")
        if line ~= "" and not line:match("^#") then
            local key, value = line:match("^([^=]+)=(.*)$")
            if key then
                config_elements[key] = value
            end
        end
    end
    file:close()
    return config_elements
end

return M