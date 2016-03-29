local only = require('only')
local supex = require('supex')
local app_utils = require("app_utils")
local cachekv = require("cachekv")
local redis_api = require("redis_pool_api")

module('settings', package.seeall)


--keyName:onlineUser:set  	place:statistic  	des:存放在线用户  action:w
--keyName:configTimestamp\timestamp  place:private本地缓存 	des:存放用户开机时间 action:r
local function power_on_settings()
	local accountID = supex.get_our_body_table()["accountID"]
	redis_api.cmd('statistic', accountID, 'sadd','onlineUser:set', accountID)
	redis_api.cmd('driview', accountID, 'set', accountID .. ':configTimestamp', os.time())--TODO

end


local function update_citycode_channel(accountID)
	--每５分钟获取下城市
	local remainder = os.time() % 300
	if (remainder >= 0) and (remainder < 15) then
		local ok,cityCode = app_utils.get_city_code()
		if ok and cityCode then
			redis_api.cmd('statistic', accountID,'sadd', cityCode .. ':cityOnlineUser', accountID)
			cachekv.push('private', accountID, 'set', accountID..':cityCode', cityCode)
		end
	end
end

local function speed_statistics()
	local accountID = supex.get_our_body_table()["accountID"]
	local num_chage = supex.get_our_body_table()["longitude"]
	local speed_tb	= supex.get_our_body_table()["speed"]

	for i = 1, #num_chage do
		local speed = speed_tb[i]
		local speed_step = {
			[0] = '00',
			[1] = '01',
			[2] = '02',
			[3] = '03',
			[4] = '04',
			[5] = '05',
			[6] = '06',
			[7] = '07',
			[8] = '08',
			[9] = '09',
			[10] = '10',
			[11] = '11',
			[12] = '12',
			[13] = '13',
			[14] = '14',
			[15] = '15',
		}

		local ok,travelID = cachekv.pull('private', accountID, 'get', accountID .. ':travelID')

		local speed_mod = math.floor(speed/10)
		if speed_mod > 15 then
			speed_mod = 15
		end
		local speed_key = speed_step[speed_mod]
		--[[
		local ok_date, cur_date = redis_api.cmd('private', accountID, 'get', string.format("%s:speedDistribution", accountID))
		if not ok_date or not cur_date then
		cur_date = os.date("%Y%m")
		redis_api.cmd('private', accountID,'set', string.format("%s:speedDistribution", accountID), cur_date)
		end
		]]--
		local cur_date = os.date("%Y%m")
		if ok_status and travelID and speed_key and cur_date then
			local datacore_statistics_var_key  = accountID .. ":" .. travelID .. ":speedDistribution"  .. ":" .. cur_date
			--only.log('D', string.format("[i = %d][key = %s]", i, datacore_statistics_var_key))
			redis_api.cmd('dataCoreRedis', accountID, 'hincrby', accountID .. ":" .. travelID .. ":speedDistribution"  .. ":" .. cur_date, speed_key, 1)
			redis_api.cmd('dataCoreRedis', accountID, 'expire', accountID .. ":" .. travelID .. ":speedDistribution"  .. ":" .. cur_date, 48*3600)
		end
	end
end

--keyName:currentSpeed               place:private  des:初始化速度            action:w
--keyName:currentDirection           place:private  des:初始化方向角          action:w
local function collect_settings()
	local body_tb	= supex.get_our_body_table()

	local accountID =  body_tb["accountID"]
	local longitude =  body_tb["longitude"][1] or 0
	local latitude  =  body_tb["latitude"][1]  or 0
	local speed     =  body_tb["speed"][1]     or 0
	local direction =  body_tb["direction"][1] or '-1'
	local altitude  =  body_tb["altitude"][1]  or 0
	local gpstime   =  body_tb["GPSTime"][1]   or 0
	local gps_info  = longitude .. "," .. latitude


	redis_api.cmd('private', accountID, 'mset',accountID .. ':currentBL', gps_info,accountID .. ':currentGPSTime', gpstime,	accountID .. ':currentSpeed', speed,accountID .. ':currentDirection', direction,accountID .. ':currentAltitude', altitude)

	update_citycode_channel( accountID )

	speed_statistics()
end
--[[
function is_off_site( )
	local accountID = supex.get_our_body_table()["accountID"]
	local lon = supex.get_our_body_table()["longitude"][1]
	local lat = supex.get_our_body_table()["latitude"][1]
	local ok, home_city = cachekv.pull("private", accountID, 'get', accountID .. ':homeCityCode')
	if not ok or not home_city then
		only.log("I", "can't get homeCityCode from redis!")
		return false
	end
	local ok, city_code = cachekv.pull('private', accountID, 'get', accountID .. ':cityCode')
	if not ok then
		return false
	end
	if tonumber(city_code) == tonumber(home_city) then
		only.log('D', 'cityCode homeCityCode is same!')
		return false
	end
	return true
end
]]--
local function power_off_settings()
	local accountID = supex.get_our_body_table()["accountID"]

	redis_api.cmd('statistic', accountID, 'srem', 'onlineUser:set', accountID)


	-->>city online
	local ok, cityCode = cachekv.pull('private', accountID, 'get', accountID .. ":cityCode")
	if ok and cityCode then
		redis_api.cmd('statistic', accountID, 'srem', cityCode .. ':cityOnlineUser', accountID)
	end
end

function handle()
	if supex.get_our_body_table()["powerOn"] then
		local ok,result = pcall( power_on_settings )
		if not ok then
			only.log("E", result)
		end
	end
	if supex.get_our_body_table()["collect"] then
		local ok,result = pcall( collect_settings )
		if not ok then
			only.log("E", result)
		end
	end
	if supex.get_our_body_table()["powerOff"] then
		local ok,result = pcall( power_off_settings )
		if not ok then
			only.log("E", result)
		end
	end
end

