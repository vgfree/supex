package.cpath = "../../../open/lib/?.so;" .. package.cpath                                           
local socket = require('socket')

local file

local host = "127.0.0.1"
local port = "8222"

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
                socket.sleep(0.1)
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
                socket.sleep(0.1)
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
                if coroutine.status(prod) ~= "dead" then
                        local x = receive(prod)  -- get new value
                        senddata(host, port, x)        -- consume new value
                else
                        break
                end
        end
end

p = producer()
--f = filter(p)
consumer(p)
