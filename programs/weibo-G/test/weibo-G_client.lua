package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4070
local serv = "weibo_send_group_message"

local function http(cfg, data)
	local tcp = socket.tcp()
	if tcp == nil then
		error('load tcp failed')
		return false
	end
	tcp:settimeout(10000)
	local ret = tcp:connect(cfg["host"], cfg["port"])
	if ret == nil then
		error("connect failed!")
		return false
	end

	tcp:send(data)
	local result = tcp:receive("*a")
	tcp:close()
	return result
end

local function get_data( body )
	--return "POST /" .. serv .. "Apply.json HTTP/1.0\r\n" ..
	return "POST /weibo_send_group_message HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


for i = 1, 30 do
	local body = {}
	body[i] = string.format('{"GID":"111111111","level":"20","label":"%d","message":"helloworld!"}', i)
	
	data = get_data(body[i])
	print("data: \n" .. data .. "\n")

	local info = http({host = host, port = port}, data)
	print("info: \n" .. info)
end
