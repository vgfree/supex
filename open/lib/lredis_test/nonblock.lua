package.path = "../?.lua;" .. package.path
package.cpath = "../?.so;" .. package.cpath
local lredis = require 'lredis'
local idx = 1
local socket = require('socket')

local function fcb ()
	idx = idx + 1
end

lredis.reg_idle_cb(fcb)

local memb = {
	host = "192.168.1.12",
	port = 6379,
}

local file = io.open('about.txt', 'w+')
for i = 1, 1025*1024 do
  file:write('1234567890abcdef')
end
file:close()

file = io.open('about.txt')
local data = file:read('*all')

print(#data)

ok,memb["sock"] = pcall(lredis.connect, socket, memb["host"], memb["port"])
print(ok, memb["sock"])
stat,ret = pcall(memb["sock"][ "set" ], memb["sock"], "qytestkey", data)

print(stat, ret.value)
print(idx)

stat,ret = pcall(memb["sock"][ "get" ], memb["sock"], "qytestkey")

print(idx)

local file = io.open('about_recv.txt', 'w+')
file:write(ret.value)
file:close()
