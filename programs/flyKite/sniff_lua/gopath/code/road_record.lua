--文件名称: road_record.lua
--创建者  : 耿玄玄
--创建时间: 2015-08-25
--文件描述: 存储gopath定位道路信息类
--历史记录: 1.重构途径计算，修改按road获取数据的同时计算途径；增加补传数据计算

local only			= require ('only')
local raidus_find_poi_func	= require ('raidus_find_poi_func')
local redis_api			= require ('redis_pool_api')
local luakv_api			= require('luakv_pool_api')

local RADIUS_POI		= 200       --  poi搜索半径

module('road_record', package.seeall)

RoadRecord = {
	roadID,
	d_roadID,	--动态roadID
	mileage,
	actualMileage,
	startPoint,	--当前road开始GPS点信息
	endPoint,	--当前road结束GPS点信息
	minSpeed,
	maxSpeed,
	avgSpeed,
	speedSum,
	gpsCount,
}

function RoadRecord:new(road_id, d_road_id)
	local self = {
	}

	setmetatable(self, RoadRecord)
	RoadRecord.__index = RoadRecord

	self['roadID'] = road_id or "0"
	self['d_roadID'] = d_road_id or "0"
	self['gpsCount'] = 0
	self['speedSum'] = 0
	self['mileage'] = 0
	self['actualMileage'] = 0

	return self
end 

--功  能: 根据速度和、gps点数据计算平均速度
--参  数: speedSum: 速度累加和
--	  gpsCount: gps点总数
--返回值: 计算得出的平均速度
local function get_avg_speed(speedSum, gpsCount)
	if gpsCount == 0 then
		return 0
	else
		return math.floor(speedSum / gpsCount)
	end
end

--功  能: 将gps点添加到当前道路信息中，并更新起始点，速度等信息
--参  数: gps_point: gps点数据
function RoadRecord:addPoint(gps_point)
	if not gps_point or not next(gps_point) then
		only.log('E', "RoadRecord:addPoint ERROR")
		return
	end

	--开始点比较
	if not self['startPoint'] or self['startPoint']['time'] > gps_point['time'] then
		self['startPoint'] = gps_point
	end

	--结束点比较
	if not self['endPoint'] or self['endPoint']['time'] < gps_point['time'] then
		self['endPoint'] = gps_point
	end

	--最小速度比较
	if not self['minSpeed'] or self['minSpeed'] > gps_point['speed'] then
		self['minSpeed'] = gps_point['speed']
	end

	--最大速度比较
	if not self['maxSpeed'] or self['maxSpeed'] < gps_point['speed'] then
		self['maxSpeed'] = gps_point['speed']
	end

	--累计速度与gps点数(用于计算平均速度)
	self['speedSum'] = self['speedSum'] + gps_point['speed']
	self['gpsCount'] = self['gpsCount'] + 1
	self['avgSpeed'] = get_avg_speed(self['speedSum'], self['gpsCount'])
end

--功  能: 将一个RoadRecord合并到当前RoadRecord中，并更新起始点，速度等信息
--参  数: record: 待合并的RoadRecord信息
function RoadRecord:addRecord(record)
	if not record or not record['roadID'] then
		only.log('E', "RoadRecord:addRecord ERROR")
		return
	end

	if self['roadID'] == '0' then
		self['roadID'] = record['roadID']
	end

	--开始点比较
	if not self['startPoint'] or self['startPoint']['time'] > record['startPoint']['time'] then
		self['startPoint'] = record['startPoint']
	end

	--结束点比较
	if not self['endPoint'] or self['endPoint']['time'] < record['endPoint']['time'] then
		self['endPoint'] = record['endPoint']
	end

	--最小速度比较
	if not self['minSpeed'] or self['minSpeed'] > record['minSpeed'] then
		self['minSpeed'] = record['minSpeed']
	end

	--最大速度比较
	if not self['maxSpeed'] or  self['maxSpeed'] < record['maxSpeed'] then
		self['maxSpeed'] = record['maxSpeed']
	end

	--累计速度与gps点数(用于计算平均速度)
	self['speedSum'] = self['speedSum'] + record['speedSum']
	self['gpsCount'] = self['gpsCount'] + record['gpsCount']

	--累计有效里程
	self['mileage'] = self['mileage'] + record['mileage']
	--累计实际里程
	self['actualMileage'] = self['actualMileage'] + record['actualMileage']
	self['avgSpeed'] = get_avg_speed(self['speedSum'], self['gpsCount'])
end

--功  能: 将里程累加到当前记录中
--参  数: mileage: 计算所得有效里程
--	  actualMileage: 计算所得实际里程
function RoadRecord:addMileage(mileage, actualMileage)
	self['mileage'] = self['mileage'] + mileage
	self['actualMileage'] = self['actualMileage'] + mileage + actualMileage
end

function RoadRecord:writeLuaKV(key)
	if not key then
		return 
	end
	
	local t1 = socket.gettime()
	
--	only.log('D', "RoadRecord:writeLuaKV %s", scan.dump(self))

	self['recordKey'] = key
	local ok, _ = luakv_api.cmd('local', '', 'hmset', key, 
	'roadID',	self['roadID'],
	'd_roadID',	self['d_roadID'],
	'mileage',	self['mileage'],
	'actualMileage',self['actualMileage'],
	'startTime',	self['startPoint']['time'],
	'startLon',	self['startPoint']['lon'],
	'startLat',	self['startPoint']['lat'],
	'startDir',	self['startPoint']['dir'],
	'startSpeed',	self['startPoint']['speed'],
	'endTime',	self['endPoint']['time'],
	'endLon',	self['endPoint']['lon'],
	'endLat',	self['endPoint']['lat'],
	'endDir',	self['endPoint']['dir'],
	'endSpeed',	self['endPoint']['speed'],
	'minSpeed',	self['minSpeed'],
	'maxSpeed',	self['maxSpeed'],
	'avgSpeed',	self['avgSpeed'],
	'gpsCount', self['gpsCount']
	)

	if not ok then
		only.log('E', string.format("RoadRecord:writeLuaKV ERROR, key:%s, roadID:%s", key, self['roadID']))
	end

	local t2 = socket.gettime()
	only.log('D', "RoadRecord:writeLuaKV time %s", t2 - t1)
end

function RoadRecord:readLuaKV(key)
	if not key then
		return 
	end

	local t1 = socket.gettime()
	self['recordKey'] = key
	local ok, ret = luakv_api.cmd('local', '', 'hmget', key, 
	'roadID',
	'd_roadID',
	'mileage',
	'actualMileage',
	'startTime',
	'startLon',
	'startLat',
	'startDir',
	'startSpeed',
	'endTime',
	'endLon',
	'endLat',
	'endDir',
	'endSpeed',
	'minSpeed',
	'maxSpeed',
	'avgSpeed',
	'gpsCount'
	)

	if not ok or #ret ~= 18 then
		only.log('E', string.format("RoadRecord:readLuaKV ERROR, key:%s, roadID:%s", key, self['roadID']))
	end

	self['roadID'] 	= ret[1]
	self['d_roadID'] 	= ret[2]
	self['mileage'] = tonumber(ret[3])
	self['actualMileage'] = tonumber(ret[4])
	self['startPoint'] = {
		['time'] 	= tonumber(ret[5]),
		['lon']	= tonumber(ret[6]),
		['lat']	= tonumber(ret[7]),
		['dir']	= tonumber(ret[8]),
		['speed']	= tonumber(ret[9]),
	}
	self['endPoint'] = {
		['time'] 	= tonumber(ret[10]),
		['lon']	= tonumber(ret[11]),
		['lat']	= tonumber(ret[12]),
		['dir']	= tonumber(ret[13]),
		['speed']	= tonumber(ret[14]),
	}
	self['minSpeed'] = tonumber(ret[15])
	self['maxSpeed'] = tonumber(ret[16])
	self['avgSpeed'] = tonumber(ret[17])
	self['gpsCount'] = tonumber(ret[18])

	local t2 = socket.gettime()
	only.log('D', "RoadRecord:readLuaKV time %s", t2 - t1)
--	only.log('D', "RoadRecord:readLuaKV %s", scan.dump(self))
end

function RoadRecord:remLuaKV()
	if not self['recordKey'] then
		only.log('E', "RoadRecord:remLuaKV ERROR")
		return
	end

	local t1 = socket.gettime()
	local ok, _ = luakv_api.cmd('local', '', 'del', self['recordKey'])
	if not ok then
		only.log('E', string.format("RoadRecord:remLuaKV ERROR, key:%s, roadID:%s", self['recordKey'], self['roadID']))
	end
	local t2 = socket.gettime()
	only.log('D', "RoadRecord:remLuaKV time %s", t2 - t1)
end

--获取道路详细信息
local function gps_data_get_road_info(roadID)
	if roadID == 0 then
		return nil
	end
	local t1 = socket.gettime()
	local key = roadID .. ":roadInfo"
	local ok, kv_tab = redis_api.cmd('mapRoadInfo', "", 'hgetall', key)

	if  ok and kv_tab then
		return kv_tab
	else
		only.log('W', "[hgetall road info error]")
	end
	local t2 = socket.gettime()
	only.log('D', "RoadRecord:gps_data_get_road_info time %s", t2 - t1)
	return nil
end

--获取周围POI
local function get_nearby_poi(lon, lat)
	local t1 = socket.gettime()
	local tb = {
		longitude = lon / 10000000,
		latitude = lat / 10000000,
		number = 1,
		radius = RADIUS_POI,
	}
	local res = raidus_find_poi_func.handle(tb)

	local t2 = socket.gettime()
	only.log('D', "RoadRecord:get_nearby_poi time %s", t2 - t1)
	if res then
		return res['name']
	end

	return ""
end

--功  能: 将当前RoadRecord转换为回调接口定义的格式
--返回值: road_info: 转换完成的对象
function RoadRecord:getRoadInfo()
	local t1 = socket.gettime()
	local road_info = {}
	local st_point = self['startPoint']
	local ed_point = self['endPoint']

	road_info['mileage']		= math.floor(self['mileage'])
	road_info['actualMileage']	= math.floor(self['actualMileage'])
	road_info['roadID']		= self['roadID']
	road_info['startPOIName']	= get_nearby_poi(st_point['lon'], st_point['lat']) or ''
	road_info['startLongitude']	= st_point['lon']
	road_info['startLatitude']	= st_point['lat']
	road_info['startTime']		= st_point['time']
	road_info['endPOIName']		= get_nearby_poi(ed_point['lon'], ed_point['lat']) or ''
	road_info['endLongitude']	= ed_point['lon']
	road_info['endLatitude']	= ed_point['lat']
	road_info['endTime']		= ed_point['time']
	road_info['minimumSpeed']	= self['minSpeed'] or 0
	road_info['maximumSpeed']	= self['maxSpeed'] or 0
	road_info['averageSpeed']	= self['avgSpeed'] or 0

	if road_info['roadID'] ~= '0' then
		local kv_tab = gps_data_get_road_info(self['roadID'])
		if kv_tab then
			road_info['roadName']        =   kv_tab['RN'] or "newRoad"
			road_info['provinceCode']    =   kv_tab['PC']
			road_info['countyCode']      =   kv_tab['CC']
			road_info['limitSpeed']      =   kv_tab['SR']
			road_info['provinceName']    =   kv_tab['PN']
			road_info['roadType']        =   kv_tab['RT']
			road_info['cityName']        =   kv_tab['CA']
			road_info['roadRootID']      =   '0'
			road_info['countyName']      =   kv_tab['CN']
			road_info['cityCode']        =   kv_tab['CD']
		end
	end
	road_info['roadName'] = road_info['roadName'] or "newRoad"
	local t2 = socket.gettime()
	only.log('D', "RoadRecord:getRoadInfo time %s", t2 - t1)

	if not next(road_info) then
		return nil
	else
		return road_info
	end
end

