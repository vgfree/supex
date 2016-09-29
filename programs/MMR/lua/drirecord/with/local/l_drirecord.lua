-- auth: baoxue
-- time: 2014.04.29

local utils 		= require('utils')
local only 		= require('only')
local redis_api 	= require('redis_pool_api')
local mysql_api 	= require('mysql_pool_api')
local supex 		= require('supex')
local map		= require('map')
local luakv 		= require('luakv')

module('l_drirecord', package.seeall)

function bind()
	return '["collect", "accountID", "tokenCode"]'
end

function match()
	return true
end

function drive_is_variable_speed()
	local speed_tb  = supex.get_our_body_table()["speed"]
	local time_tb  = supex.get_our_body_table()["GPSTime"]
	local accountID         = supex.get_our_body_table()["accountID"]

	local ok,vs = luakv_api.cmd("owner", accountID, "get", accountID .. ":variableSpeed")
	vs = tonumber(vs) or 0

	local list = {
		accelerate = 0,
		decelerate = 0
	}
	for i=1, #speed_tb -1 do
		local cs = (speed_tb[i + 1] - speed_tb[i]) / (time_tb[i + 1] - time_tb[i])
		if vs ~= cs then
			if cs == 0 then
				--变速一次
				if vs > 0 then
					list["accelerate"] = list["accelerate"] + 1
				else
					list["decelerate"] = list["decelerate"] + 1
				end
			end
		end
		vs = cs
	end
	luakv_api.cmd("owner", accountID, "set", accountID .. ":variableSpeed", tostring(vs))
	return list
end




local function direction_sub(dir1, dir2)
        local angle = math.abs(dir1 - dir2)
        return (angle <= 180) and angle or (360 - angle)
end


function drive_is_sharp_turn()
	local speed_tb  = supex.get_our_body_table()["speed"]
	local direction_tb  = supex.get_our_body_table()["direction"]
	local accountID         = supex.get_our_body_table()["accountID"]

	local ok,vs = luakv_api.cmd("owner", accountID, "get", accountID .. ":sharpTurn")
	vs = tonumber(vs) or 0

	local add_turn = 0
	for i=1, #direction_tb -1 do
		local cs = direction_sub(direction_tb[i + 1], direction_tb[i])
		if vs > 10 and cs <= 10 then
			if speed_tb[i + 1] > 50 then
				--一次转弯
				add_turn = add_turn + 1
			end
		end
		vs = cs
	end
	luakv_api.cmd("owner", accountID, "set", accountID .. ":sharpTurn", tostring(vs))
	return add_turn
end




function drive_mile()
	local speed_tb  = supex.get_our_body_table()["speed"]
	local time_tb  = supex.get_our_body_table()["GPSTime"]
	local lat_tb  = supex.get_our_body_table()["latitude"]
	local lon_tb  = supex.get_our_body_table()["longitude"]
	local accountID         = supex.get_our_body_table()["accountID"]

	if #time_tb == 0 then
		return 0
	end

	local ok,stt_lon = luakv_api.cmd("owner", accountID, "get", accountID .. ":driveLastLon")
	local ok,stt_lat = luakv_api.cmd("owner", accountID, "get", accountID .. ":driveLastLat")
	local ok,stt_time = luakv_api.cmd("owner", accountID, "get", accountID .. ":driveLastTime")
	local ok,stt_speed = luakv_api.cmd("owner", accountID, "get", accountID .. ":driveLastSpeed")
	local ok,mile = luakv_api.cmd("owner", accountID, "get", accountID .. ":driveMile")
	mile = tonumber(mile) or 0

	local add_mile = 0
	if stt_time then
		local affter = time_tb[1] - tonumber(stt_time)
		if affter ~= 0 then
			if affter <= 15 then
				add_mile = add_mile + ( (speed_tb[1] + tonumber(stt_speed))/affter )
			else
				add_mile = add_mile + map.get_two_point_dist(tonumber(stt_lat), tonumber(stt_lon), lat_tb[1], lon_tb[1])
			end
		end
	end
	for i=1, #time_tb -1 do
		local affter = time_tb[i + 1] - time_tb[i]
		if affter ~= 0 then
			if affter <= 15 then
				add_mile = add_mile + ( (speed_tb[i + 1] + speed_tb[i])/affter )
			else
				add_mile = add_mile + map.get_two_point_dist(lat_tb[i + 1], lon_tb[i + 1], lat_tb[i], lon_tb[i])
			end
		end
	end
	mile = mile + add_mile

	luakv_api.cmd("owner", accountID, "set", accountID .. ":driveLastLon", lon_tb[#lon_tb])
	luakv_api.cmd("owner", accountID, "set", accountID .. ":driveLastLat", lat_tb[#lat_tb])
	luakv_api.cmd("owner", accountID, "set", accountID .. ":driveLastTime", time_tb[#time_tb])
	luakv_api.cmd("owner", accountID, "set", accountID .. ":driveLastSpeed", speed_tb[#speed_tb])
	luakv_api.cmd("owner", accountID, "set", accountID .. ":driveMile", mile)
	return add_mile
end




function drive_online()
	local accountID         = supex.get_our_body_table()["accountID"]
	local speed_tb  = supex.get_our_body_table()["speed"]
	local time_tb  = supex.get_our_body_table()["GPSTime"]
	if #time_tb == 0 then
		return 0
	end

	local ok,goon_time = luakv_api.cmd("owner", accountID, "get", accountID .. ":goonTime")
	goon_time = tonumber(goon_time) or os.time()

	local tired = false
	local online = time_tb[1] - goon_time
	if online > 3 * 60 * 60 then
		tired = true
	end

	local ok,stop_time = luakv_api.cmd("owner", accountID, "get", accountID .. ":stopTime")
	if stop_time then
		-- 只对第一个点做分析
		local offline = time_tb[1] - tonumber(stop_time)
		if speed_tb[1] == 0 then
			if offline > 10 * 60 then
				luakv_api.cmd("owner", accountID, "del", accountID .. ":goonTime")
			end
		else
			luakv_api.cmd("owner", accountID, "del", accountID .. ":stopTime")
			if offline > 10 * 60 then
				luakv_api.cmd("owner", accountID, "set", accountID .. ":goonTime", time_tb[1])
			end
		end
	else
		if speed_tb[1] == 0 then
			luakv_api.cmd("owner", accountID, "set", accountID .. ":stopTime", time_tb[1])
		end
	end
	return tired and 1 or 0
end



function drive_is_over_speed()
	local speed_tb  = supex.get_our_body_table()["speed"]
	local longitude_tb  = supex.get_our_body_table()["longitude"]
	local latitude_tb  = supex.get_our_body_table()["latitude"]
	local direction_tb  = supex.get_our_body_table()["direction"]
	local accountID         = supex.get_our_body_table()["accountID"]

	local ok, result = redis.cmd('match_road', '', 'hmget', 'LOCATE', longitude_tb[1], latitude_tb[1], direction_tb[1])
	if not ok then
		only.log('E', string.format("get segmentID failed by : [** lon:%s, lat:%s, dir:%s**]", longitude_tb[1], latitude_tb[1], direction_tb[1]))
		return 0
	end

	local rt = result[5]
	local MAX_SPEED = {
		[0] 	=  	120,
		[10] 	=	80,
	}	
	local max_speed = MAX_SPEED[rt] or 60


	local ok,vs = luakv_api.cmd("owner", accountID, "get", accountID .. ":overSpeed")
	vs = tonumber(vs) or 0

	local add_over = 0
	for i=1, #speed_tb do
		local cs = speed_tb[i] > max_speed and 1 or 0
		if vs == 1 and cs == 0 then
			--一次超速
			add_over = add_over + 1
		end
		vs = cs
	end
	luakv_api.cmd("owner", accountID, "set", accountID .. ":overSpeed", tostring(vs))
	return add_over
end

function work()
	local add_shift = drive_is_variable_speed()
	local add_turn = drive_is_sharp_turn()
	local add_mile = drive_mile()
	local add_tired = drive_online()
	local add_over = drive_is_over_speed()

	local accountID         = supex.get_our_body_table()["accountID"]
	local mirrtalkID         = supex.get_our_body_table()["mirrtalkID"]
	local tokenCode         = supex.get_our_body_table()["tokenCode"]
	local sql = string.format("UPDATE daoke_AccelerationInfo SET SAcceleration=SAcceleration + %d, RAcceleration=RAcceleration + %d, Sharpturn=Sharpturn + %d, miles=miles + %d, overspeed=overspeed + %d, tired=tired + %d WHERE mirrtalkid='%s' and accountid='%s' and tokenCode='%s'",
		add_shift["accelerate"], add_shift["decelerate"], add_turn, add_mile, add_over, add_tired, mirrtalkID, accountID, tokenCode)
	local ok, ret = mysql_api.cmd('dataTest','UPDATE', sql)
end
