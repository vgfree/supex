package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4141

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
	return "POST /rtmilesMerge.json HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

local history = {
	-->> rtmiles_user_init
	'{"operate":"new_one_app","mode":"exact","tmpname":"_power_on_","appname":"rtmiles_user_init","nickname":"用户数据初始化","args":{"powerOn":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["user_data_init"]}',
	'{"operate":"fix_app_cfg","appname":"rtmiles_user_init","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"rtmiles_user_init"}',
}


local body = {
	-->[tmp]
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_driving_mileage_","remarks":"驾驶里程","args":["collect","accountID","GPSTime","speed","direction", "T_LONGDRI_MILEAGE"]}',
    '{"operate":"new_one_tmp","mode":"local","tmpname":"_driving_online_","remarks":"驾驶时长","args":["collect","accountID","GPSTime","T_ONLINE_HOUR"]}',
	-->> l_f_continuous_driving_mileage
	'{"operate":"new_one_app","mode":"local","tmpname":"_driving_mileage_","appname":"l_f_continuous_driving_mileage","nickname":"连续驾驶里程触发","args":{"T_LONGDRI_MILEAGE":[],"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":["function","is_continuous_driving_mileage_point"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_continuous_driving_mileage","config":{"bool":{"is_continuous_driving_mileage_point":{"increase":10}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_continuous_driving_mileage_remind"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_continuous_driving_mileage"}',
    -->> l_f_fatigue_driving
    '{"operate":"new_one_app","mode":"local","tmpname":"_driving_online_","appname":"l_f_fatigue_driving","nickname":"疲劳驾驶触发","args":{"T_ONLINE_HOUR":[],"collect":["boolean",true],"GPSTime":["function","drive_online_point"],"accountID":[]},"func":["app_task_forward"]}',
    '{"operate":"fix_app_cfg","appname":"l_f_fatigue_driving","config":{"bool":{"drive_online_point":{"increase":3600}},"ways":{"cause":{"trigger_type":"fixed_time","fix_num":1,"delay":300}},"work":{"app_task_forward":{"app_uri":"p2p_fatigue_driving"}}}}',
    '{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_fatigue_driving"}',
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
