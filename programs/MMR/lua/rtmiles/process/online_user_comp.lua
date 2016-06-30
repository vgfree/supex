--版权声明：无
--文件名称：online_user_comp.lua
--创建者  ：耿玄玄
--创建日期：2015-09-01
--文件描述：维持用户在线状态列表
--修    改：

local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local ONLINE_TTL_TIME = 120

module('online_user_comp', package.seeall)

function handle()
	if (not supex.get_our_body_table()["collect"]) then
		return
	end

        local req_body = supex.get_our_body_table()
	local M = req_body['M']
	local tokenCode = req_body['tokenCode']
	local accountID = req_body['accountID']
	local model = req_body['model']
	local GPSTime	= req_body['GPSTime']
	local gps_count = #GPSTime

	if gps_count == 0 then
		return
	end

	local last_idx = 1
	if GPSTime[gps_count] > GPSTime[1] then	-- 获取最新时间
		last_idx = gps_count
	end

	local time = GPSTime[last_idx]
	local lon = req_body['longitude'][last_idx]
	local lat = req_body['latitude'][last_idx]
	local dir = req_body['direction'][last_idx]
	local speed = req_body['speed'][last_idx]
	local altitude = req_body['altitude'][last_idx]
	
	local online_key = string.format("%s:online", M)
	local ok, ret = redis_api.cmd('mapOnlineUserM', M, 'hmset', online_key,
		'accountID', accountID,
		'tokenCode', tokenCode,
		'GPSTime'  , time,
		'longitude', lon,
		'latitude' , lat,
		'direction', dir,
		'speed'	   , speed,
		'model'	   , model,
		'altitude' , altitude)
	
	if not ok then
		only.log('E', "write mapOnlineUser with M ERROR")
	end

	if accountID ~= '' then
		local online_key = string.format("%s:online", accountID)
                local ok, ret = redis_api.cmd('mapOnlineUserAccountID', accountID, 'hmset', online_key,
                        'M', M,
                        'tokenCode', tokenCode,
                        'GPSTime'  , time,
                        'longitude', lon,
                        'latitude' , lat,
                        'direction', dir,
                        'speed'    , speed,
			'model'	   , model,
                        'altitude',altitude)

                if not ok then
                        only.log('E', "write mapOnlineUser with accountID ERROR")
                end

	end
--	ok, ret = redis_api.cmd('mapOnlineUser', M, 'expire', online_key, ONLINE_TTL_TIME)
--	if not ok then
--		only.log('E', "set ttl ERROR")
--	end
end
