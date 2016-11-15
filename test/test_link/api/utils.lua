--
--
--

local socket = require("socket")

module("utils", package.seeall)

--[[=================================RANDOM FUNCTION=======================================]]--
local function get_random_seed()
        local t = string.format("%f", socket.gettime())
        local st = string.sub(t, string.find(t, "%.") + 1, -1) 
        return tonumber(string.reverse(st))
end
math.randomseed(get_random_seed()) -- set random seed
