-- 测试文件需要重新编写
package.cpath = "../lib/?.so;" .. package.cpath
local interface = require('luaflif')

local function convert()
--[[        local INPUT_FILE1 = "../../image/miku.flif"
        local OUTPUT_FILE1 = "../../image/miku_f_p.png"
        
        interface.lua_flif2png(INPUT_FILE1, OUTPUT_FILE1)

        local INPUT_FILE2 = "../../image/asuka.png"
        local OUTPUT_FILE2 = "../../image/asuka_p_f.flif"
        
        interface.lua_png2flif(INPUT_FILE2, OUTPUT_FILE2)
--]]
end

do 
        convert();
        
end
