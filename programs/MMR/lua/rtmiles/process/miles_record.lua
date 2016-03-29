--版权声明：无
--文件名称：miles_record.lua
--创建者  ：耿玄玄
--创建日期：2015-07-02
--文件描述：里程数据redis处理类
--修    改：2015-07-02 重构实时里程

local utils = require('utils')
local only = require('only')
local redis_pool_api = require('redis_pool_api')

local DEFS = require('realtime_defs')

module('miles_record', package.seeall)

MilesRecord = {
        IMEI,
        tokenCode,
        accountID,
        createTime,     --服务器接收时间
        maxSpeed,       --数据包中最大速度
        minSpeed,       --数据包中最小速度
        startTime,     --数据包中开始时间
        startLongitude,
        startLatitude,
        startDirection,
        endTime,       --数据包中结束时间
        endLongitude,
        endLatitude,
        endDirection,
        normal,         --异常停车标识
        sumMileage,     --有效里程
        actualMileage,  --实际里程
        GPSLossRate,    --GPS丢失率
        GPSacptCount,    --实时GPS点接收总数，已经去重
        lowSpeedTime,   --低速行驶时间，speed小于20且direction~=-1
        stopTime,       --停车时间
        avgSpeed,       --平均速度
        actualSpeedSum, --速度总和，用于计算平均速度
        actualGPSacptCount,   --所有GPS点接收总数，用于计算平均速度
}

-- Return new object of MilesRecord
function MilesRecord:new(IMEI, tokenCode, accountID)
        if not IMEI or not tokenCode then
                return nil
        end

        local self = {
                }

        setmetatable(self, MilesRecord)
        MilesRecord.__index = MilesRecord

        self['IMEI'] = IMEI
        self['tokenCode'] = tokenCode
        self['accountID'] = accountID
        self['recordKey'] = string.format("%s:%s:mileage", IMEI, tokenCode)

        return self
end

--名称：initFromRedis
--功能：从MilesPackage中初始化MilesRecord
--返回：无
--修改：2015-07-01 重构实时里程
function MilesRecord:initFromRedis()
        local ok,ret = redis_pool_api.cmd('mapRTMileage', self['IMEI'] or '', "HMGET", self['recordKey'],
                'startTime',
                'endTime',
                'sumMileage',
                'actualMileage',
                'GPSacptCount',
                'lowSpeedTime',
                'stopTime',
                'maxSpeed',
                'actualSpeedSum',
                'actualGPSacptCount')

        if (not ok) or (not ret) or (#ret == 0) then
                only.log('E', string.format("get record from redis error, record key>>%s", self['recordKey']))
                return nil
        end

        self['startTime'] = tonumber(ret[1])
        self['endTime'] = tonumber(ret[2])
        self['sumMileage'] = tonumber(ret[3])
        self['actualMileage'] = tonumber(ret[4])
        self['GPSacptCount'] = tonumber(ret[5])
        self['lowSpeedTime'] = tonumber(ret[6])
        self['stopTime'] = tonumber(ret[7])
        self['maxSpeed'] = tonumber(ret[8])
        self['actualSpeedSum'] = tonumber(ret[9])
        self['actualGPSacptCount'] = tonumber(ret[10])

        return true
end

--名称：initFromData
--功能：从MilesPackage中初始化MilesRecord
--返回：无
--修改：2015-07-01 重构实时里程
function MilesRecord:initFromData(miles_package)
        self['createTime'] = miles_package['createTime']
        self['startLongitude'] = miles_package['startPoint']['longitude']
        self['startLatitude'] = miles_package['startPoint']['latitude']
        self['startDirection'] = miles_package['startPoint']['direction']
        self['startTime'] = miles_package['startPoint']['GPSTime']
        self['endLongitude'] = miles_package['endPoint']['longitude']
        self['endLatitude'] = miles_package['endPoint']['latitude']
        self['endDirection'] = miles_package['endPoint']['direction']
        self['endTime'] = miles_package['endPoint']['GPSTime']
        self['normal'] = miles_package['normal']
        self['maxSpeed'] = miles_package['maxSpeed']
        self['sumMileage'] = 0
        self['actualMileage'] = 0
        self['GPSLossRate'] = 0
        self['GPSacptCount'] = 0
        self['lowSpeedTime'] = 0
        self['stopTime'] = 0
        self['avgSpeed'] = 0
        self['actualSpeedSum'] = 0
        self['actualGPSacptCount'] = 0
        
        self:setNewData(miles_package)

        return true
end


--clone 一个新table并返回
--名称：clone
--功能：一个新table并返回
--返回：新clone的record数据
--修改：2015-07-01 重构实时里程
function MilesRecord:clone()
        local clone_miles = {}
        for k,v in pairs(self) do
                clone_miles[k] = v
        end
        
        return clone_miles
end

--名称：setNewData
--功能：从MilesPackage中初始化MilesRecord
--返回：无
--修改：2015-07-01 重构实时里程
function MilesRecord:setNewData(miles_package)
        --更新起点信息
        if miles_package['startPoint']['GPSTime'] < self['startTime'] then
                self['startTime'] = miles_package['startPoint']['GPSTime']
                self['startLongitude'] = miles_package['startPoint']['longitude']
                self['startLatitude'] = miles_package['startPoint']['latitude']
                self['startDirection'] = miles_package['startPoint']['direction']
        end

        --更新终点信息
        if miles_package['endPoint']['GPSTime'] > self['endTime'] then
                self['endTime'] = miles_package['endPoint']['GPSTime']
                self['endLongitude'] = miles_package['endPoint']['longitude']
                self['endLatitude'] = miles_package['endPoint']['latitude']
                self['endDirection'] = miles_package['endPoint']['direction']
        end

        if not miles_package['isExtra'] then
                --有效里程
                if miles_package['mileage'] ~= 0 then
                        self['sumMileage'] = self['sumMileage'] + miles_package['mileage']
                end
                --GPS丢失率

                self['GPSacptCount'] = self['GPSacptCount'] + miles_package['acptCount']
        end

        --实际里程
        if miles_package['mileage'] ~= 0 then
                self['actualMileage'] = self['actualMileage'] + miles_package['mileage']
        end
        --实时数据时，异常关机标志
        if not miles_package['isExtra'] and miles_package['normal'] then
                self['normal'] = miles_package['normal']
        end
        --低速行驶时间
        if miles_package['lowSpeedTime'] ~= 0 then
                self['lowSpeedTime'] = self['lowSpeedTime'] + miles_package['lowSpeedTime']
        end
        --停车时间
        if miles_package['stopTime'] ~= 0 then
                self['stopTime'] = self['stopTime'] + miles_package['stopTime']
        end
        --行车时间
        self['driveTime'] = self['endTime'] - self['startTime']
        --最大速度
        if self['maxSpeed'] < miles_package['maxSpeed'] then
                self['maxSpeed'] = miles_package['maxSpeed']
        end
        --平均速度

        if miles_package['acptCount'] ~= 0 then
                self['actualSpeedSum'] = self['actualSpeedSum'] + miles_package['speedSum']
                self['actualGPSacptCount'] = self['actualGPSacptCount'] + miles_package['acptCount']

                local lossRate = ((self['endTime'] - self['startTime'] + 1 - self['GPSacptCount']) / (self['endTime'] - self['startTime'] + 1)) * 100
                self['GPSLossRate'] = lossRate - lossRate%1

                if self['actualGPSacptCount'] == 0 then
                        self['avgSpeed'] = 0
                else
                        local avgSpeed = self['actualSpeedSum'] / self['actualGPSacptCount']
                        self['avgSpeed'] = avgSpeed - avgSpeed % 1
                end
        end
end

--名称：write
--功能：从MilesPackage中初始化MilesRecord
--返回：无
--修改：2015-07-01 重构实时里程
function MilesRecord:write()
        local ok , ret = redis_pool_api.cmd('mapRTMileage', self['IMEI'] or "" ,"HMSET", self['recordKey'],
                'createTime', self['createTime'],
                'startLongitude', self['startLongitude'],
                'startLatitude', self['startLatitude'],
                'startDirection', self['startDirection'],
                'startTime', self['startTime'],
                'endLongitude', self['endLongitude'],
                'endLatitude', self['endLatitude'],
                'endDirection', self['endDirection'],
                'endTime', self['endTime'],
                'GPSLossRate', self['GPSLossRate'],
                'sumMileage', self['sumMileage'],
                'actualMileage', self['actualMileage'],
                'GPSacptCount', self['GPSacptCount'],
                'normal', self['normal'] or 1,
                'lowSpeedTime', self['lowSpeedTime'] or 0,
                'stopTime', self['stopTime'] or 0,
                'maxSpeed', self['maxSpeed'] or 0,
                'avgSpeed', self['avgSpeed'] or 0,
                'driveTime', self['driveTime'] or 0,
                'actualSpeedSum', self['actualSpeedSum'] or 0,
                'actualGPSacptCount', self['actualGPSacptCount'] or 0)

        if not ok then
                only.log('E', string.format("MilesRecord:write error, record key>>%s", self['recordKey']))
                return
        end

        -- write to last gps time to, wait for data curing
        ok , ret = redis_pool_api.cmd('mapRTMileage', self['IMEI'] or '', "ZADD", DEFS['STATIC']['LASTED_MILE_ZSET'], self['endTime'], self['recordKey'])
        if not ok then
                only.log('E', string.format("Write %s to LASTED_MILE_ZSET error", self['recordKey']))
                return
        end

        -- write mileage count
        ok , ret = redis_pool_api.cmd('mapRTMileage',self['IMEI'], "INCR", 'mileageCount')
        if not ok then
                only.log('E', "incr mileageCount error")
                return
        end
end

--比较新record与原先的record有哪些元素不同，并返回这些不同元素的key和value，用于更新redis
function MilesRecord:diffAttr(old_miles)
        local diff_attr = {}
        
        for k,v in pairs(self) do
        repeat
                if k == 'createTime' then       --createTime不更新
                        break
                end
                
                if (not old_miles[k]) or (self[k] ~= old_miles[k]) then
                        table.insert(diff_attr, k)
                        table.insert(diff_attr, v)
                end
        until true
        end
        
        return diff_attr
end

--更新redis，更新发生变化的元素
function MilesRecord:update(old_miles)
        local diff_attr = self:diffAttr(old_miles)
        if diff_attr and next(diff_attr) then
                local ok , ret = redis_pool_api.cmd('mapRTMileage', self['IMEI'] or "" ,"HMSET", self['recordKey'], unpack(diff_attr))

                if not ok then
                        only.log('E', string.format("MilesRecord:update error, record key>>%s", self['recordKey']))
                        return
                end
        end
end
