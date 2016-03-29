local only      = require('only')
local supex     = require('supex')
local utils     = require('utils')
local link      = require('link')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local weibo	= require('weibo')
local http_short  = require("http_short_api")

module('p2p_power_on', package.seeall)


--功	能：用户上线提示,暂时停用
function work_to_poweron_online( )
	local tab = {
		accountID = supex.get_our_body_table()["accountID"],
		powerOn   = tostring(supex.get_our_body_table()["powerOn"]),
		tokenCode = supex.get_our_body_table()["tokenCode"],
		model     = supex.get_our_body_table()["model"],
	}
	local body_data = cjson.encode(tab)
	local app_server = link["OWN_DIED"]["http"]["appcenterApply.json"]
	local data = utils.compose_http_json_request(app_server, "appcenterApply.json?app_name=a_app_report_user_online", nil, body_data)
	local ok_status = http_short.http(app_server, data, false)
	if not ok_status then
		only.log('D',string.format("post data %s:%s failed!",app_server.host,app_server.port))
	end
end


--功	能：用户开机,提示3句话日志
function work_to_poweron_standardapp( )
	local accountID = supex.get_our_body_table()["accountID"]
	local imei = supex.get_our_body_table()["IMEI"]
	----没有绑定accountID的终端,不触发
	if (not imei) or (not accountID) or (#accountID == 15) then 
		only.log('W', "accountID:%s is imei ,jmp work_to_poweron_standardapp-->--", accountID)
		return false
	end
	local ok, travelID = redis_api.cmd('private', accountID, 'get', imei .. ":travelID")
	if not ok or travelID == nil then
		only.log('E', "get IMEI:travelID failed,accountID:%s IMEI:%s ", accountID, imei)
		return false
	end
	----- 没有订阅----
	--[[
	if not weibo.check_subscribed_msg(accountID, weibo["DRI_APP_STANDARDAPP"]) then
		only.log('D',string.format("accountID:%s is imei ,not subscribed  work_to_poweron_standardapp-->--",accountID))
		return false
	end
	]]--

	local tab = {
		accountID = accountID,
		powerOn   = tostring(supex.get_our_body_table()["powerOn"]),
		model     = supex.get_our_body_table()["model"],
		travelID  = travelID
	}

	local body_data = utils.table_to_kv(tab)

	local app_server = link["OWN_DIED"]["http"]["customizationapp/poweronDiaryStart"]
	local data = utils.post_data("customizationapp/poweronDiaryStart", app_server, body_data)
	local ok = http_short.http(app_server, data, false)
	if not ok then
		only.log('E', "post data %s:%s failed!", app_server["host"], app_server["port"])
		return false
	end
	only.log('D', data)
	return true
end

--功	能：开机微薄
local function power_on_weibo()
	only.log("I", "power_on_weibo working... ")
	--[[
	if not weibo.check_subscribed_msg(accountID, weibo["DRI_APP_POWER_ON_WEIBO"]) then
		only.log('D','开机微博，被客户禁止!')
		return false
	end
	]]--

	-- 判断当前时间在哪一个时间段
	local idx = 1
	local currrent = os.date("%X", os.time())
	local time = {
		{min = '05:00:00', max = '09:30:00'},
		{min = '09:30:01', max = '11:30:00'},
		{min = '11:30:01', max = '14:00:00'},
		{min = '14:00:01', max = '17:30:00'},
		{min = '17:30:01', max = '20:00:00'},
		{min = '20:00:01', max = '23:00:00'},
		{min = '23:00:01', max = '23:59:59'},
		{min = '00:00:00', max = '04:59:59'},
	}
	for i = 1 , #time do
		if currrent > time[i]['min'] and currrent < time[i]['max']  then
			idx = i
			break
		end
	end
	local idx_type = {"one","two","three","four","five","six","seven","seven"}
	local idx_file = {
		{"101","102","103","104","105","106","107","108"},
		{"109","110","111","112"}, 
		{"113","114","115","116"}, 
		{"117","118","119"}, 
		{"120","121","122","123","124"}, 
		{"125","126","127","128"}, 
		{"129","130","131","132","133","134"}, 
		{"129","130","131","132","133","134"}, 
	}
	local id = (os.time() % #idx_file[idx]) + 1
	local str = idx_type[idx] .. "/" .. idx_file[idx][id]
	WORK_FUNC_LIST["half_url_power_on_unknow_str_send_weibo"]("power_on_weibo", str)
end

-- 功	能：播报天气预报
local function weather_send()
        if not (weibo.user_control('p2p_weather_forcast')) then
        	return false
        end
	local sleepTime = (os.time()+ 8*60*60 + 10) %(60 *60 *24)
	local body = string.format(
	'{"APPLY":"crzptY_weather_forcast","MODE":"insert",' ..
	'"EXEC":"make","ARGS":{"data":%s},"TIME":%d,"LIVE":1}',
	supex.get_our_body_data(), sleepTime)

	local path = "crzptY_weather_forcast"
	local app_srv = link["OWN_DIED"]["http"][ path ]
	local app_uri = "crazypointApply.json"
	local data = utils.compose_http_json_request(app_srv, app_uri, nil, body)
	--supex.http(app_srv["host"], app_srv["port"], data, #data)
	http_short.http(app_srv,data,false)
end



--功	能：开机触发播报的内容
function handle()
	--1 开机微薄
	pcall(power_on_weibo)

	--2 用户开机提醒
	--pcall(work_to_poweron_online)

	--3 用户开机,提示3句话日志
	pcall(work_to_poweron_standardapp)

	-- 4 播报天气预报
	pcall(weather_send)
end

