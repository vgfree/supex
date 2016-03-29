-- 
-- file: test.lua
-- date: 2014-11-27
-- desc: test case.
--


local cutils = require('cutils')


local function test_http_GET()
    local ip = "127.0.0.1"
    local port = 8088
    local data = "GET /tsearchapi/v2/getgpssize?imei=171614946623890&startTime=1416995700&endTime=1416995700 HTTP/1.0\r\n" ..
                "Host: 127.0.0.1:8088\r\n\r\n"
    local size = #data
    local ok, ret = cutils.http(ip, port, data, size)
    if not ok then
        print("test_http_GET error:" .. ret)
        return false
    end
    print("test_http_GET ok, response:" ..ret)
    return true
end


local function test_http_POST()
    local ip = "127.0.0.1"
    local port = 8088
    local body = "imei=171614946623890&startTime=1416995700&endTime=1416995700"
    local data = "POST /tsearchapi/v2/getgpssize HTTP/1.0\r\n" ..
                "User-Agent: curl/7.33.0\r\n" ..
                "Host: 127.0.0.1:8088\r\n" ..
                "Connection: close\r\n" ..
                "Content-type: application/json\r\n" ..
                "Content-Length:" .. #body .. "\r\n" ..
                "Accept: */*\r\n\r\n" ..
                body

    local size = #data
    local ok, ret = cutils.http(ip, port, data, size)
    if not ok then
        print("test_http_POST error:" .. ret)
        return false
    end
    print("test_http_POST OK, response:" .. ret)
    return true
end

test_http_GET()
test_http_POST()

