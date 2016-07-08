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
	host = "127.0.0.1",
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

local hashname = 'Sicily'

local cmds = {
  {
    cmd = "GEOADD",
    args = {13.361389, 38.115556, "Palermo", 15.087269, 37.502669, "Catania", 13.583333, 37.316667, "Agrigento"}
  },
  {
    cmd = "GEODIST",
    args = {"Palermo", "Catania"}
  },
  {
    cmd = "GEODIST",
    args = {"Palermo", "Catania", 'km'}
  },
  {
    cmd = "GEODIST",
    args = {"Palermo", "Catania", 'mi'}
  },
  {
    cmd = "GEODIST",
    args = {"Palermo", "BAR"}
  },
  {
    cmd = "GEORADIUS",
    args = {15, 37, 100, 'km'}
  },
  {
    cmd = "GEORADIUS",
    args = {15, 37, 200, 'km'}
  },
  {
    cmd = "GEORADIUS",
    args = {15, 37, 200, 'km', "WITHDIST"}
  },
  {
    cmd = "GEORADIUS",
    args = {15, 37, 200, 'km', "WITHCOORD"}
  },
  {
    cmd = "GEORADIUS",
    args = {15, 37, 200, 'km', "WITHDIST", "WITHCOORD"}
  },
  {
    cmd = "GEOPOS",
    args = {"Palermo", "Catania", "NonExisting"}
  },
  {
    cmd = "GEORADIUSBYMEMBER",
    args = {"Agrigento", 100, "km"}
  },
  {
    cmd = "GEOHASH",
    args = {"Palermo", "Catania"}
  },
}


ok,memb["sock"] = pcall(lredis.connect, socket, memb["host"], memb["port"])
print(ok, memb["sock"])

for i = 1, #cmds do
stat,ret = pcall(memb["sock"][ cmds[i].cmd:lower() ], memb["sock"], hashname, unpack(cmds[i].args))

print('===================================================')
local value = ret
print(stat, value)
if type(value) == 'table' then
  for j = 1, #value do
    local tmp = value[j]
    print(tmp)
    if type(tmp) == 'table' then
      for k = 1, #tmp do
        local temp = tmp[k]
        print(temp)
        if type(temp) == 'table' then
          for l = 1, #temp do
            print(temp[l])
          end
        end
      end
    end
  end
end

print('$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$')
end

--[[
local file = io.open('about_recv.txt', 'w+')
file:write(ret.value)
file:close()
]]
