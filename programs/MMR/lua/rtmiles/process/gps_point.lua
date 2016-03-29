--版权声明：无
--文件名称：gps_point.lua
--创建者  ：耿玄玄
--创建日期：2015-06-27
--文件描述：gps点信息
--修    改：2015-06-27 重构实时里程

module("gps_point", package.seeall)

GPSPoint = {

        }

-- Return new object of GPSPoint
function GPSPoint:new()
        local self = {
                }

        setmetatable(self, GPSPoint)
        GPSPoint.__index = GPSPoint
        GPSPoint.__tostring = self.tostring

        return self
end

--名称：writeLastPoint
--功能：写入上次gps点信息
--参数：self --> DataPackage
--返回：无
--修改：2015-07-01 重构实时里程
function GPSPoint:init(arg)
        self['longitude']    = tonumber(arg['longitude'] or -1)
        self['latitude']     = tonumber(arg['latitude'] or -1)
        self['speed']        = tonumber(arg['speed'] or 0)
        self['GPSTime']      = tonumber(arg['GPSTime'] or -1)
        self['direction']    = tonumber(arg['direction'] or -1)
end

-- Return string of GPSPoint
function GPSPoint:tostring()
        local str = "{ longitude={" .. self['longitude'] .. "}, latitude={" .. self['latitude'] ..
                "}, speed={" .. self['speed'] .. "}, GPSTime={" .. self['GPSTime'] ..
                "}, direction={" .. self['direction'] .. "} }"

        return str
end
