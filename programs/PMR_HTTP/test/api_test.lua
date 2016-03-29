package.cpath = "../../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 2223

--local serv = "api_test"
--local serv = "api_get_trafficinfo_by_name"
local serv = "match_road"

local function http(cfg, data)
	local tcp = socket.tcp()
	if tcp == nil then
		error('load tcp failed!')
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

local function get_data(body)
	return "POST /" .. serv .. " HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


local body = "{"longitude":[121.3609102,121.3609102,121.3608877,121.3608538,121.3608135],"latitude":[31.2242363,31.2242363,31.2242292,31.224225,31.2242277],"IMEI":"481001458136617","model":"SG900","collect":true,"speed":[0,2,2,4,7],"tokenCode":"asetEn9dhx","accountID":"DmBuB45EbZ","GPSTime":[1436694833,1436694832,1436694831,1436694830,1436694829],"altitude":[-61,-61,-63,-66,-68],"direction":[-1,300,305,310,332]}"

	local data = get_data(body)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
