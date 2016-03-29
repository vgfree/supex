package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4040

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
	return "POST /drisampMerge.json HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

local history = {
	-->> drisamp_user_init
	'{"operate":"new_one_app","mode":"exact","tmpname":"_power_on_","appname":"drisamp_user_init","nickname":"用户数据初始化","args":{"powerOn":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["user_data_init"]}',
	'{"operate":"fix_app_cfg","appname":"drisamp_user_init","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"drisamp_user_init"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_road_traffic"}',
	'{"operate":"fix_app_cfg","appname":"l_f_road_traffic","config":{"bool":{"check_time_is_between_in":{"time_start":22.5*60*60,"time_end":6*60*60}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_for_ward":{"app_uri":"p2p_traffic_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_road_traffic"}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_fetch_ambitus_tweet"}',--TODO
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_driving_online_","remarks":"驾驶时长","args":["collect","accountID","GPSTime"]}',
	-->> l_f_fatigue_driving
	'{"operate":"new_one_app","mode":"local","tmpname":"_driving_online_","appname":"l_f_fatigue_driving","nickname":"疲劳驾驶触发","args":{"collect":["boolean",true],"GPSTime":["function","drive_online_point"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_fatigue_driving","config":{"bool":{"drive_online_point":{"increase":3600}},"ways":{"cause":{"trigger_type":"fixed_time","fix_num":1,"delay":300}},"work":{"app_task_forward":{"app_uri":"p2p_fatigue_driving"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_fatigue_driving"}',
}


-- [ctl]	open close insmod rmmod
local body = {
	-->[tmp]
	'{"operate":"new_one_tmp","mode":"exact","tmpname":"_first_boot_","remarks":"历史上第一次开机","args":["powerOn","accountID","IMEI","tokenCode","model"]}',
	'{"operate":"new_one_tmp","mode":"exact","tmpname":"_power_on_","remarks":"用户开机","args":["powerOn","accountID","IMEI","tokenCode","model"]}',
	'{"operate":"new_one_tmp","mode":"exact","tmpname":"_power_off_","remarks":"用户关机","args":["powerOff","accountID","IMEI","tokenCode","model"]}',
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_over_speed_","remarks":"超速驾驶","args":["collect","accountID","speed","position"]}',
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_home_offsite_","remarks":"异地","args":["collect","accountID","position"]}',
	'{"operate":"new_one_tmp","mode":"exact","tmpname":"_oneday_boot_","remarks":"今日第一次开机","args":["powerOn","accountID","IMEI","tokenCode","model"]}',
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_road_traffic_","remarks":"路况","args":["collect","accountID","speed","direction"]}',
	'{"operate":"new_one_tmp","mode":"exact","tmpname":"_scene_version_1_","remarks":"系统更新提醒","args":["powerOn","accountID","IMEI","tokenCode","model"]}',

	-->> e_f_first_boot
	'{"operate":"new_one_app","mode":"exact","tmpname":"_first_boot_","appname":"e_f_first_boot","nickname":"历史第一次开机情景","args":{"powerOn":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"e_f_first_boot","config":{"bool":{},"ways":{"cause":{"trigger_type":"once_life","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_newbie_guide","scene_forward":{"app_type":8}}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_f_first_boot"}',
	-->>e_f_power_on
	'{"operate":"new_one_app","mode":"exact","tmpname":"_power_on_","appname":"e_f_power_on","nickname":"开机","args":{"powerOn":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"e_f_power_on","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_power_on","scene_forward":{"app_type":7}}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_f_power_on"}',
	-->>e_f_power_off
	'{"operate":"new_one_app","mode":"exact","tmpname":"_power_off_","appname":"e_f_power_off","nickname":"用户关机","args":{"powerOff":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"e_f_power_off","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_power_off","scene_forward":{"app_type":6}}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_f_power_off"}',
	-->> l_f_over_speed
	'{"operate":"new_one_app","mode":"local","tmpname":"_over_speed_","appname":"l_f_over_speed","nickname":"超速提醒","args":{"collect":["boolean",true],"speed":["function","is_over_speed"],"position":[],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_over_speed","config":{"bool":{"is_over_speed":{"speed":125}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_over_speed","scene_forward":{"app_type":5}}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_over_speed"}',
	-->> l_f_home_offsite
	'{"operate":"new_one_app","mode":"local","tmpname":"_home_offsite_","appname":"l_f_home_offsite","nickname":"异地触发","args":{"collect":["boolean",true],"position":["function","is_off_site"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_home_offsite","config":{"bool":{},"ways":{"cause":{"trigger_type":"fixed_time","fix_num":1,"delay":180}},"work":{"app_task_forward":{"app_uri":"p2p_offsite_remind","scene_forward":{"app_type":4}}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_home_offsite"}',
	-->> e_f_oneday_boot
	'{"operate":"new_one_app","mode":"exact","tmpname":"_oneday_boot_","appname":"e_f_oneday_boot","nickname":"今日第一次开机情景","args":{"powerOn":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"e_f_oneday_boot","config":{"bool":{},"ways":{"cause":{"trigger_type":"one_day","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_solarcalendar","scene_forward":{"app_type":3}}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_f_oneday_boot"}',
	-->> l_f_road_traffic
	'{"operate":"new_one_app","mode":"local","tmpname":"_road_traffic_","appname":"l_f_road_traffic","nickname":"路况","args":{"collect":["boolean",true],"position":["function","is_road_traffic"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_road_traffic","config":{"bool":{},"ways":{"cause":{"trigger_type":"fixed_time","every_time":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_road_traffic","scene_forward":{"app_type":2}}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_road_traffic"}',
	-->e_f_scene_version_1
	 '{"operate":"new_one_app","mode":"exact","tmpname":"_scene_version_1_","appname":"e_f_scene_version_1","nickname":"系统更新提醒","args":{"powerOn":["boolean",true],"accountID":[],"IMEI":[],"tokenCode":[],"model":[]},"func":["app_task_forward"]}',
        '{"operate":"fix_app_cfg","appname":"e_f_scene_version_1","config":{"bool":{},"ways":{"cause":{"trigger_type":"once_life","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_system_update","scene_forward":{"app_type":1}}}}}',
        '{"operate":"ctl_one_app","status":"rmmod","mode":"exact","appname":"e_f_scene_version_1"}',
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
