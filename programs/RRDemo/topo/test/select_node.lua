package.cpath = '/opt/luasql-master/src/?.so;'..package.cpath
require "car_client" 
local luasql = require "luasql.mysql"

function receive(prod)                                                                                  
        local status, value = coroutine.resume(prod)
        return status, value
end

function send(x)
        coroutine.yield(x)
end

local Mysqlclass = {}

function Mysqlclass:new (o)
        o = o or {sqlstr = "SELECT roadID, speed, imei FROM vehicle", sqldb = "roadMap", sqluser = "mttp", sqlpas = "pmttp", sqlhost = "192.168.1.16"}
        self.__index = self
        return setmetatable(o, self)
end

function Mysqlclass:printf()
        return self.sqlstr
end

function Mysqlclass:init()
        self.env = luasql.mysql()      
        self.conn = self.env:connect(self.sqldb, self.sqluser, self.sqlpas, self.sqlhost, 3306)
        self.conn:execute"SET NAMES UTF8"
        self.cur = assert(self.conn:execute(self.sqlstr))
end

function Mysqlclass:fetch()
        row = self.cur:fetch({}, "a")   
        self.file = io.open("car.txt", "w+")
        return coroutine.create(function()
                while row do               
                        varstr = string.format("{\"roadid\":\"%s\", \"speed\":\"%s\", \"imei\":\"%s\"}\n", row.roadID, row.speed, row.imei)
                        --var = {nodeid = row.nodeID, longi = row.longitude, lati = row.latitude}
                        self.file:write(varstr)
                        send(varstr)
                        row = self.cur:fetch(row, "a")
                end 
        end)
end

function Mysqlclass:consumer(prod)
        while true do
                local status, var = receive(prod)
                if status and var then
                        print("re ", var.nodeid)
                        senddata("127.0.0.1", "5555", var)
                else
                        break
                end
        end
end

function Mysqlclass:del()
        self.file:close()
        self.conn:close()
        self.env:close()
end
--[[
test = Mysqlclass:new()
print(test:printf())
test:init()
p = test:fetch()
test:consumer(p)
test:del()

te = Mysqlclass:new{sqlstr="lalallal"}
print(te:printf())
--]]
return Mysqlclass
