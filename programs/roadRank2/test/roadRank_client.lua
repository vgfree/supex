package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
local	MAX = 1
for i=1, MAX do

	local tcp = socket.tcp()
	tcp:setoption('keepalive', true)
	tcp:settimeout(1, 'b')  -- five second timeout

	local ret = tcp:connect("127.0.0.1", 8222)
	if ret == nil then
		return false
	end

        body = '{"longitude":[127.3405357,127.3405073,127.3404813,127.3404482,127.3404177],"latitude":[43.7227393,43.7226935,43.7226432,43.722578,43.7225062],"mirrtalkID":"409101916615134","collect":true,"speed":[21,22,27,29,27],"tokenCode":"mFlhuu2UlY","accountID":"AmukwhmvmU","GPSTime":[1435988723,1435988722,1435988721,1435988720,1435988719],"altitude":[283,283,284,284,284],"direction":[24,22,18,16,14]}'
	local data = "POST /publicentry HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	"Host: 127.0.0.1:8222\r\n" ..
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

