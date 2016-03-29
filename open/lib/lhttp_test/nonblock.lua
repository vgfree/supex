package.path = "../?.lua;" .. package.path
package.cpath = "../?.so;" .. package.cpath
local lhttp = require 'lhttp'
local idx = 1

local function fcb ()
	idx = idx + 1
end

lhttp.reg_idle_cb(fcb)

local memb = {
	host = "www.baidu.com",
	port = 80,
}
local data = [[
GET / HTTP/1.0
User-Agent: curl/7.32.0
Host: www.baidu.com
Accept: */*

]]
data = data:gsub('\n', '\r\n')
print(data)

ok,memb["sock"] = pcall(lhttp.connect, memb["host"], memb["port"])
print(ok, memb["sock"])
stat,ret = pcall(memb["sock"][ "origin" ], memb["sock"], data)

print(stat, ret)
print(idx)
