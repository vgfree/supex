package.path = "../../open/lib/?.lua;" .. package.path
package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 2222
local serv = "tsearchapi"

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

local function gen_url(T)
	local url
	for k,v in pairs(T or {}) do
		if url then
			url = string.format([[%s&%s=%s]], url, k, v)
		else
			url = k .. '=' .. v
		end
	end
	return url or ""
end

local function gen_data( path, body )
	return "POST /" .. path .. " HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	--"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


local body = {
	["tsearchapi/v2/getactiveusers"] = {
		time = "1422876000",
	},
	["tsearchapi/v2/getExtActiveUser"] = {
		time = "1422876000",
	},
	["tsearchapi/v2/getgps"] = {
                imei = "572607499291789",
		startTime = "1422876000",
		endTime = "1422876000",
	},
	["tsearchapi/v2/getExtGps"] = {
                imei = "875305743811908",
		startTime = "1422876000",
		endTime = "1422876000",
	},
	["tsearchapi/v2/geturllog"] = {
                imei = "990409213605939",
		startTime = "1422876000",
		endTime = "1422876000",
	},
        ["tsearchapi/v2/getgpssize"] = {
                imei = "572607499291789",
                startTime = "1422876000",
                endTime = "1422876000",
        },
        ["tsearchapi/v2/getCheckImport"] = {
        },
}

for key,val in pairs(body) do
	local data = gen_data( key, gen_url(val) )
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
