local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../open/lib/?.lua;",
        "../../open/apply/?.lua;",
        "../../open/spxonly/?.lua;",
        "../../open/linkup/?.lua;",
        "../../open/public/?.lua;",

        "open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
require("zmq")

local ctx = zmq.init(1)
local s = ctx:socket(zmq.PUSH)


s:bind("tcp://*:5556")


local fd  = io.open("Craire.jpeg", "r")
local pic = fd:read("*a")
fd:close()

local picStr = string.format("%s", pic)
s:send(picStr)

s:close()
print("S has closed")
ctx:term()
