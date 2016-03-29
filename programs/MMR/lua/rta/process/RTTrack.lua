--版权声明：无
--文件名称：RTTrack.lua
--创建者：tengxinghui
--创建日期：2015.12.8
--文件描述：实时轨迹
--历史记录：无

local redis_api	= require('redis_pool_api')
local luakv_api = require('luakv_pool_api')
local only 	= require('only')
local utils 	= require('utils')
local cfg 	= require('cfg')
local cjson	= require('cjson')
local func_add_grid_info    = require('func_add_grid_info')
local SUM_DIST	= 60             --方向角筛选后gps点距离累加的限制距离
local LIMIT_DIST = 20           --方向角筛选后两个相邻gps点之间的限制距离
local LIMIT_DIR_DIFF = 12       --gps点方向角变化的限制条件

module('RTTrack', package.seeall)

--名称：hmset_to_redis
--功能：存储数据到redis
--参数：imei,redis键,gps数据包,
--	model---摄像头模式
--	redisName --- redis名字
--返回值：true-正常返回,false-非正常返回

local function hmset_to_redis(imei,key,gps,model,redisName)

	local ok,ret = redis_api.cmd(redisName,imei or '',"hmset",key,
		'GPSTime',gps['GPSTime'],
		'longitude',gps['longitude'],
		'latitude',gps['latitude'],
		'direction',gps['direction'],
		'speed',gps['speed'],
		'altitude',gps['altitude'],
		'model',model)

	if not ok then
		return false
	end

	return true
end

--名称：store_newest_gps
--功能：存储最新的gps数据
--参数：数据包
--返回值：true-正常返回,false-非正常返回

local function store_newest_gps(table)

	local points = table['points']
	local gps = points[#points]

	local imei = table['IMEI']
	local accountID = table['accountID']
	local model = table['model']
		
	local key = string.format("%s:newestGPS",imei)
	local success = hmset_to_redis(imei,key,gps,model,'RTTrack')
	if not success then
		only.log('I',"hmset mapUserNewestGPS failed in function store_newest_gps!")
	end

	if tostring(accountID) ~= '' and string.len(accountID) == 10 then

		local key = string.format("%s:online",accountID)
		local ok = hmset_to_redis(imei,key,gps,model,'mapOnlineUserAccountID')
		if not ok then
			only.log('I',"hmset mapOnlineUserAccountID failed in function store_newest_gps!")
		end
	end

end

--名称：store_data_to_mapGPSData
----功能：存储数据到mapGPSData这个redis中
----参数：数据包
----返回值：无
   
local function store_data_to_mapGPSData(table)
	
	local accountID = table['accountID']
	if tostring(accountID) == '' or string.len(accountID) ~= 10 then
		accountID = table['IMEI']
	else
		local gps_tab = {}

                local points = table['points']
                local gps = points[#points]

                gps_tab ={
                        ['imei']        = table['IMEI'],
                        ['model']       = table['model'],
                        ['longitude']   = gps['longitude'],
                        ['latitude']    = gps['latitude'],
                        ['direction']   = gps['direction'],
                        ['time']        = gps['GPSTime'],
                        ['accountID']   = accountID,
                }

                func_add_grid_info.entry(gps_tab)

	end
end

--名称：sadd_imei_to_redis
--功能：以集合的方式存储imei到redis
--参数：imei,GPSTime
--返回值：true-正常返回,false-非正常返回

local function sadd_imei_to_redis(imei,GPSTime)

	local time = os.date("%Y%m%d%H",GPSTime)
	local key = string.format("RTTrackUser:%s",time)

	local ok,ret = redis_api.cmd('RTTrack',imei or '',"sadd",key,imei)
	if not ok then
		only.log('I',"sadd RTTrackUser failed in function store_rttrack_user!")
		return false
	end

	return true
end

--名称：store_rttrack_user
--功能：根据是否为第一个数据包来存储imei
--参数：table
--返回值：true-正常返回,false-非正常返回

local function store_rttrack_user(table)
	
	local imei = table['IMEI']
	local isFirst = table['isFirst']
	local points = table['points']

	if isFirst then
		local ok = sadd_imei_to_redis(imei,points[1]['GPSTime'])
		if not ok then
			return false
		end
	else
		local ok = sadd_imei_to_redis(imei,points[#points]['GPSTime'])
		if not ok then
			return false
		end
	end	
end

--名称：direction_sub
--功能：对方向角求差
--参数：dir1---第一个方向角
--	dir2---第二个方向角
--返回值：放回两个方向角的差

local function direction_sub(dir1, dir2)
        local angle = math.abs(dir1 - dir2)
        return (angle <= 180) and angle or (360 - angle)
end

--名称：filter_direction
--功能：根据方向角筛选
--参数：points---数据包
--	cur_gps--筛选的当前数据包
--返回值：cur_gps---筛选的当前数据包
--	  gps_tab---筛选后的结果

local function filter_direction(points,cur_gps)

	local cur_dir = cur_gps['direction']
	local gps_tab = {}

	for i=1,#points do

		local next_dir = points[i]['direction']
		if cur_dir == -1 then
			cur_dir = next_dir
			cur_gps = points[i]
			goto DONOTHING 
		end
		if next_dir == -1 then
			goto DONOTHING
		end

		if direction_sub(next_dir,cur_dir) > LIMIT_DIR_DIFF then
			cur_dir = next_dir
			table.insert(gps_tab,cur_gps)
			cur_gps = points[i]
		end
		::DONOTHING::
	end
	return cur_gps,gps_tab
end

--名称：process_first_packet
--功能：对每个imei的第一个数据包进行处理
--参数：imei---设备号
--	points-数据包
--	key---redis键
--返回值：true---正常返回
--        false---非正常返回

local function process_first_packet(imei,points,key)

	local cur_gps = points[1]

	-->>方向角筛选
	local cur_gps,gps_tab = filter_direction(points,cur_gps)
	local ok,fmt_cur_gps = pcall(cjson.encode,cur_gps)
	if not ok then
		only.log('I',"cur_gps encode failed!")
		return false
	end

	-->>根据情况存储值到redis
	if #gps_tab == 0 then
		local ok,ret = luakv_api.cmd('luakv',imei or '',"hmset",key,'cur_gps',fmt_cur_gps,'sum_dist',0)
		if not ok then
			only.log('I',"oprate redis failed")
			return false
		end
	else
		local ok,fmt_gps_tab = pcall(cjson.encode,gps_tab)
		if not ok then
			only.log('I',"gps_tab encode failed!")
			return false
		end

		local ok,ret = luakv_api.cmd('luakv',imei or '',"hmset",key,'cur_gps',fmt_cur_gps,'gps_tab',fmt_gps_tab,'sum_dist',0)
		if not ok then
			only.log('I',"hmset luakv failed in function process_first_packet!")
			return false
		end
	end

	return true
end

--名称：get_distance
--功能：计算两个gps点之间的距离
--参数：cur_gps---当前gps点
--	next_gps--下一个gps点
--返回值：dist---两个gps点之间的距离

local function get_distance(cur_gps,next_gps)

        local average_speed = (cur_gps['speed'] + next_gps['speed']) / 7.2 --公里每小时转米每秒
        local time_diff = next_gps['GPSTime'] - cur_gps['GPSTime']
        local dist = average_speed * time_diff
        --only.log('D',"dist========"..dist);

        return dist

end

--名称：sadd_gps_to_redis
--功能：将筛选后的数据以有序集合的形式存储到redis
--参数：imei,key,
--	GPS---pgs数据包
--	fmt_cur_point---转码后的当前gps点
--	sum_dist---累加的距离
--返回值：true---正常返回
--        false---非正常返回

local function sadd_gps_to_redis(imei,key,GPS,fmt_cur_point,sum_dist)

	local gps_tab = {}
	table.insert(gps_tab,GPS[#GPS-1])
	table.insert(gps_tab,GPS[#GPS])

	local ok,fmt_gps_tab = pcall(cjson.encode,gps_tab)
	if not ok then
		only.log('I',"gps_tab encode failed!")
		return false
	end
	local ok,ret = luakv_api.cmd('luakv',imei or '',"hmset",key,'cur_gps',fmt_cur_point,'gps_tab',fmt_gps_tab,'sum_dist',sum_dist)
	if not ok then
		only.log('I',"hmset redis failed in function process_packet")
		return false
	end

	for i=1,#GPS-2 do
		if not GPS[i]['status'] then

			local GPSTime = GPS[i]['GPSTime']
			local time = os.date("%Y%m%d%H",GPSTime)
	       		local key = string.format("RTTrack:%s:%s",imei,time)

			local log = GPS[i]['longitude'] * 10000000
			local lat = GPS[i]['latitude'] * 10000000
			local dir = GPS[i]['direction']
			local speed = GPS[i]['speed']

			local value = string.format("%s|%s|%s|%s|%s",GPSTime,log,lat,dir,speed)
			local ok,ret = redis_api.cmd('RTTrack',imei or '',"sadd",key,value)

			if not ok then
				only.log('E',"failure to zadd redis!")
				return false
			end
		end
	end
        return true
end

--名称：distance_filter
--功能：根据距离筛选pgs点
--参数：imei,key
--	GPS---gps数据包
--	fmt_cur_point---转码后的当前gps
--	ret---从redis中取的数据
--返回值：true---正常返回
--        false---非正常返回

local function distance_filter(imei,key,GPS,fmt_cur_point,ret)
    
        local sum_dist = tonumber(ret[3])
        local cur_dist = get_distance(GPS[1],GPS[2])
	sum_dist = sum_dist + cur_dist

        for i=2,#GPS - 1 do

                local next_dist = get_distance(GPS[i],GPS[i+1])

                if cur_dist >= LIMIT_DIST and next_dist < LIMIT_DIST then

                        sum_dist = sum_dist + next_dist

                elseif cur_dist <= LIMIT_DIST and next_dist > LIMIT_DIST then

                        sum_dist = 0

                elseif cur_dist <= LIMIT_DIST and next_dist <= LIMIT_DIST then

                        if sum_dist >= SUM_DIST then
                                sum_dist = 0
                        else
                                sum_dist = sum_dist + next_dist
                                GPS[i]['status'] = true
                        end
                else
    
                end
                cur_dist = next_dist
        end

	local success = sadd_gps_to_redis(imei,key,GPS,fmt_cur_point,sum_dist)
	if not success then
		return false
	end

	return true
end

--名称：process_packet
--功能：处理gps数据包
--参数：imei
--	key---redis的键
--	ret---包含当前gps点和上次筛选后的gps数据
--	points---gps数据包
--返回值：true---正常返回
--	  false--非正常返回

local function process_packet(imei,key,ret,points)

	local fmt_cur_point = ret[1]
	local ok,cur_point = pcall(cjson.decode,fmt_cur_point)
	if not ok then
		only.log('I',"cur_gps decode failed!")
		return false
	end

	local cur_gps,gps_tab = filter_direction(points,cur_point)
	local ok,fmt_cur_gps = pcall(cjson.encode,cur_gps)
	if not ok then
		only.log('I',"cur_point encode failed!")
		return false
	end

	if not ret[2] or #ret[2] == 0 then
		--only.log('I',"ret[2] is nil")
	else
		local fmt_gps_tab = ret[2]
		local ok,point_tab = pcall(cjson.decode,fmt_gps_tab)
		if not ok then
			only.log('I',"gps_tab decode failed!")
			return false
		end

		for k,v in ipairs(point_tab) do
			table.insert(gps_tab,k,v)
		end
	end

	if #gps_tab < 3 then
		local ok,fmt_gps_tab = pcall(cjson.encode,gps_tab)
		local ok,ret = luakv_api.cmd('luakv',imei or '',"hmset",key,'cur_gps',fmt_cur_gps,'gps_tab',fmt_gps_tab)
		if not ok then
			only.log('E',"hmset redis failed in function process_packet")
			return false
		end
		return true
	end

	local status = distance_filter(imei,key,gps_tab,fmt_cur_gps,ret)
	if not status then
		return false
	end

	return true
end

function handle(table)

	local isDelay = table['isDelay']
	if isDelay then
		return true
	end

	local imei = table['IMEI']
	local isFirst = table['isFirst']

	local points = table['points']
	if #points == 0 then
		only.log('I',"points is null")
		return true
	end

	store_newest_gps(table)

	store_data_to_mapGPSData(table)

	store_rttrack_user(table)

        local key = string.format("%s:rttrack_data",imei)
	local ok,ret = luakv_api.cmd('luakv',imei or '',"hmget",key,'cur_gps','gps_tab','sum_dist')

	if not ok then
		only.log('I',"hmget luakv failed")
		return false
	end

	if not ret or #ret == 0 then
		local ok = process_first_packet(imei,points,key)
		if not ok then
			return false
		end
		return true
	end
	local status = process_packet(imei,key,ret,points)
	if not status then
		return false
	end

	return true
end

