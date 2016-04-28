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
--[[
local gsonData = '{"ERRORCODE":"0",
"RESULT":{
"accountID":"uRgZGyPykT",
"content":{"mediaList":[{"format":"jpg","index":"3","pixels":"1920*1080","size":"318215"}]},
"imei":"306627488190175",
"imsi":"460017204705594",
"mod":"XZ001",
"operationType":"3",
"remarkMsg":"拍摄成功",
"type":"20"}
}'
]]--

s:bind("tcp://*:5556")


local fdp  = io.open("./zmq-test/Craire.jpeg", "r")
local pic = fdp:read("*a")
fdp:close()
local picStr = string.format("%s", pic)

local fda = io.open("./zmq-test/testsound.amr", "r")
local voice = fda:read("*a")
fda:close()
local voiceData = string.format("%s", voice)

test_tab = {
  head = 'feedback?imei=123456789123456',
  gson = 'gsonData',
  picture = picStr,
  voice = voiceData,
}


s:send_table(test_tab)
--s:send(test_tab['voice'])

s:close()
print("S has closed")
ctx:term()
