package.cpath = "/opt/openresty/lualib/?.so;" .. package.cpath

-- Module instantiation
local cjson = require "cjson"
local cjson2 = cjson.new()
local cjson_safe = require "cjson.safe"

local file
local write_file
local words = {}
local flag = 0
--local val
local gps_cnt = 0
local host = "127.0.0.1"
local port = "8222"
--[[
function senddata(host, port, body)
        local   MAX = 1
        for i=1, MAX do
 
                local tcp = socket.tcp()
                tcp:setoption('keepalive', true)
                tcp:settimeout(1, 'b')  -- five second timeout
 
                local ret = tcp:connect(host, port)
                if ret == nil then
                        return false
                end
 
                local data = "POST /publicentry HTTP/1.0\r\n" ..
                "User-Agent: curl/7.33.0\r\n" ..
                "Host: " .. host .. ":" .. port .. "\r\n" ..
                "Content-Type: application/json; charset=utf-8\r\n" ..
                --"Content-Type: application/x-www-form-urlencoded\r\n" ..
                "Connection: close\r\n" ..
                "Content-Length:" .. #body .. "\r\n" ..
                "Accept: */*\r\n\r\n" ..
                body
 
 
                tcp:send(data)
                local result = tcp:receive("*a")
                print(result)
                tcp:close()
        end
        --os.execute("sleep 10")
end
--]]
function min(num1, num2, num3)
        local t = {}
        t[1] = num1
        t[2] = num2
        t[3] = num3
        local key, min = 1, t[1]
        for k, v in ipairs(t) do
                if t[k] < min then
                        key, min = k, v
                end
        end
        return min;
end

function decode_gps(gps)
        if not gps then return 0 end
        val = cjson.decode(gps)
        local a = table.getn(val.longitude)
        local b = table.getn(val.latitude)
        local c = table.getn(val.GPSTime)
        return min(a, b, c)
end

function avg_speed(speed_tab)
        local sum = 0
        local max = 0
        for n in pairs(speed_tab) do
                sum = sum + speed_tab[n]
                if speed_tab[n] > max then max =  speed_tab[n] end
        end
        return sum/table.getn(speed_tab), max
end

function write_to_file(data_tab)
        local string_tab = table.concat(data_tab, "\t") .. "\n" 
        write_file:write(string_tab)
end

function opinion_file(data)
        if data == nil then return end
        local i = string.find(data,"longitude")
        if i ~= nil then
                local val = cjson.decode(data)
                local a = table.getn(val.longitude)
                local b = table.getn(val.latitude)
                local c = table.getn(val.GPSTime)

                if (min(a, b, c) > 0) then
                        if flag == 1 or flag == 2 or flag == 3 then
                                write_to_file(words)
                                words = {}
                                flag = 0;
                        end
                        local avg, max = avg_speed(val.speed)
                        table.insert(words, val.IMEI)
                        table.insert(words, val.IMSI)
                        table.insert(words, val.GPSTime[1])
                        table.insert(words, val.longitude[1])
                        table.insert(words, val.latitude[1])
                        table.insert(words, avg)
                        table.insert(words, max)
                        table.insert(words, val.direction[1])
                        flag = 1
                end
                return
        end
        i = string.find(data, "pmr road_info")
        if i ~= nil then
                local k, j = string.find(data, "roadname ", i)
                local k1, j1 = string.find(data, " road_rootID ", j)
                local k2, j2 = string.find(data, " segmentID ", j1)
                local k3, j3 = string.find(data, " countycode ", j2)
                table.insert(words, string.sub(data, j+1, k1-1))
                table.insert(words, string.sub(data, j1+1, k2-1))
                table.insert(words, string.sub(data, j2+1, k3-1))
                flag = 2
                return
        end
        i = string.find(data, "kv_roadID IMEI")
        if i ~= nil then
                print(data)
                local k, j = string.find(data, "max_speed ", i+1)
                local k1, j1 = string.find(data, " avg_speed ", j)
                local k2, j2 = string.find(data, " end_time ", j1)
                table.insert(words, string.sub(data, j+1, k1-1))
                table.insert(words, string.sub(data, j1+1, k2-1))
                flag = 3
                return
        end
end

function send (x)
        coroutine.yield(x)
end

function readfilen(n, v)
        while n > 0 do
                local line = file:read()
                if line then 
                        send(line)
                else 
                        file:close()
                        break
                end
                --socket.sleep(0.1)
                n = n - 1
        end
end

function readfile()
        while true do
                local line = file:read()
                if line then
                        send(line)
                else 
                        file:close()
                        break
                end
                --socket.sleep(0.1)
        end
end

function receive (prod)
        local status, value = coroutine.resume(prod)
        return value
end

function producer ()
        file = io.open(arg[1], "r")
        write_file = io.open(arg[2], "w")

        return coroutine.create(function ()
                while true do
                        local x = io.read()      -- produce new value
                        if x == 'n' or x == '\n' then
                                print("next")
                                readfilen(1,arg[1])
                        elseif x == 'c' then
                                readfile()
                                return
                                --break
                        end
                        local num = tonumber(x)
                        if num then
                                print("num")
                                readfilen(num, arg[1])
                        end
                end
        end)
end

function filter (prod)
        return coroutine.create(function ()
                local line = 1
                while true do
                        local x = receive(prod)  -- get new value
                        x = string.format("%5d %s", line, x)
                        send(x)       -- send it to consumer
                        line = line + 1
                end
        end)
end

function consumer (prod)
        while true do
                --if coroutine.status(prod) ~= "dead" then
                if coroutine.status(prod) == "suspended" then
                        local x = receive(prod)         -- get new value
                        --print(x)
                        --local num = decode_gps(x)         -- consume new value
                        --gps_cnt = gps_cnt + num
                        opinion_file(x)
                else
                        break
                end
        end
        print("done")
        write_file:close()
        --print(gps_cnt)
end

p = producer()
--f = filter(p)
consumer(p)
