--版权声明：无
--文件名称：miles_package.lua
--创建者  ：耿玄玄
--创建日期：2015-07-01
--文件描述：里程数据计算包类，对gps数据包进行处理
--修    改：2015-07-01 重构实时里程

local utils = require('utils')
local scan = require('scan')
local only = require('only')
local redis_pool_api = require('redis_pool_api')
local MilesRecordModule = require("miles_record")
local MilesRecord = MilesRecordModule.MilesRecord

local DEFS = require('realtime_defs')

module('miles_package', package.seeall)

MilesPackage = {
        M,
        tokenCode,
        accountID,
        createTime,     --服务器接收时间
        maxSpeed,       --数据包中最大速度
        minSpeed,       --数据包中最小速度
        startPoint,     --数据包中开始时间gps点信息，GPSPoint类型
        endPoint,       --数据包中结束时间gps点信息，GPSPoint类型
        lastPoint,      --上个包中最后时间gps点信息，GPSPoint类型
        points,         --经过筛选的待计算gps点信息，GPSPoint类型数组
        isExtra,        --是否是补传数据，true：补传数据；false：实时数据
        isTokenChanged, --tokenCode是否变化
        normal,         --异常停车标识

        --以下变量通过计算获得
        mileage,     --有效里程
        lowSpeedTime,   --低速行驶时间，speed小于20且direction~=-1
        stopTime,       --停车时间
        speedSum, --速度总和，用于计算平均速度
        acptCount,
}

-- Return new object of MilesPackage
function MilesPackage:new(isExtra)
        local self = {
                }

        setmetatable(self, MilesPackage)
        MilesPackage.__index = MilesPackage
        MilesPackage.__tostring = self.tostring
        self['isExtra'] = isExtra

        return self
end

--名称：init
--功能：从Datapackage中初始化MilesPackage
--参数：self --> DataPackage
--返回：无
--修改：2015-07-01 重构实时里程
function MilesPackage:init(points, data_package)
        self['M'] = data_package['M']
        self['tokenCode'] = data_package['tokenCode']
        self['accountID'] = data_package['accountID']
        self['createTime'] = data_package['createTime']
        self['points'] = points
        self['isDelay'] = data_package['isDelay']
        self['isTokenChanged'] = data_package['isTokenChanged']
        
        if data_package['isExtra'] then
                self['startPoint'] = data_package['extraStartPoint']
                self['endPoint'] = data_package['extraEndPoint']
        else
                self['startPoint'] = data_package['startPoint']
                self['endPoint'] = data_package['endPoint']
        end

        self['maxSpeed'] = 0
        self['mileage'] = 0
        self['lowSpeedTime'] = 0
        self['stopTime'] = 0
        self['speedSum'] = 0
        self['acptCount'] = 0
        self['normal'] = 1
end

--名称：calcMil
--功能：根据GPS数据 计算里程
--参数：self --> DataPackage
--返回：无
--修改：2015-07-01 重构实时里程
function MilesPackage:calcMil()
        local points = self['points']
        local loss_info = {}
        local n = table.getn(points)

        if n < 1 then
                return false
        end

        local mileage = 0
        for var = 1,n do
                repeat
                        if not points[var]['repeat'] then   --重复点不参与累计信息计算
                                --获得本次低速
                                if points[var]['speed'] < 20 and points[var]['direction'] ~= -1 then
                                        self['lowSpeedTime'] = self['lowSpeedTime'] + 1
                        end

                        --获得本次停车时间
                        if points[var]['direction'] == -1 or points[var]['speed'] == 0 then
                                self['stopTime'] = self['stopTime'] + 1
                        end

                        --获得本次最大速度
                        if points[var]['speed'] > self['maxSpeed'] then
                                self['maxSpeed'] = points[var]['speed']
                        end

                        --获得本次速度和
                        self['speedSum'] = self['speedSum'] + points[var]['speed']
                        self['acptCount'] = self['acptCount'] + 1
                        end

                        if var == 1 then
                                break   --第一个点 continue
                        end

                        local itvl = points[var].GPSTime - points[var-1].GPSTime

                        local speed1 = ((points[var-1]['direction'] == -1) and 0) or points[var-1]['speed']
                        local speed2 = ((points[var]['direction'] == -1) and 0) or points[var]['speed']

                        if itvl > DEFS['STATIC']['TIME_INTERVAL_MAX'] then
--                                local prevPt = {}
--                                local nextPt = {}
--
--                                prevPt['GPSTime'] = points[var-1]['GPSTime']
--                                prevPt['speed'] = speed1
--                                nextPt['GPSTime'] = points[var]['GPSTime']
--                                nextPt['speed'] = speed2

                                -- 插入 loss_info 并返回
--                                local range_info = {}
--                                range_info['begin'] =  points[var - 1]
--                                range_info['end'] =  points[var]
--                                table.insert(loss_info, range_info)
                                only.log('W', string.format("MilesPackage:calcMil, timeitval:%s", itvl))

                                break   -- continue the for loop
                        elseif itvl < 1 then
                                break   -- continue the for loop
                        end

                        local tmpMil = ((speed1 + speed2) / 2 ) * ( 1000 / 3600 ) * itvl
                        tmpMil = tmpMil - tmpMil%0.01  -- format %.2f
                        mileage = mileage + tmpMil
                until true
        end

        self['mileage'] = mileage
end

--名称：process
--功能：开始处理GPS数据
--参数：self --> DataPackage
--返回：无
--修改：2015-07-01 重构实时里程
function MilesPackage:process()
        self:calcMil()
        -- 异常停车标示
        if self['acptCount'] > 0 and (not self['isExtra']) and  (not self['isDelay']) then
                if self['endPoint']['direction'] ~= -1 
                   and self['endPoint']['speed'] > 5 then
                        self['normal'] = 0 
                end
        end
end
