--版权声明：无
--文件名称：data_package.lua
--创建者  ：耿玄玄
--创建日期：2015-06-27
--文件描述：GPS数据包类，对gps数据包进行处理
--修    改：2015-06-27 重构实时里程

local supex = require('supex')
local utils = require('utils')
local scan = require('scan')
local only = require('only')
local redis_api = require('redis_pool_api')
local cachekv = require('cachekv')

local DEFS = require('realtime_defs')
local GPSPointModule = require("gps_point")
local GPSPoint = GPSPointModule.GPSPoint
local MilesPackageModule = require("miles_package")
local MilesPackage = MilesPackageModule.MilesPackage
local MilesRecordModule = require("miles_record")
local MilesRecord = MilesRecordModule.MilesRecord
local luakv_api     = require('luakv_pool_api')

module('data_package', package.seeall)

DataPackage = {
        M,
        tokenCode,
        accountID,
        createTime,     --服务器接收时间
        GPSCount,      --数据包中的GPS点数
        maxSpeed,       --数据包中最大速度
        minSpeed,       --数据包中最小速度
        startPoint,     --数据包中开始时间gps点信息，GPSPoint类型
        endPoint,       --数据包中结束时间gps点信息，GPSPoint类型
        lastPoint,      --上个包中最后时间gps点信息，GPSPoint类型
        points,      --数据包中所有gps点信息，GPSPoint类型数组
        isDelay,        -- 后发先至数据
        hasExtra,        -- 补传数据
        isTokenChanged, --tokenCode是否变化
        normal,         --异常停车标识
}

-- Return new object of DataPackage
function DataPackage:new()
        local self = {
                }

        setmetatable(self, DataPackage)
        DataPackage.__index = DataPackage

        return self
end

--processLastTokenCode
--功能： 处理上个里程数据，将里程记录key移到已完成list中
--参数：last_tokenCode，上次已完成的tokenCode
--返回：
--修改：2015-06-27 重构实时里程
function DataPackage:processLastTokenCode(last_tokenCode)
        --.处理上次tokenCode，移到sorted set中
        -- add mileage to FINISH_MILE_ZSET
        local last_mileage_key = string.format("%s:%s:mileage", self['M'], last_tokenCode)
        local set_ok,_ = redis_api.cmd('mapRTMileage', self['M'] or '',  "zadd", DEFS['STATIC']['FINISH_MILE_ZSET'], self['startPoint']['GPSTime'] or 0, last_mileage_key)

        if not set_ok then
                only.log('W', string.format("key>>%s, add to FINISH_MILE_ZSET error", last_mileage_key))
        end

        -- delete this mileage from LASTED_MILE_ZSET
        local rem_ok, _ = redis_api.cmd('mapRTMileage', self['M'] or '', "ZREM", DEFS['STATIC']['LASTED_MILE_ZSET'], last_mileage_key)

        if not rem_ok then
                only.log('W', string.format("key>>%s, rem from LASTED_MILE_ZSET error", last_mileage_key))
        end
end

--名称：isTokenChanged
--功能：检查是否是开机数据包，如果tokenCode发生变化，则认为是
--返回：boolean:   true --> 是开机数据包
--                false --> 不是开机数据包
--修改：2015-06-27 重构实时里程
function DataPackage:isTokenChanged()
        local ret = false
        local key = self['M'] .. ":tokenCode"
        -- get last tokenCode from redis
        local ok, last_tokenCode = redis_api.cmd('owner', self['M'] or '',  "get", key)
        if not ok or not last_tokenCode or last_tokenCode == "" then
              ret = true
        end

        --tokenCode发生变化，则认为是开机
        if (not last_tokenCode) or last_tokenCode ~= self['tokenCode'] then
                if (not self['isDelay']) then
                        redis_api.cmd('owner', self['M'] or '',  "set", key, self['tokenCode'])
                end

                return true, last_tokenCode
        else
                return false, nil
        end
end

--名称：writeLastPoint
--功能：写入最新点的数据到redis中
--参数2：last_point --> GPSPoint, 数据包中的最新点
--返回：无
--修改：2015-07-01 重构实时里程
function DataPackage:writeLastPoint(last_point)       
        local last_time_key = string.format("%s:lastTime", self['M'])
        local last_lon_key = string.format("%s:lastLon", self['M'])
        local last_lat_key = string.format("%s:lastLat", self['M'])
        local last_speed_key = string.format("%s:lastSpeed", self['M'])
        local last_dir_key = string.format("%s:lastDir", self['M'])
        
        local ok, _ = luakv_api.cmd('localRedis', self['M'] or '', 
                {
                        {'set', last_time_key, last_point['GPSTime']},
                        {'set', last_lon_key, last_point['longitude']},
                        {'set', last_lat_key, last_point['latitude']},
                        {'set', last_speed_key, last_point['speed']},
                        {'set', last_dir_key, last_point['direction']}
                })
        
        if not ok then
                only.log('E', string.format("DataPackage:writeLastPoint ERROR, %s", last_time_key))
        end
end

--名称：readLastPoint
--功能：检查数据是否超时
--返回：last_point --> 上个包数据的最新时间点信息，GPSPoint
--修改：2015-07-01 重构实时里程
function DataPackage:readLastPoint()
        local last_time_key = string.format("%s:lastTime", self['M'])
        local last_lon_key = string.format("%s:lastLon", self['M'])
        local last_lat_key = string.format("%s:lastLat", self['M'])
        local last_speed_key = string.format("%s:lastSpeed", self['M'])
        local last_dir_key = string.format("%s:lastDir", self['M'])
        
        local ok, ret = luakv_api.cmd('localRedis', self['M'] or '', 
                {
                        {'get', last_time_key},
                        {'get', last_lon_key},
                        {'get', last_lat_key},
                        {'get', last_speed_key},
                        {'get', last_dir_key}
                })
        
        if (not ok) or (not ret) or (#ret ~= 5) then
                only.log('E', "DataPackage:readLastPoint ERROR")
                return nil
        else
                local last_point = GPSPoint:new()
                last_point:init{GPSTime=ret[1], longitude=ret[2], latitude=ret[3], speed=ret[4], direction=ret[5]}
                return last_point
        end
end

--名称：init_body
--功能：初始化实时数据或者补传数据
--返回：req_body   实时或补传gps数据
--修改：2015-07-01 重构实时里程
function DataPackage:init_body(req_body)
        local tmpPoints = {}
        local begin_index, end_index = 1, 1
        local gps_count = #req_body['GPSTime']
        for key,val in ipairs(req_body['GPSTime']) do
                local ptTime = req_body['GPSTime'][key]
                local p = GPSPoint:new()
                p:init{GPSTime=ptTime,longitude=req_body['longitude'][key],latitude=req_body['latitude'][key],
                        speed=req_body['speed'][key],direction=req_body['direction'][key]}

                --GPSTime作为key， GPSPoint对象作为value
                tmpPoints[ptTime] = p

                --获得本次开始和结束时间
                if ptTime < req_body['GPSTime'][begin_index] then
                        begin_index = key
                end

                if ptTime > req_body['GPSTime'][end_index] then
                        end_index = key
                end
        end
        
        local start_point = GPSPoint:new()
        start_point:init{GPSTime = req_body['GPSTime'][begin_index], longitude = req_body['longitude'][begin_index],
                latitude = req_body['latitude'][begin_index], speed = req_body['speed'][begin_index],
                direction = req_body['direction'][begin_index]}

        --结束时间
        local end_point = GPSPoint:new()
        end_point:init{GPSTime = req_body['GPSTime'][end_index], longitude = req_body['longitude'][end_index],
                latitude = req_body['latitude'][end_index], speed = req_body['speed'][end_index],
                direction = req_body['direction'][end_index]}
        
        --TODO 检查是否超时
        --[[
        if gps_count > 4200 or (os.time() - end_point['GPSTime']) > 4200 then
                return false, nil, nil, 0
        end
        ]]--
        
        return true, tmpPoints, start_point, end_point, gps_count
end

--名称：init
--功能：根据gps数据，初始化DataPackage，此函数是初始化实时数据和补传数据的公共函数
--参数：req_body --> GPS数据body
--返回：无
--修改：2015-06-27 重构实时里程
function DataPackage:init(req_body)
        self['M']        = req_body['M']
        self['tokenCode']    = req_body['tokenCode']
        self['accountID']    = req_body['accountID']
        self['createTime']   = os.time()

        local ok, tmpPoints, start_point, end_point, gps_count
        
        --实时数据初始化
        if #req_body['GPSTime'] > 0 then
                ok, tmpPoints, start_point, end_point, gps_count = self:init_body(req_body)
                self['points'] = tmpPoints
                self['startPoint'] = start_point
                self['endPoint'] = end_point
                self['GPSCount'] = gps_count
        end
        
        --补传数据初始化
        if req_body['extragps'] and #req_body['extragps']['GPSTime'] > 0 then
                ok, tmpPoints, start_point, end_point, gps_count = self:init_body(req_body['extragps'])
                if ok then 
                        self['hasExtra'] = true  --包含补传数据
                        self['extraPoints'] = tmpPoints
                        self['extraStartPoint'] = start_point
                        self['extraEndPoint'] = end_point
                        self['extraGPSCount'] = gps_count
                end
        end        

        --检查tokeCode是否发生变化
        self['isTokenChanged'], self['lastTokenCode'] = self:isTokenChanged()
        if not self['isTokenChanged'] then
                self['lastPoint'] = self:readLastPoint()
                
                if self['lastPoint'] and self['endPoint'] 
                        and self['lastPoint']['GPSTime'] > self['endPoint']['GPSTime']
                then
                        self['isDelay'] = true --先发后至的数据
                end
        end
        

        return true
end

--名称：checkData
--功能：检查数据
--返回：无
--修改：2015-06-27 重构实时里程
function DataPackage:checkData()
        if not  self['M'] then
                only.log('E', "check M fail")
                return false
        end

        if not self['tokenCode'] then
                only.log('E', "tokenCode is nil")
                return false
        end

        return true
end

--名称：writeLossPoints
--功能：记录实时数据中丢数据的范围
--参数：begin_point -- gps丢失范围起始点
--      end_point     gps丢失范围终止点
--返回：无
--修改：2015-07-01 重构实时里程
function DataPackage:writeLossPoints(begin_point, end_point)
        only.log('D', string.format("DataPackage:writeLossPoints, begin: %s; end: %s", begin_point['GPSTime'], end_point['GPSTime']))
        local begin_time_key = string.format("%s:lossBeginTime", self['M'])
        local begin_speed_key = string.format("%s:lossBeginSpeed", self['M'])
        local begin_dir_key = string.format("%s:lossBeginDir", self['M'])
        
        local end_time_key = string.format("%s:lossEndTime", self['M'])
        local end_speed_key = string.format("%s:lossEndSpeed", self['M'])
        local end_dir_key = string.format("%s:lossEndDir", self['M'])
        
        local ok, _ = luakv_api.cmd('localRedis', self['M'] or "",
                {
                        {"set", begin_time_key, begin_point['GPSTime']}, 
                        {"set", begin_speed_key, begin_point['speed']}, 
                        {"set", begin_dir_key, begin_point['direction']},
                        {"set", end_time_key, end_point['GPSTime']}, 
                        {"set", end_speed_key, end_point['speed']}, 
                        {"set", end_dir_key, end_point['direction']}
                })

        
        if not ok then
                only.log('E', string.format("DataPackage:writeLossPoints error"))
        end

        --TODO设置超时4200秒
end

--名称：readLossPoints
--功能：读取丢失数据范围，并且去除补传数据中的重复数据
--参数：points_array	需要进行去重的gps点数组
--	range_begin_point  查询开始点
--     range_end_point   查询结束点
--返回：去重后的gps点数组
--修改：2015-07-01 重构实时里程
function DataPackage:readLossPoints(points_array, range_begin_point, range_end_point)
        if not points_array then 
                return {}
        end
        
        local ret_points = {}
        
        local begin_point, end_point
        local begin_time_key = string.format("%s:lossBeginTime", self['M'])
        local begin_speed_key = string.format("%s:lossBeginSpeed", self['M'])
        local begin_dir_key = string.format("%s:lossBeginDir", self['M'])
        
        local end_time_key = string.format("%s:lossEndTime", self['M'])
        local end_speed_key = string.format("%s:lossEndSpeed", self['M'])
        local end_dir_key = string.format("%s:lossEndDir", self['M'])

        local ok, ret = luakv_api.cmd('localRedis', self['M'] or "", 
                {
        	        {"get", begin_time_key}, 
        	        {"get", begin_speed_key}, 
        	        {"get", begin_dir_key}, 
        	        {"get", end_time_key}, 
        	        {"get", end_speed_key}, 
        	        {"get", end_dir_key}
                })
        
        if not ok or not ret or (#ret ~= 6) then
                return ret_points
        end
        
        begin_point = GPSPoint:new()
        begin_point:init{GPSTime=ret[1], speed=ret[2], direction=ret[3]}
        
        end_point = GPSPoint:new()
        end_point:init{GPSTime=ret[4], speed=ret[5], direction=ret[6]}
        
        only.log('D', string.format("DataPackage:readLossPoints, begin: %s; end: %s", ret[1], ret[4]))
        
        local begin_time = begin_point['GPSTime'] + 1
        local end_time = end_point['GPSTime'] - 1
        
        if begin_time > end_time then
                return ret_points
        end
        
        table.insert(ret_points, begin_point)   --先插入第一个点
        
        --插入GPS数据包中的点
        for time = begin_time, end_time do
                if points_array[time] then
                        table.insert(ret_points, points_array[time])
                end
        end
        
        table.insert(ret_points, end_point)     --插入最后一个点
        
        if #ret_points > 4 then
                --删除丢失信息，防止重复补传
                self:remLossPoints()
        end

        return ret_points
end

function DataPackage:remLossPoints()
        local begin_time_key = string.format("%s:lossBeginTime", self['M'])
        local begin_speed_key = string.format("%s:lossBeginSpeed", self['M'])
        local begin_dir_key = string.format("%s:lossBeginDir", self['M'])
        
        local end_time_key = string.format("%s:lossEndTime", self['M'])
        local end_speed_key = string.format("%s:lossEndSpeed", self['M'])
        local end_dir_key = string.format("%s:lossEndDir", self['M'])
        local ok, ret = luakv_api.cmd('localRedis', self['M'] or "", 
                {
        	        {"del", begin_time_key}, 
        	        {"del", begin_speed_key}, 
        	        {"del", begin_dir_key}, 
        	        {"del", end_time_key}, 
        	        {"del", end_speed_key}, 
        	        {"del", end_dir_key}
                }
        )
        
        if not ok then
                only.log('E', "DataPackage:remLossPoints ERROR, M:%s", self['M'])
        end
end

--名称：getExtraPoints
--功能：获取补传数据gps计算点数组
--返回：参与里程计算的补传gps点数组
--修改：2015-07-01 重构实时里程
function DataPackage:getExtraPoints()
        local ret_points = {}
        if (not self['isDelay']) and self['lastPoint'] then   
                --先发后置数据，补传数据与上个点之间去重
                local begin_time = self['lastPoint']['GPSTime'] + 1
                local end_time = self['startPoint']['GPSTime'] - 1
                if begin_time >= end_time then
                        return ret_points
                end
                
		self['lastPoint']['repeat'] = true
                table.insert(ret_points, self['lastPoint'])
                self['lastPoint'] = self['extraEndPoint']       
                --lastpoint参与补传计算，使用补传数据最后一个点作为接下来的实时数据的计算
                for time = begin_time, end_time do
                        if self['extraPoints'][time] then
                                table.insert(ret_points, self['extraPoints'][time])
                        end
                end
        else --后发先至的数据包，需要将整个补传数据包进行去重 
                ret_points = self:readLossPoints( self['extraPoints'], self['extraStartPoint'], self['extraEndPoint'])
        end
        
        return ret_points
end

--名称：getRealPoints
--功能：获取实时数据gps计算点数组
--返回：参与里程计算的实时gps点数组
--修改：2015-07-01 重构实时里程
function DataPackage:getRealPoints()
        local ret_points = {}
        if self['isDelay'] then
                ret_points = self:readLossPoints( self['points'], self['startPoint'], self['endPoint'], true)
                return ret_points
        elseif self['lastPoint'] then     --非先发后至数据，插入上个包的最新点参与里程计算
                self['lastPoint']['repeat'] = true
                table.insert(ret_points, self['lastPoint'])
        end
        
        local begin_time = self['startPoint']['GPSTime']
        local end_time = self['endPoint']['GPSTime']
        --插入GPS数据包中的点
        for time = begin_time, end_time do
                if self['points'][time] then
                        table.insert(ret_points, self['points'][time])
                end
        end
        
        return ret_points
end

--名称：longDriveMiles
--功能：判断实时里程计算是否达到整数公里,传递给连续驾驶模块
--参数：old_actual_miles	上一次实际里程
--	new_actual_miles	本次实际里程
--	new_sum_miles		本次有效里程
--	max_speed		累积最大速度
--	avg_speed		累积平均速度
--	stop_time		累积停车时间
--返回：无
--修改：2015-07-01 重构实时里程
function DataPackage:longDriveMiles(old_actual_miles, new_actual_miles, new_sum_miles, max_speed, avg_speed, stop_time)
        local old_floor = math.floor(old_actual_miles/1000)
        local new_floor = math.floor(new_actual_miles/1000)

	if old_actual_miles == 0 and new_actual_miles > 0 then	--0公里时传递给连续驾驶模块
		local long_dri_miles = {}
		long_dri_miles['sumMileage'] = math.floor(new_sum_miles/1000) --有效里程，整数公里
		long_dri_miles['actualMileage'] = new_floor	--实际里程，整数公里
		long_dri_miles['maxSpeed'] = max_speed
		long_dri_miles['avgSpeed'] = avg_speed
		long_dri_miles['stopTime'] = stop_time

                _G.add_origin_key_value("T_LONGDRI_MILEAGE", long_dri_miles)	--连续驾驶提醒
        elseif new_floor > old_floor then
		local long_dri_miles = {}
		long_dri_miles['sumMileage'] = math.floor(new_sum_miles/1000) --有效里程，整数公里
		long_dri_miles['actualMileage'] = new_floor	--实际里程，整数公里
		long_dri_miles['maxSpeed'] = max_speed
		long_dri_miles['avgSpeed'] = avg_speed
		long_dri_miles['stopTime'] = stop_time

                _G.add_origin_key_value("T_LONGDRI_MILEAGE", long_dri_miles)	--连续驾驶提醒
        end
end

--名称：longDriveMiles
--功能：判断实时里程计算是否达到整数公里,传递给连续驾驶模块
--参数：drive_time	驾驶时长
--返回：无
--修改：2015-07-01 重构实时里程
function DataPackage:driveOnlineTime(drive_time)
	local key = string.format("%s:DriveHours", self['M'])

	if self['isTokenChanged'] then	--重新开机，重置时间为0
		redis_api.cmd('owner', self['M'] or '',  "set", key, 0)
	else	--计算开机时间，达到整数时间则传递给疲劳驾驶模块
		local hours = math.floor(drive_time/3600) 
		if hours > 0 and (drive_time % 3600 < 300 ) then --在线大于1小时，且在这一小时的前10分钟内触发
			local ok, last_hours = redis_api.cmd('owner', self['M'] or '',  "get", key)
			if ok and tonumber(last_hours or 0) < hours then
                                redis_api.cmd('owner', self['M'] or '',  "set", key, hours)
                		_G.add_origin_key_value("T_ONLINE_HOUR", hours)	--疲劳驾驶模块
			end	
		end
	end
end

--名称：process
--功能：开始处理GPS数据
--返回：无
--修改：2015-07-01 重构实时里程
function DataPackage:process()
        if not self:checkData() then
                return
        end

        local redis_miles       --当前redis中已有的记录
        local old_miles         --保存计算前的redis中已有的数据
        local first_redis = false       --初次初始化redis记录
        if not self['isTokenChanged'] then
                redis_miles = MilesRecord:new(self['M'], self['tokenCode'], self['accountID'])
                local ok = redis_miles:initFromRedis()
                if not ok then
                        redis_miles = nil       --redis中没有该记录
                else
                        old_miles = redis_miles:clone()
                end
        elseif self['lastTokenCode'] then
                self:processLastTokenCode(self['lastTokenCode'])
        end
        
        --先处理补传数据，后处理实时数据
        local extra_miles_package
        if self['hasExtra'] and (not self['isTokenChanged']) then    
                local extra_gps_points = self:getExtraPoints()
                extra_miles_package = MilesPackage:new(true)
                extra_miles_package:init(extra_gps_points, self)
                extra_miles_package:process()
        end
        
        self['startPoint']['repeat'] = false
        
        local gps_points = self:getRealPoints()
        local miles_package = MilesPackage:new(false)
        miles_package:init(gps_points, self)
        miles_package:process()      --实时数据计算结果
        
        --如果redis已经有数据，将redis数据与计算结果数据合并
        if not redis_miles then
                redis_miles = MilesRecord:new(self['M'], self['tokenCode'], self['accountID'])
                redis_miles:initFromData(miles_package)
                first_redis = true
        else 
                redis_miles:setNewData(miles_package)
        end
        
        --若有补传数据，则将补传数据合并
        if self['hasExtra'] and extra_miles_package then
                redis_miles:setNewData(extra_miles_package)
        end
        
        --写入或更新redis数据
        if first_redis then
                redis_miles:write()
        else
                redis_miles:update(old_miles)
        end
         
        if not self['isDelay'] then
                self:writeLastPoint(self['endPoint'])
                
                if self['lastPoint'] and self['startPoint'] 
                        and (self['startPoint']['GPSTime'] - self['lastPoint']['GPSTime']) > DEFS['STATIC']['TIME_INTERVAL_MAX']
                then
                        self:writeLossPoints(self['lastPoint'], self['startPoint']) 
                end
        end
        
        --TODO 按整数公里,传递数据给连续驾驶提醒模块
        local old_sumMileage = tonumber((old_miles or {})['sumMileage'] or 0)
        local new_sumMileage = tonumber(redis_miles['sumMileage'] or 0)
        local old_actualMileage = tonumber((old_miles or {})['actualMileage'] or 0)
        local new_actualMileage = tonumber(redis_miles['actualMileage'] or 0)
        
        self:longDriveMiles(old_actualMileage, new_actualMileage, new_sumMileage, redis_miles['maxSpeed'] or 0, redis_miles['avgSpeed'] or 0, redis_miles['stopTime'] or 0)  --处理连续驾驶有效里程
	self:driveOnlineTime(tonumber(redis_miles['driveTime'] or 0))	--处理驾驶时长
        
        only.log('D', string.format("last_point:%s, hasExtra:%s, isDelay:%s, sumMileage1:%s, sumMileage2:%s, actualMileage1:%s, actualMileage2:%s", 
                        (self['lastPoint'] or {})['GPSTime'] or 0 , self['hasExtra'] or false, self['isDelay'] or false, old_sumMileage, new_sumMileage,
                        old_actualMileage, new_actualMileage))
        
        only.log('D', string.format("lastTime:%s, begin:%s, end:%s, extraBegin:%s, extraEnd:%s", 
                        (self['lastPoint'] or {})['GPSTime'] or 0,
                        (self['startPoint'] or {})['GPSTime'] or 0,
                        (self['endPoint'] or {})['GPSTime'] or 0,
                        (self['extraStartPoint'] or {})['GPSTime'] or 0,
                        (self['extraEndPoint'] or {})['GPSTime'] or 0
                        ))
        only.log('D', string.format("normal mileage:%s, extra mileage:%s", 
                        (miles_package or {})['mileage'] or 0, 
                        (extra_miles_package or {})['mileage'] or 0))
end
