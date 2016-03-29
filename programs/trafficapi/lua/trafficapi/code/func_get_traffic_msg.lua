--Author	: dujun
--Date		: 2015-11-09
--Function	: get front speed info by given longitude, latitude and direction

local redis		= require('redis_pool_api')
local luakv		= require('luakv_pool_api')
local utils		= require('utils')
local only		= require('only')
local msg		= require('api_msg')
local gosay		= require('gosay')
local safe		= require('safe')
local socket		= require('socket')
local supex		= require('supex')

module("func_get_traffic_msg", package.seeall)

url_info = nil

function str_split(s, c)
	if not s then print("not string")  return nil end
	local tab = {}
	local str = s
	local index
	while true do
		index = string.find(str, c)
		if index == nil then 
			tab[#tab + 1]  = str
			break
		end
		tab[#tab + 1]  = string.sub(str, 1, index - 1)
		str = string.sub(str, index + 1)
	end
	return tab
end

function get_segmentID(res)

	local direction = res['direction']
	local longitude = res['longitude']
	local latitude = res['latitude']

	local ok, result = redis.cmd('match_road', '', 'hmget', 'LOCATE', longitude, latitude, direction)
	if not ok then
		only.log('E', string.format("get segmentID failed by : [** lon:%s, lat:%s, dir:%s**]", longitude, latitude, direction))
		resp_msg(3, 'false', msg['MSG_ERROR_LOCATION'])
		return false
	end

	local segmentID_tab = {
		segmentID = result[2],
		roadRootID = result[1],
	}

	if segmentID_tab['roadRootID'] and segmentID_tab['segmentID'] then
		return segmentID_tab
	else
		only.log('E', string.format("location failed by : [** lon:%s, lat:%s, dir:%s **]", longitude, latitude, direction))
		resp_msg(3, 'false', msg['MSG_ERROR_LOCATION'])
		return false
	end
end

function get_currentInfo(res, roadRootID, segmentID)

	local key_1 = roadRootID .. string.format(",%d", segmentID)
	local ok, tab_1 = redis.cmd('mapSGInfo', '', 'hmget', key_1 .. ':SGInfo', 'NAME','SGSC','SGFER','RT')
	if not ok  then 
		only.log('E', string.format("get roadName failed by : key[%s:SGInfo]", key_1))
		resp_msg(3, 'false', msg['SYSTEM_ERROR'])
		return false
	end
	local startIR, roadLevel
	if tab_1[3] then
		startIR = tonumber(tab_1[3])
	end
	if tab_1[4] and tonumber(tab_1[4])then
		if tonumber(tab_1[4]) == 0 then
			roadLevel = 1
		else
			roadLevel = 2
		end
	else
		only.log('E', string.format("get roadLevel failed by : key[%s:SGInfo]", key_1))
	end

	local lon = tonumber(res['longitude'])
	local lat = tonumber(res['latitude'])
	local dir = tonumber(res['direction'])
	local uid = res['uid'] or ''
	local speed = tonumber(res['speed']) or -1

	if tonumber(speed) then
		speed = math.floor(speed)
	end
	local ret,_ = math.modf(lon*100)
	local stat, _ = math.modf(lat*100)
	local key_2 = string.format("%d&%d", ret, stat)

	local ok, tab_2 = redis.cmd('mapGridOnePercent_v2', '', 'hmget', key_2, 'cityName', 'countyName')
	if not ok or not tab_2 then
		only.log('E', string.format("get cityName failed by : key[%s]", key_2))
		resp_msg(3, 'false', msg['SYSTEM_ERROR'])
		return false
	end

	local currentInfo = {
		roadName = tab_1[1] or '',
		speed = speed,
		cityName = tab_2[1] or '',
		countyName = tab_2[2] or -1,
		dir = math.floor(dir) or -1,
		cityVols = 0,
		startIN = tab_1[2] or '',
		startIR = startIR or -1,
		roadLevel = roadLevel or -1
	}

	return currentInfo
end

function get_trafficType(isHistory, res, tab, maxspeed)

	local trafficType
	if tonumber(isHistory) == 1 or tonumber(res) == -1 or tonumber(maxspeed) == 0 or tonumber(res) == 0 then
		trafficType = 4
	elseif tab and tonumber(tab) == 0 then
		if res and tonumber(res) >= 60 then
			trafficType = 1
		elseif res and tonumber(res) >= 30 and tonumber(res) < 60 then
			trafficType = 2
		else
			trafficType = 3
		end
	else
		if res and tonumber(res) >= 20 then
			trafficType = 1
		elseif res and tonumber(res) >= 10 and tonumber(res) < 20 then
			trafficType = 2
		else
			trafficType = 3
		end
	end
	return trafficType
end

function operate_redis(roadRootID, segmentID)

	only.log('D', 'operat_redis : get_SGInfo')
	only.log('D', 'RRID : %d, SGID :%d', roadRootID, segmentID)
	local road_tab = get_SGInfo(roadRootID, segmentID)
	if not road_tab then
		only.log('E', string.format("Failed to get roadInfo by : key[%s,%s]", roadRootID, segmentID))
		gosay.resp_msg(msg['SYSTEM_ERROR'])
		return false, nil
	end
	only.log('D', 'road_tab = ' .. scan.dump(road_tab))

	local key = string.format("%d%03d:trafficInfo", roadRootID, segmentID)
	local ok, speed_tab = luakv.cmd('luakv', '', 'get', key)
	local traffic_tab
	if not ok or not speed_tab then
		only.log('W', string.format("Failed to get speedInfo by : key[%s]", key))
		traffic_tab = {-1, -1, -1, -1}
	else
		traffic_tab = str_split(speed_tab, ':')
	end
	if not traffic_tab or #traffic_tab ~= 4 then
		traffic_tab = {-1, -1, -1, -1}
	end

	return road_tab, traffic_tab
end

function get_speed_info(roadRootID, segmentID, interval)

	local tab, traffic_tab = operate_redis(roadRootID, segmentID)
	if not tab or not traffic_tab or not next(tab) then
		only.log('E', string.format("Failed to get SGInfo by : key[%s,%s]", roadRootID, segmentID))
		return false
	end
	if not traffic_tab then traffic_tab = {-1, -1, -1, -1} end

	local isHistory, trafficType, pass_time
	if tonumber(traffic_tab[3]) and (tonumber(traffic_tab[3]) + tonumber(interval) >= os.time()) then
		isHistory = 0
	else
		isHistory = 1
		traffic_tab = {-1, -1, -1, -1}
	end

	trafficType = get_trafficType(isHistory, traffic_tab[2], tab[6], traffic_tab[1])
	pass_time = get_pass_time(traffic_tab[2], tab[6], tab[1])
	local data = {
		MS = tonumber(traffic_tab[1]) or -1,
		AS = tonumber(traffic_tab[2]) or -1,
		CT = tonumber(traffic_tab[3]) or -1,
		PT = pass_time,
		RL = tonumber(tab[1]) or -1,
		IN = tab[5],
		IR = tonumber(tab[2]) or -1,
		IL = tonumber(string.format("%.6f", tab[3])),
		IB = tonumber(string.format("%.6f", tab[4])),
		HT = 0,
		TT = tonumber(trafficType) or 4
	}

	return data
end

function get_pass_time(avgspeed, rt, len)

	local pass_time
	avgspeed = tonumber(avgspeed)
	rt = tonumber(rt)
	len = tonumber(len)
	if avgspeed ~= -1 and avgspeed ~= 0 then
		pass_time = math.floor(len*3.6/avgspeed)
	elseif rt == 1 then
		pass_time = math.floor(len*0.06)
	elseif rt == 10 then
		pass_time = math.floor(len*0.09)
	else
		pass_time = math.floor(len*0.12)
	end
	return pass_time
end

function get_next_segmentID(roadRootID, segmentID, last_count)
	local ret_count = last_count
	if last_count == 0 then
		local key = string.format("%s,%s:SGInfo", roadRootID, segmentID)
		local ok, count = redis.cmd('mapSGInfo', '', 'hmget', key, 'COUNT', 'NEXT_SGID')
		if not ok or not count then
			only.log('E', "Faile to get count by : key[%s]", key)
			return nil, nil, nil
		end
		last_count = tonumber(count[1])
	end

	if segmentID and last_count and tonumber(segmentID) < tonumber(last_count) then
		segmentID = segmentID + 1
	else 
		local key = string.format("%s,%s:SGInfo", roadRootID, segmentID)
		only.log('I', "key: %s", key)
		local ok, count = redis.cmd('mapSGInfo', '', 'hmget', key, 'COUNT', 'NEXT_SGID')
		if not ok or not count then
			only.log('E', string.format("Faile to get count by : key[%s]", key))
			return nil, nil, nil
		end

		if not count[2] then
			return nil,nil,nil
		end
		next_tab = str_split(count[2],',')
		roadRootID = next_tab[1]
		segmentID = next_tab[2]
		ret_count = 0
	end

	return roadRootID, segmentID, ret_count
end

function compare_TT(TT1, TT2)

	if TT1 == TT2 then
		cur_TT = TT1
	elseif (TT1 == 1 and TT2 == 4) or (TT1 == 4 and TT2 == 1) then
		cur_TT = 1
	elseif TT1 == 1 then
		cur_TT = TT2
	elseif TT2 == 1 then
		cur_TT = TT1
	elseif (TT1 == 2 or TT1 == 3 or TT1 == 6) or (TT2 == 2 or TT2 == 3 or TT2 == 6) then
		cur_TT = 6
	else
		only.log('E', string.format("merge traffic TT ERROR, TT1:%s, TT2:%s",TT1,TT2))
	end

	return cur_TT
end

--名称：arc_tan；
--功能：计算角度；
--参数：两个经度纬度值;
--返回值：角度值；
--修改：新生成函数,

function arc_tan(lon1, lon2, lat1, lat2)
	if not lon1 or not lon2 or not lat1 or not lat2 then
		only.log('E', 'diff_lon_lat_parmnet error')
		return false
	end

	local lon_diff = lon2 - lon1
	local lat_diff = lat2 - lat1
	if 0 == lon_diff then
		if lat_diff > 0 then
			return 0
		end
		return 180
	end
	if  0 == lat_diff then
		if lon_diff > 0 then
			return 90
		end
		return  270
	end
	local lon_temp = math.abs(lon_diff)	
	local lat_temp = math.abs(lat_diff)
	-->>转换成可以使用的角度
	if lon_diff > 0 and lat_diff >0  then
		return  math.atan(lon_temp/lat_temp)*180/3.14
	elseif lon_diff >0 and lat_diff < 0  then
		return  180 - math.atan(lon_temp/lat_temp)*180/3.14
	elseif  lon_diff< 0 and lat_diff < 0  then
		return  180 +  math.atan(lon_temp/lat_temp)*180/3.14
	else
		return 360 -  math.atan(lon_temp/lat_temp)*180/3.14
	end
end

-->> 返回数据结果
function resp(status, data)
	local afp = supex.rgs( status )
	supex.say(afp, data)
	return supex.over(afp)
end

function resp_msg(status, bool, msg, result)
    local out_msg, info
    if not result then
        info = msg[2]
    else
        info = string.format(msg[2], result)
    end

    local star = string.sub(info, 1, 1)
    if star == '[' or star == '{' then
        out_msg = string.format('%s|%s|{"ERRORCODE":"%s", "RESULT":%s}', status, bool, msg[1], info )
    else
        out_msg = string.format('%s|%s|{"ERRORCODE":"%s", "RESULT":"%s"}', status, bool, msg[1], info )
    end
    
    resp(200, out_msg)
    return
end
