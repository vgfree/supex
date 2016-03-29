package.cpath = '/opt/luasql-master/src/?.so;'..package.cpath
local luasql = require "luasql.mysql"

local Mysqlclass = {}

function Mysqlclass:new (o)
        o = o or {sqlstr = "SELECT nodeID, longitude, latitude FROM nodeInfo where nodeID between 10000000 and 10000003", sqldb = "roadMap", sqluser = "mttp", sqlpas = "pmttp", sqlhost = "192.168.1.16"}
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
        --self.cur = assert(self.conn:execute(self.sqlstr))
end

function Mysqlclass:insertNode(x, y)
        local nodeid = 1
        for i=1,x do
                for j=1,y do
                        local res = assert (self.conn:execute(string.format([[
                            INSERT INTO nodeInfo (longitude, latitude, nodeID)
                            VALUES ('%f', '%f', '%d')]], i+20.11111, j+110.22222, nodeid)
                        ))
                        nodeid = nodeid+1
                end
        end
end

function Mysqlclass:insertArc(x,y)
        local nodeid = 1
        local roadid = 1
        for i=1,x do
                for j=1,y-1 do
                        local res = assert (self.conn:execute(string.format([[
                                INSERT INTO arcInfo (roadID, fNodeID, eNodeID)
                                VALUES ('%d', '%d', '%d')]], roadid, nodeid-1+j+(i-1)*x, nodeid+j+(i-1)*x)
                        ))
                        roadid = roadid+1
                end
        end

        for i=1,y do
                for j=1,x-1 do
                        local res = assert (self.conn:execute(string.format([[
                                INSERT INTO arcInfo (roadID, fNodeID, eNodeID)
                                VALUES ('%d', '%d', '%d')]], roadid, nodeid+(j-1)*y+i-1, nodeid+j*y+i-1)
                        ))
                        roadid = roadid+1
                end
        end
end

function Mysqlclass:insertVehicle(num, x, y)
        local roadid, speed, imei
        math.randomseed(os.time())
        for i=1,num do
                roadid = math.random(x, y)
                speed = math.random(80)
                imei = 100000000000000 + i
                local res = assert (self.conn:execute(string.format([[
                INSERT INTO vehicle (roadID, speed, imei)
                VALUES ('%d', '%f', '%d')]], roadid, speed, imei)
                ))
        end
end

function Mysqlclass:clear(tab)
        assert (self.conn:execute("delete from " .. tab))
        assert (self.conn:execute("truncate " .. tab))
end

function Mysqlclass:del()
        self.conn:close()
        self.env:close()
end
test = Mysqlclass:new()
test:init()
test:clear("nodeInfo")
test:insertNode(7,7)
test:clear("arcInfo")
test:insertArc(7,7)
test:clear("vehicle")
test:insertVehicle(10, 2, 80)
test:del()
return Mysqlclass
