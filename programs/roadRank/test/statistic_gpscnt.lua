package.cpath = "/opt/openresty/lualib/?.so;" .. package.cpath

-- Module instantiation
local cjson = require "cjson"
local cjson2 = cjson.new()
local cjson_safe = require "cjson.safe"

local file
local val
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
                        print(x)
                        local num = decode_gps(x)         -- consume new value
                        gps_cnt = gps_cnt + num
                else
                        break
                end
        end
        print(gps_cnt)
end

p = producer()
--f = filter(p)
consumer(p)
