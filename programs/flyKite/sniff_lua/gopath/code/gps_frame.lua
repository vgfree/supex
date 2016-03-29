--文件名称: gps_frame.lua
--创建者  : 耿玄玄
--创建时间: 2015-08-20
--文件描述: 处理每个frame的途径(frame即时长 相同的一个片段)
--历史记录: 1.重构途径计算，修改按frame获取数据的同时计算途径；增加补传数据计算

local only			= require ('only')
local redis_api			= require ('redis_pool_api')
local http_api			= require ('http_short_api')
local mysql_api			= require ('mysql_pool_api')
local fun_point_match_road	= require ('spx_point_match_road')
local utils			= require ('utils')
local scan			= require ('scan')
local lua_decoder		= require ('libluadecoder')
local link			= require ("link")
local raidus_find_poi_func	= require ('raidus_find_poi_func')
local RoadRecordModule 		= require('road_record')
local RoadRecord		= RoadRecordModule.RoadRecord

local TEN_MINUTES               = 600
--不同字段在tsearch接口返回数据中的位置
local G_TSEARCH_IDX_TIME		= 1
local G_TSEARCH_IDX_LON		= 7
local G_TSEARCH_IDX_LAT		= 8
local G_TSEARCH_IDX_DIR		= 10
local G_TSEARCH_IDX_SPEED		= 11
local G_TSEARCH_IDX_TOKENCODE	= 12

module('gps_frame', package.seeall)

GPSFrame = {
	IMEI,
	tokenCode,
	frameStart,	--frame开始时间
	frameEnd,	--frame结束时间
	startPoint,	--当前frame开始GPS点信息
	endPoint,	--当前frame结束GPS点信息
	recordCount,	--当前frame中record个数
	recordSet,
}

function GPSFrame:new(IMEI, tokenCode, startTime, endTime)
	local self = {
	}

	setmetatable(self, GPSFrame)
	GPSFrame.__index = GPSFrame

	--初始化原始属性
	self['IMEI'] = IMEI
	self['tokenCode'] = tokenCode
	self['frameStart'] = startTime
	self['frameEnd'] = endTime
	self['recordCount'] = 0
	self['recordSet'] = {}

	return self
end 

--功  能: 根据record index 组装当前frame计算得出的road record的luakv使用的key
--参  数: record_idx: 当前frame计算所得的RoadRecord存在luakv中的索引
--返回值: luakv_key: record_idx索引位置的luakv key
function GPSFrame:getRecordKey(record_idx)
	local luakv_key = string.format("%s:%s:%s:RoadRecord", self['IMEI'], self['frameStart'], record_idx)
	return luakv_key
end

--功  能: 调用tsearch接口获取gps数据
--参  数: tsearch_api	: 获取正常数据或者补偿数据api name
--	  IMEI		: IMEI	
--	  st_time	: tsearch查询开始时间
--	  ed_time	: tsearch查询结束时间
--返回值: frame_data: frame时间段内正常或补传数据
function GPSFrame:getGPSData(tsearch_api, IMEI, st_time, ed_time)
	local body_info = {imei = IMEI, startTime = st_time, endTime = ed_time}
	local serv = link["OWN_DIED"]["http"]["tsearchapi/v2/getgps"]
	local body = utils.gen_url(body_info)
	local api_name = "tsearchapi/v2/" .. tsearch_api

	local body_data = utils.compose_http_json_request(serv, api_name, nil, body)

	local ok, ret = supex.http(serv['host'], serv['port'], body_data, #body_data)
	if not ok or not ret then return nil end
	-->获取RESULT后的数据	
	only.log('D', "%s, length:%s", tsearch_api, #ret)
	local data = utils.parse_api_result(ret, tsearch_api)
	if not data then
		return {}
	end
	if #data == 0 then return {} end
	--data中，偶数下标的元素才是gps数据,去掉奇数元素
	for k,_ in ipairs (data) do
		table.remove(data, k)
	end

	--使用tsearch解码格式解析数据
	local frame_data = lua_decoder.decode(#data, data)
	data = nil
	if not frame_data then
		return {}
	end

	return frame_data
end

--功  能: 获取gps数据
--返回值: frame_gps_data: frame时间段内正常gps数据
--	  frame_ext_data: frame时间段内补传gps数据
function GPSFrame:getData()
	local frame_gps_data
	local frame_ext_data

	if self['frameStart'] > self['frameEnd'] then
		only.log('E', string.format("frame time ERROR, start[%s] > end[%s]", self['frameStart'], self['frameEnd']))
		return {}, {} 
	end

	--获取正常数据
	frame_gps_data = self:getGPSData("getgps", self['IMEI'], self['frameStart'], self['frameEnd'])
	--获取补传数据
	frame_ext_data = self:getGPSData("getExtGps", self['IMEI'], self['frameStart'], self['frameEnd'])

	return frame_gps_data, frame_ext_data
end

--将gps数组转换为 gpstime为key的kv形式
function GPSFrame:arrayToKV(dest_data, src_data, isExtra)
	if not src_data or not dest_data then
		return
	end

	for i,val in ipairs(src_data) do
		repeat
			local gps_point = {
				['time'] = tonumber(val[G_TSEARCH_IDX_TIME]),
				['lon'] = tonumber(val[G_TSEARCH_IDX_LON]),
				['lat'] = tonumber(val[G_TSEARCH_IDX_LAT]),
				['speed'] = tonumber(val[G_TSEARCH_IDX_SPEED]),
				['dir'] = tonumber(val[G_TSEARCH_IDX_DIR]),
				['tokenCode'] = val[G_TSEARCH_IDX_TOKENCODE],
				['isExtra'] = isExtra,
			}

			if gps_point['tokenCode'] ~= self['tokenCode'] then	--tokenCode不相同，直接抛弃掉
				break 		--continue
			end

			dest_data[gps_point['time']] = gps_point

			--开始gps点
			if not self['startPoint'] then
				self['startPoint'] = gps_point
			elseif self['startPoint']['time'] > gps_point['time'] then
				self['startPoint'] = gps_point
			end

			--结束gps点
			if not self['endPoint'] then
				self['endPoint'] = gps_point
			elseif self['endPoint']['time'] < gps_point['time'] then
				self['endPoint'] = gps_point
			end
		until true
	end
end

--功  能: 合并正常数据与补传数据
--参  数: gps_data: 正常数据
--	  ext_data: 补传数据
--返回值: merge_data: 合并后的gps数据
function GPSFrame:merGPSData(gps_data, ext_data)
	local merge_data = {}

	self:arrayToKV(merge_data, ext_data, true)	--先转换补传数据	
	self:arrayToKV(merge_data, gps_data, false)	--后转换正常数据，覆盖补传数据进行去重。

	return merge_data
end

local function direction_sub(dir1, dir2)
	local angle = math.abs(dir1 - dir2)
	return (angle <= 180) and angle or (360 - angle)
end

--过滤GPS数据
-- 过滤原则
-- 001. 设备静止过滤，dir！=-1且前后点dir差小于30的非飘数据保留
-- 002. dir = -1的过滤，第一个-1点参加gps定位，
--      其他不参加gps定位，但是全部参与里程计算
function GPSFrame:filterGPSData(data)
	local st_idx = self['startPoint']['time']
	local ed_idx = self['endPoint']['time']
	local last_point_idx
	local last_point

	if not st_idx or not ed_idx or st_idx > ed_idx then
		only.log('E', string.format("GPSFrame:filterGPSData, st_idx > ed_idx ERROR, IMEI:%s, tokenCode:%s", 
		self['IMEI'], self['tokenCode']))
		return false
	end

	local stop_flag = false

	for i=st_idx, ed_idx do
		local gps_point = data[i]
		if not gps_point then
			goto SKIP
		end

		--方向角筛选,第一个方向角为-1 或者速度等于0的，参加计算
		if gps_point['dir'] == -1 or gps_point['speed'] == 0 then
			if stop_flag then
				data[i] = nil
				goto CONTINUE
			else
				stop_flag = true
			end
		else
			stop_flag = false
		end	

		-- 漂移点筛选
		-- 前后方向键大于30，移除
		if last_point and last_point_idx then
			if  (gps_point['time'] - last_point['time']) == 1 
				and direction_sub(gps_point['dir'], last_point['dir']) > 30 then
				data[last_point_idx] = nil
				data[i] = nil
			end
		end

		::CONTINUE::
		last_point_idx = i
		last_point = gps_point

		::SKIP::
	end

	return true
end

--功  能: 计算两点间的里程
--参  数: prev_gps: 第一个gps点
--	  next_gps: 第二个gps点
--返回值: mileage:  计算所得有效里程
--	  actualMileage: 计算所得实际里程
function GPSFrame:calcMiles(prev_gps, next_gps)
	local mileage, actualMileage = 0, 0
	local time1 = prev_gps['time']
	local speed1 = (prev_gps['dir'] ~= -1) and prev_gps['speed'] or 0
	local time2 = next_gps['time']
	local speed2 = (next_gps['dir'] ~= -1) and next_gps['speed'] or 0

	if time1 >= time2 then
		return 0, 0
	elseif (time2 - time2) > 15 then	--时间间隔大于15秒 不计算
		return 0, 0
	else
		local s = ((speed1 + speed2) / 2 ) * ( 1000 / 3600 ) * (time2 - time1)
		if prev_gps['isExtra'] and next_gps['isExtra'] then	--两个点都是补传数据，记为实际里程
			actualMileage = s
		else	--其中一个点为正常数据， 记为实际里程
			mileage	= s
		end
	end	

	return mileage, actualMileage
end

--计算途径
function GPSFrame:calcPath(data)
	local st_idx = self['startPoint']['time']
	local ed_idx = self['endPoint']['time']
	local last_road_record
	local last_point

	for i=st_idx, ed_idx do
		repeat
			local gps_point = data[i]

			if not gps_point then
				break	--continue
			end

			--计算里程
			if not last_point then
				last_point = gps_point
			else
				local mileage, actualMileage = self:calcMiles(last_point, gps_point)
				if last_road_record then
					last_road_record:addMileage(mileage, actualMileage)
				end
				last_point = gps_point
			end

			--get road id
			local road_id = "0"
			local ok,result = fun_point_match_road.entry(gps_point['dir'], gps_point['lon'] / 10000000, gps_point['lat'] / 10000000, self['accountID'])
			if ok and result then
				road_id = result['roadID'] or "0"
				d_road_id = string.sub(result['lineID'], 1, -4)	--动态roadID
			end

			if not last_road_record then
				local road_record = RoadRecord:new(road_id, d_road_id)
				road_record:addPoint(gps_point)
				last_road_record = road_record
				break	--continue
			end

			--根据road id进行数据聚合得到一个RoadRecord
			if last_road_record['roadID'] == road_id then
				last_road_record:addPoint(gps_point)
			else
				--存储到luakv中
				self['recordCount'] = self['recordCount'] + 1
				--local luakv_key = self:getRecordKey(self['recordCount'])
				--last_road_record:writeLuaKV(luakv_key)
				table.insert(self['recordSet'], last_road_record)

				local road_record = RoadRecord:new(road_id, d_road_id)
				road_record:addPoint(gps_point)
				last_road_record = road_record
			end
		until true
	end

	self['recordCount'] = self['recordCount'] + 1
	--local luakv_key = self:getRecordKey(self['recordCount'])
	--last_road_record:writeLuaKV(luakv_key)
	table.insert(self['recordSet'], last_road_record)
end

--开始处理当前frame的途径
function GPSFrame:process()
	local t1 = socket.gettime()
	local gps_data, ext_data = self:getData()
	only.log('D', string.format("IMEI:%s, tokenCode:%s, frameStart:%s, frameEnd:%s", self['IMEI'], self['tokenCode'], self['frameStart'], self['frameEnd']))
	local t2 = socket.gettime()
	only.log('D', "GPSFrame:process getData time:%s", t2 - t1)

	--合并正常数据与补传数据
	local data = self:merGPSData(gps_data, ext_data)
	gps_data, ext_data = nil, nil
	local t3 = socket.gettime()
	only.log('D', "GPSFrame:process merGPSData time:%s", t3 - t2)

	if not next(data) then
		only.log('W', string.format("merge gps data is none! IMEI:%s, tokenCode:%s", self['IMEI'], self['tokenCode']))
		return
	end

	only.log('D', "startTime:%s, endTime:%s", self['startPoint']['time'], self['endPoint']['time'])
	if (self['endPoint']['time'] - self['startPoint']['time']) > (48+1) * 3600 then
		only.log('E', "time over two days,IMEI:%s, tokenCode:%s, start:%s, end:%s",self['IMEI'], self['tokenCode'], self['startPoint']['time'], self['endPoint']['time'])
		return
	end

	--过滤
	if not self:filterGPSData(data) then
		return
	end
	local t4 = socket.gettime()
	only.log('D', "GPSFrame:process filterGPSData time:%s", t4 - t3)

	--计算途径
	self:calcPath(data)
	data = nil

	local t5 = socket.gettime()
	only.log('D', "GPSFrame:process calcPath time:%s", t5 - t4)
end

