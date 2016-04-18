--文件名称: road_record.lua
--创建者  : 刘勇跃
--创建时间: 2016-04-10
--文件描述: 存储定位道路信息类

local only			= require ('only')
local redis_api			= require ('redis_pool_api')
local mysql_api			= require ('mysql_pool_api')
local switch                    = require ('config_half_full')

module('road_record', package.seeall)

local ROADTYPE = {
	[0]  = "高速",
	[1]  = "国道",
	[2]  = "省道",
	[3]  = "县道",
	[4]  = "乡道",
	[5]  = "村道",
	[10] = "城市快速路",
	[11] = "城市主干道",
	[12] = "城市次干道"
}
local MAXSPEED = {
	[0]  = 120,
	[1]  = 80,
	[2]  = 70,
	[3]  = 60,
	[4]  = 50,
	[5]  = 30,
	[10] = 80,
	[11] = 60,
	[12] = 40
}

local FULL_STR = '０１２３４５６７８９ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ'

local HALF_TAB = {
	[1] = '0',
	[2] = '1',
	[3] = '2',
	[4] = '3',
	[5] = '4',
	[6] = '5',
	[7] = '6',
	[8] = '7',
	[9] = '8',
	[10] = '9',
	[11] = 'A',
	[12] = 'B',
	[13] = 'C',
	[14] = 'D',
	[15] = 'E',
	[16] = 'F',
	[17] = 'G',
	[18] = 'H',
	[19] = 'I',
	[20] = 'J',
	[21] = 'K',
	[22] = 'L',
	[23] = 'M',
	[24] = 'N',
	[25] = 'O',
	[26] = 'P',
	[27] = 'Q',
	[28] = 'R',
	[29] = 'S',
	[30] = 'T',
	[31] = 'U',
	[32] = 'V',
	[33] = 'W',
	[34] = 'X',
	[35] = 'Y',
	[36] = 'Z',
	[37] = 'a',
	[38] = 'b',
	[39] = 'c',
	[40] = 'd',
	[41] = 'e',
	[42] = 'f',
	[43] = 'g',
	[44] = 'h',
	[45] = 'i',
	[46] = 'j',
	[47] = 'k',
	[48] = 'l',
	[49] = 'm',
	[50] = 'n',
	[51] = 'o',
	[52] = 'p',
	[53] = 'q',
	[54] = 'r',
	[55] = 's',
	[56] = 't',
	[57] = 'u',
	[58] = 'v',
	[59] = 'w',
	[60] = 'x',
	[61] = 'y',
	[62] = 'z'
}

RoadRecord = {
	CollectTime,    --GPSTime
	Longitude,     
	Latitude,
	Altitude,
	Direction,
	Speed,
	Mileage,        
	A,              --acceletation
	Uid,            --tokenCode
	Name,           --roadName
	RoadType,       --RT
	OverSpeed,      
	OverPercent,
	countyCode,
	countyName,
	Pname,
	cityName,
	Weather,
	Temperature,
}

function RoadRecord:new()
	local self = {
	}

	setmetatable(self, RoadRecord)
	RoadRecord.__index = RoadRecord
	
	self['CollectTime'] = 'null'
	self['Longitude']   = 'null'
	self['Latitude']    = 'null'
	self['Altitude']    = 'null'
	self['Direction']   = 'null'
	self['Speed']       = 'null'
	self['Mileage']     = 0
	self['A']           = 0.0
	self['Uid']         = 'null'
	self['Name']        = 'null'
	self['RoadType']    = 'null'
	self['OverSpeed']   = 'null'
	self['OverPercent'] = 'null'
	self['countyCode']  = 'null'
	self['countyName']  = 'null'
	self['cityName']    = 'null'
	self['PName']       = 'null'
	self['Weather']     = 'null'
	self['Temperature'] = 'null'

	return self
end 

--功  能: 全角符号转换半角
local function SBC2DBC(start_str)
	if not start_str then
		return nil
	end
	local n = string.len(start_str) / 3
	local end_str = ''
	local j = 1
	for i = 1, n do
		local sub = string.sub(start_str,j,j+2)
		local k   = string.find(FULL_STR,sub)
		if k then
			local k_str = HALF_TAB[(k-1)/3 + 1] 
			end_str = end_str..k_str
		else
			end_str = end_str..sub
		end
		j = j + 3
	end
	n,j = nil,nil
	return end_str
end

--功  能: 获取道路信息
--参  数: IMEI, tokenCode, GPSPoint
function RoadRecord:getRoadInfo(IMEI, tokenCode, gps_point)
	local gpstime       = gps_point['create']

	self['Longitude']   = gps_point['lon'] / 10000000
	self['Latitude']    = gps_point['lat'] / 10000000
	self['Direction']   = gps_point['dir']
	self['Altitude']    = gps_point['alt']
	self['Speed']       = gps_point['speed']
	self['Uid']         = tokenCode
	self['CollectTime'] = os.date('%Y-%m-%d %H:%M:%S', gps_point['time'])

	local redis_arg = {
		['Longitude']   = gps_point['lon'] / 10000000,
		['Latitude']    = gps_point['lat'] / 10000000,
		['Direction']   = gps_point['dir'],
		['Altitude']    = gps_point['alt'],
		['Speed']       = gps_point['speed'],
		['Uid']         = tokenCode,
		['imei']        = IMEI,
		['gpstime']     = gpstime
	}

	--PMR
	local ok, ret = redis_api.cmd('kkb_redis','','hmget','MLOCATE',tonumber(IMEI),self['Longitude'],self['Latitude'],self['Direction'],self['Altitude'],self['Speed'],gpstime)
	if not ok or #ret == 0 then
		only.log('E','PMR error!'..scan.dump(redis_arg))	
	end
	only.log('D','PMR is '..scan.dump(ret)..#ret)
	local rt = tonumber(ret[5])
	self['RoadType']   = ROADTYPE[rt]
	self['Name']       = SBC2DBC(ret[6])
	ok,ret = nil,nil
	
	--get overspeed info
	if not MAXSPEED[rt] then
		only.log('D',string.format('%s roadtype error!',self['Name']))   --检查道路数据
	elseif self['Speed'] <= MAXSPEED[rt] then
		self['OverSpeed']   = '否'
		self['OverPercent'] = 'null'
	else
		self['OverSpeed']   = '是'
		local limit = MAXSPEED[rt]
		local overpercent   = math.ceil(((self['Speed'] - limit) / limit) * 100)
		self['OverPercent'] = tostring(overpercent)..'%' 
		rt,limit,overpercent = nil,nil,nil
	end

	--countyInfo
	local lon_100 = math.floor(self['Longitude'] * 100)
	local lat_100 = math.floor(self['Latitude'] * 100)
	local key     = tostring(lon_100)..'&'..tostring(lat_100)
	local ok, ret = redis_api.cmd('county_100_redis','','hgetall',key)
	if not ok or not ret then
		only.log('E','HGETALL FROM REDIS ERROR!')
	end
	
	self['countyCode'] = ret['countyCode']
	self['countyName'] = ret['countyName']
	self['cityName']   = ret['cityName']
	self['PName']      = ret['PName']
	lon_100,lat_100,key,ok,ret = nil,nil,nil,nil,nil

	--get weather info
	if math.floor(self['countyCode'] / 10000) == 11 then
		citycode = 110000     --北京市
	elseif math.floor(self['countyCode'] / 10000) == 12 then
		citycode = 120000     --天津市
	elseif math.floor(self['countyCode'] / 10000) == 31 then
		citycode = 310000     --上海市
	elseif math.floor(self['countyCode'] / 10000) == 50 then
		citycode = 500000     --重庆市
	else
		citycode = math.floor((self['countyCode']) / 100) * 100   --转换成cityCode
	end

	local cur_time = string.sub(self['CollectTime'],1,10)..'%%'
	local select_weather_sql = string.format(
			"SELECT text, temperature FROM 天气信息 WHERE cityCode = %d and lastUpdate LIKE '%s';",
			citycode, cur_time
		)
	local ok, ret_weather = mysql_api.cmd('kkb_sql','SELECT',select_weather_sql)
	if not ok or #ret_weather == 0 then
		only.log('E', 'SELECT weather ERROR!'..scan.dump(select_weather_sql))
		self['Weather'] = ''
		self['Temperature'] = ''
	else
		--获取当天天气温度范围
		local min, max, wea
		for i = 1, #ret_weather do
			if i == 1 then
				local cur = tonumber(ret_weather[i]['temperature'])
				wea = ret_weather[i]['text']
				min = cur
				max = cur
			else
				local cwe = ret_weather[i]['text']
				if not string.find(wea, cwe, -1, true) then
					wea = wea..'转'..cwe
				end	
				local cur = tonumber(ret_weather[i]['temperature'])
				min = cur < min and cur or min
				max = cur > max and cur or max
				only.log('D',string.format('cur is %s,min is %s, max is %s',cur,min,max))
			end
		end
		self['Weather'] = ret_weather[1]['text']
		self['Temperature'] = string.format('%s ~ %s', min, max )
	end
	citycode,cur_time,select_weather_sql,ok,ret_weather = nil,nil,nil,nil,nil,nil
end

--功  能: 将里程累加到当前记录中
--参  数: mileage: 计算所得有效里程
function RoadRecord:addMileage(mileage)
	self['Mileage'] = mileage 
end

--功  能: 将加速度添加到当前记录中
--参  数: acceleration: 计算所得加速度
function RoadRecord:addAcceleration(acceleration)
	self['A'] = acceleration
end
