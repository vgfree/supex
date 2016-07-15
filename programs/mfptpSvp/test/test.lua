package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 6688

local tcp = socket.tcp()
if tcp == nil then
	error('load tcp failed')
	return false
end
tcp:settimeout(10000)
local ret = tcp:connect(host, port)
if ret == nil then
	error("connect failed!")
	return false
end
-------------------
local fd = io.open("test/data", "r")
if not fd then
	error("data err!")
	return false
end
local data = fd:read("*a")
fd:close()
-------------------

for i =1 , 1000 do
tcp:send(data)
end


--[[
local d1 = string.sub(data, 1, 34)
local d2 = string.sub(data, 35, -1)
tcp:send(d1)
os.execute("sleep 2")
tcp:send(d2)
]]--
--[[
local d1 = string.sub(data, 1, 34)
local d2 = string.sub(data, 35, -1)
tcp:send(d1 .. d2 .. d1)
os.execute("sleep 2")
tcp:send(d2)
]]--
tcp:close()
