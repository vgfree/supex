package.cpath = "../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4070
local serv = "ACB"

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
	return "POST /" .. "publicentry" .. " HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

local function handle()
	local files = { 
			--"test/300845196358282.txt",
			--"test/656926339000351.txt",
			--"/home/xuan/data/gps_data/tmp.log",
			"/home/xuan/data/gps_data/481001458136617.txt",
			}
	for k,v in ipairs(files) do
		for line in io.lines(v) do	
			local data = get_data(line)
			print(data)
			local info = http({host = host, port = port}, data)
			print(info)
			socket.sleep(0.1)
		end
	end

end


handle()
