package.path = "../../open/lib/?.lua;" .. package.path
package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 2222
local serv = "msgsearchapi"

local function http(cfg, data)
	local tcp = socket.tcp()
	if tcp == nil then
		error('load tcp failed')
		return false
	end
	tcp:settimeout(10000)
	local ret = tcp:connect(cfg["host"], cfg["port"])
	if ret == nil then
		error("connect failed!")
		return false
	end

	tcp:send(data)
	local result = tcp:receive("*a")
	tcp:close()
	return result
end

local function gen_url(T)
	local url
	for k,v in pairs(T or {}) do
		if url then
			url = string.format([[%s&%s=%s]], url, k, v)
		else
			url = k .. '=' .. v
		end
	end
	return url or ""
end

local function gen_data( path, body )
	return "POST /" .. path .. " HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	--"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


local body = {
        ["msgsearchapi/getsendmessage"] = {
                time = "1441505400",
        },
        ["msgsearchapi/getgeotomessage"] = {
                time = "1441505400",
        },
        ["msgsearchapi/getvoices"] = {
                time = "1442028000",
        },
        ["msgsearchapi/getreplyvoices"] = {
                time = "1441505400",
        },
        ["msgsearchapi/getsendbizid"] = {
                accountID = "4Pj5AoTQqz",
                appKey    = "1114536329",
                startTime = "1441780200",
                endTime   = "1441782000",
        },
        ["msgsearchapi/getsendbizid"] = {
                imei      = "985881522355553",
                appKey    = "1114536329",
                startTime = "1441780200",
                endTime   = "1441782000",
        },
        ["msgsearchapi/getsendbizid"] = {
                appKey    = "2491067261",
                startTime = "1441780200",
                endTime   = "1441782000",
        },
        ["msgsearchapi/getsendinfo"] = {
                bizid = "a4b11c6a18543d11e5a03c00a0d1ed47b4",
        },
        --[[
        ["msgsearchapi/getvoice"] = {
        },
        --]]
        ["msgsearchapi/getreplyvoice"] = {
                bizid = "a484ba6394543d11e5a56e00a0d1eb3da4",
                time  = "1441505400",
        },
        ["msgsearchapi/getfilelocation"] = {
                bizid = "a4b11c6a18543d11e5a03c00a0d1ed47b4",
        },
        ["msgsearchapi/getgeobizid"] = {
                senderLongitude = "115.05",
                senderLatitude  = "35.77",
                startTime       = "1441584600",
                endTime         = "1441591800",
        },
        ["msgsearchapi/getsendbizidsize"] = {
                accountID = "4Pj5AoTQqz",
                appKey    = "1114536329",
                startTime = "1441780200",
                endTime   = "1441782000",
        },
        ["msgsearchapi/getsendbizidsize"] = {
                imei      = "985881522355553",
                appKey    = "1114536329",
                startTime = "1441780200",
                endTime   = "1441782000",
        },
        ["msgsearchapi/getsendbizidsize"] = {
                appKey    = "2491067261",
                startTime = "1441780200",
                endTime   = "1441782000",
        },
        ["msgsearchapi/getgeobizidsize"] = {
                senderLongitude = "115.05",
                senderLatitude  = "35.77",
                startTime       = "1441584600",
                endTime         = "1441591800",
        },
        --[[
        ["msgsearchapi/getvoicesize"] = {
        },
        --]]
        ["msgsearchapi/getfeedbackinfo"] = {
                accountID = "lTPLXSoHmy",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getfeedbackinfos"] = {
                time = "1442028000",
        },
        ["msgsearchapi/getfeedbackinfosize"] = {
                accountID = "lTPLXSoHmy",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getnewstatusinfo"] = {
                bizid = "a44aa60e3658fe11e5a56e00a0d1eb3da4",
                time  = "1442028000",
        },
        ["msgsearchapi/getreadmessageinfo"] = {
                bizid = "a44aa60e3658fe11e5a56e00a0d1eb3da4",
                time  = "1442028000",
        },
        ["msgsearchapi/getreadmessage"] = {
                accountID = "CllAaRlZGE",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getreadmessages"] = {
                time = "1442028000",
        },
        ["msgsearchapi/getreadmessagesize"] = {
                accountID = "CllAaRlZGE",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getreleaseadtalk"] = {
                accountID = "lTPLXSoHmy",
                appKey    = "257131325",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getreleaseadtalks"] = {
                time = "1442028000",
        },
        ["msgsearchapi/getreleaseadtalksize"] = {
                accountID = "lTPLXSoHmy",
                appKey    = "257131325",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getreleasemessageinfo"] = {
                bizid = "a44aa60e3658fe11e5a56e00a0d1eb3da4",
                time  = "1442028000",
        },
        ["msgsearchapi/getreleasemessage"] = {
                accountID = "CllAaRlZGE",
                appKey    = "1209071138",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getreleasemessages"] = {
                time = "1442028000",
        },
        ["msgsearchapi/getreleasemessagesize"] = {
                accountID = "CllAaRlZGE",
                appKey    = "1209071138",
                startTime = "1442028000",
                endTime   = "1442028000",
        },
        ["msgsearchapi/getcitymessage"] = {
                time = "1444638600",
        },
        ["msgsearchapi/getcitymessageinfo"] = {
                bizid = "a6ebca68aa70bb11e5ac1500a0d1ed47b4",
        },
        ["msgsearchapi/getcitycode"] = {
                time = "1444969200",
        },
        ["msgsearchapi/getcitycodemessage"] = {
                cityCode  = "310000",
                startTime = "1444969200",
                endTime   = "1444969200",
        },
        ["msgsearchapi/getcitycodevoice"] = {
                cityCode  = "310000",
                startTime = "1444969200",
                endTime   = "1444969200",
        },
}

for key,val in pairs(body) do
	local data = gen_data( key, gen_url(val) )
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
