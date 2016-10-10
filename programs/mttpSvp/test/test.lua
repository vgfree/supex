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

tcp:send(data)
--local result = tcp:receive("*a")
os.execute("sleep 1")
tcp:close()
print(result)
