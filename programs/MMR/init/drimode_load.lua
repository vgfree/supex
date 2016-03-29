package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 9999

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
	return "POST /drimodeMerge.json HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

local history = {
	-->> drimode_user_init
	'{"operate":"new_one_app","mode":"exact","tmpname":"_power_on_","appname":"drimode_user_init","nickname":"用户数据初始化","args":{"powerOn":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["user_data_init"]}',
	'{"operate":"fix_app_cfg","appname":"drimode_user_init","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"drimode_user_init"}',
}


local body = {
}

--local body = {
--->>> test [auto]
--	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_test"}',
--}

for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
