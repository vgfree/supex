package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
local	MAX = 1
for i=1, MAX do

	local tcp = socket.tcp()
	tcp:setoption('keepalive', true)
	tcp:settimeout(1, 'b')  -- five second timeout

	local ret = tcp:connect("127.0.0.1", 4070)
	if ret == nil then
		return false
	end

	body = '{"collect":true, "accountID":"zdfeqE74Vi", "tokenCode":"1adada912939", "mirrtalkID":"12323sasd12", "timestamp":12345678, "longitude":[121.3977898,121.3977898,121.3977898,121.3977898,121.3977898], "latitude":[31.2202088,31.2202088,31.2202088,31.2202088,31.2202088], "direction":[150,150,150,150,150],"speed":[80,80,80,80,80],"altitude":[31,31,31,31,31],"GPSTime":[1231221,1231221,1231221,1231221,1231221]}'

	body = '{"powerOn":true, "accountID":"mdfeqE74Vi", "tokenCode":"1adada912939", "mirrtalkID":"12323sasd12", "timestamp":12345678}'

	local data = "POST /publicentry HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	"Host: 127.0.0.1:4070\r\n" ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body


	tcp:send(data)
	local result = tcp:receive("*a")
	print(result)
	tcp:close()
end
--os.execute("sleep 10")

