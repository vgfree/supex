package.cpath = "../../../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

function senddata(host, port, body)
        local	MAX = 1
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
