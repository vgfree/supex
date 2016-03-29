-- auth: baoxue
-- time: 2014.04.27

local redis_api 	= require('redis_pool_api')
local APP_CONFIG_LIST	= require('CONFIG_LIST')
local APP_POOL 		= require('pool')
local supex 		= require('supex')
local scene 		= require('scene')
local only		= require('only')
local cachekv 		= require('cachekv')
local luakv_api 	= require('luakv_pool_api')
local road_traffic      = require('road_traffic_handle')

module('judge', package.seeall)

local function is_of_site_counter(counter,app_name)
	local accountID = supex.get_our_body_table()["accountID"]

	local ok, home_city_table = redis_api.cmd("mapdata", accountID, 'Hgetall', accountID .. ':userCity')
	if not ok or not home_city_table then
		only.log("E", "operating redis is fail,key is :userCity!")
		return false
	end
	local ok, city_code = cachekv.pull('private', accountID, 'get', accountID .. ':cityCode')
	if not ok then
		only.log('E',"operating redis is fail, key is :cityCode")
		return false
	end
	local homeCityCode = home_city_table["startCityCode"]
	if tonumber(city_code) == tonumber(homeCityCode) then
		only.log('D', 'cityCode homeCityCode is same!')
		redis_api.cmd('owner', accountID, 'del', accountID .. ":identification")
		return false
	end
	local ok,value = redis_api.cmd('owner', accountID, 'get', accountID .. ":identification")
	if tonumber(value) == tonumber(city_code) then
		return false
	else
		if counter == 3 then
			redis_api.cmd('owner', accountID, 'set', accountID .. ":identification" , city_code)
			luakv_api.cmd("owner",accountID,"del",accountID .. ":counter")
		else
			counter = counter + 1
			luakv_api.cmd("owner",accountID,"set",accountID .. ":counter",counter)
			return false
		end
	end
	
	scene.push( app_name, { ["offsiteRemindCityCode"] = city_code } )
	
	return true
end




--功	能：判断是否在异地
--参	数：app_name, 应用名称
--返 回 值：符合要求返回true,不符合返回false 
--触发逻辑：归属地到异地只触发一次，异地再到异地也只触发一次
function is_off_site(app_name)
	local accountID = supex.get_our_body_table()["accountID"]
	local ok,counter = luakv_api.cmd("owner",accountID,"get",accountID .. ":counter")
	if not ok then return false end
	
	if counter then
		is_of_site_counter(counter,app_name)
	else
		is_of_site_counter(1,app_name)
	end
end



function check_time_is_between_in(app_name)
	local gps_time = supex.get_our_body_table()["GPSTime"][1]
	------check time is between_in ( time_start,time_end )
	local time_start = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["check_time_is_between_in"]["time_start"]
	local time_end = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["check_time_is_between_in"]["time_end"]

	local get_gps_time = ( tonumber(gps_time) + 8*60*60 ) % ( 24*60*60 )


	if time_start < time_end then
		----if between 10 ~ 14 
		----then time_start = 10 * 60 * 60 , time_end = 14 * 60 * 60 
		---- 10 < gps_time and gps_time < 14 
		if time_start < get_gps_time   and  get_gps_time < time_end  then
			return true
		else
			return false
		end
	elseif time_start > time_end then
		----if between 23 ~ 4  jmp new day 
		----then time_start = 23 * 60 * 60  ,time_end = 4 *60 * 60
		---- 23 < gps_time or  gps_time < 4 
		if time_start < get_gps_time  or get_gps_time < time_end then
			return true
		else
			return false
		end
	end
	return false
end




function drive_online_point_init(accountID)
	luakv_api.cmd('owner', accountID, 'set', accountID .. ':sysInternalConfigTimestamp', os.time())
end


--函	数：get_current_online_time
--功	能：计算当前用户的在线时间
--参	数：accountID,用户ID
--返 回 值：成功返回在线时间值，否则返回0
local function get_current_online_time(accountID)
	local ok, cfg_time = luakv_api.cmd('owner', accountID, 'get', accountID .. ':sysInternalConfigTimestamp')

	if ok and cfg_time then
		local timestamp = os.time()
		local value = timestamp - cfg_time
		return value
	end
	return 0
end

--keyName:isOverSpeedStartTime   place:owner       des:上次超速时间      action:w
--keyName:isOverSpeedPointCount  place:owner       des:记录超速点        action:w
function is_over_speed_init(accountID)
	luakv_api.cmd('owner', accountID, 'del', accountID .. ':isOverSpeedStartTime')
	luakv_api.cmd('owner', accountID, 'set', accountID .. ':isOverSpeedPointCount', -1)
end


--功	能：判断是否超速
--参	数：app_name:应用名称
--返 回 值：符合要求返回true,不符合返回false 
function is_over_speed(app_name)
	local direction		= supex.get_our_body_table()["direction"] and supex.get_our_body_table()["direction"][1] or -1
	local increase		= APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["is_over_speed"]["increase"]
	increase		= increase and tonumber(increase) or 120
	local accountID		= supex.get_our_body_table()["accountID"]
	local all		= #supex.get_our_body_table()["longitude"]
	local is_over_speed_start_time_key	= accountID .. ":isOverSpeedStartTime"
	local is_over_speed_point_count_key	= accountID .. ":isOverSpeedPointCount"
	-->> check speed
	local speed				= supex.get_our_body_table()["speed"][1]
	if not speed or tonumber(speed) == 0 then
		return false
	end
	-->> check lon lat
	local lon		= supex.get_our_body_table()["longitude"][1]
	local lat		= supex.get_our_body_table()["latitude"][1]
	if not lon or not lat then
		return false
	end
	-->> get point info
	local res = APP_POOL["point_match_road_result"]
	if not res then
		return false
	end
	local limit_speed	= res['SR']
	local rt		= tonumber(res['RT'])
	only.log('D', scan.dump(res))
	only.log('D', string.format("[rt = %s]", rt))

	-->> fiter ok limit road
	if not limit_speed or limit_speed == "NULL" then
		return false
	end
	limit_speed = tonumber(limit_speed)
	if limit_speed == 0 then
		return false
	end
	if rt == 0 and limit_speed < 80 then
		return false
	end
	if rt ~= 0 and limit_speed < 60 then
		return false
	end
	only.log('D', string.format("[limit_speed:%s]", limit_speed))

	-->> get over speed point count
	local high_limit_speed = limit_speed * 1.05
	local cnt = 0
	for i = 1 , all do
		speed = supex.get_our_body_table()["speed"][i]
		if speed > high_limit_speed then
			cnt = cnt + 1
		end
	end

	local ok, point_count = luakv_api.cmd('owner', accountID, 'get', is_over_speed_point_count_key)
	if not ok then
		only.log('D', "[redis is error]")
		return false
	end
	local ok, start_time = luakv_api.cmd('owner', accountID, 'get', is_over_speed_start_time_key)
	if not ok then
		only.long("D", "[redis is error]")
		return false
	end
	point_count = tonumber(point_count) or -1
	start_time = tonumber(start_time ) or -1
	local time = os.time()

	if cnt == 5 and (point_count == -1 or start_time + 180 < time) then
		luakv_api.cmd('owner', accountID, 'set', is_over_speed_point_count_key, 0)
		luakv_api.cmd('owner', accountID, 'set', is_over_speed_start_time_key, time)
		local flag = 0
		local value = string.format("%s:%s", flag, limit_speed)
		only.log('D', string.format("[over_speed][value:%s][limit_speed:%s]", value, limit_speed))
		scene.push( app_name, { ["overSpeedCarry"] = value } )
		return true
	end

	if start_time + 180 >  time then
		local over_speed_count = 0
		for i = 1 , all do
			if tonumber(supex.get_our_body_table()["speed"][i]) > high_limit_speed  then
				over_speed_count  = over_speed_count  + 1
			end
		end
		over_speed_count =  over_speed_count  + point_count

		--local NUM = 30
		local NUM = 45
		--local NUM = 60
		if over_speed_count >= NUM  then
			luakv_api.cmd('owner', accountID, 'set', is_over_speed_point_count_key, 0)
			luakv_api.cmd('owner', accountID, 'set', is_over_speed_start_time_key, time)
			local flag = 1
			local value = string.format("%s:%s", flag , limit_speed)
			only.log('D', string.format("[over_speed][value:%s][limit_speed:%s]",value,limit_speed))
			scene.push( app_name, { ["overSpeedCarry"] = value } )
			return true;
		else
			luakv_api.cmd('owner', accountID, 'set', is_over_speed_point_count_key, over_speed_count )
		end
	end


	return false
end

function is_road_traffic(app_name)
	if road_traffic.handle() then
		return true
	end
end
