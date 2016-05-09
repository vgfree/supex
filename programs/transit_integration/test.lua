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
local s = ctx:socket(zmq.PULL)

s:connect("tcp://localhost:1020")
while true do
        local data = s:recv_table()
        for i=1,#data do
                print(#data[i])
        end
end


s:close()
ctx:term()
