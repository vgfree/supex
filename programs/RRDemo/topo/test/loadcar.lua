local cl = require "select_node"

local function start()
        car = cl:new()
        car:init()
        p = car:fetch()
        car:consumer(p)
        car:del()
end

start()
