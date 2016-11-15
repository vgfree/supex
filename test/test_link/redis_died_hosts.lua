-- 
-- file: redis_short_hosts.lua
-- date: 2015/01/14
-- desc: redis host
--

-- for test
package.path = "../api/?.lua;./api/?.lua;" .. package.path

local socket = require("socket")
local redis = require("redis")
local only = require("only")
local link = require("link")
local REDIS_LIST = link["OWN_DIED"]["redis"]
-- local perf = require('perf')
local REDIS_DIEDS = link["OWN_DIED"]["redis"]

module('redis_died_hosts', package.seeall)

DIED_HOSTS = {}

local function parser(redis_host)
	local tab = {
		mode = "single",
	}
	if #redis_host == 0 then -- single
		if not redis_host["host"] or not redis_host["port"] then
			only.log("E", "link[%s] not find host or port.", k)
			return false
		end
		tab["host"] = redis_host["host"]
		tab["port"] = redis_host["port"]
	else -- multi
		tab["mode"] = "multi"
		tab["master"] = {}
		tab["slave"] = {}

		for i=1,#redis_host do
			if #redis_host[i] ~= 3 then	-- {"M", "192.168.1.12", 7777}
				only.log("E", "link[%s] config error.", k)
				return false
			end

			local node = {
				host = redis_host[i][2],
				port = redis_host[i][3],
				stat = "noused", -- FIXME
			}

			if redis_host[i][1] == "M" then
				table.insert(tab["master"], node)
			elseif redis_host[i][1] == "S" then
				table.insert(tab["slave"], node)
			else
				only.log("E", "link[%s] config error.", k)
				return false
			end -- end if
		end -- end for
	end -- end if
	return tab
end


-- get a available host.
local function get_died_host(redis_name)
	if not DIED_HOSTS[redis_name] then
		only.log("E", "redis[%s] not find", redis_name)
		return false
	end
	
	-- single
	if DIED_HOSTS[redis_name]["mode"] == "single" then
		return DIED_HOSTS[redis_name]["host"], DIED_HOSTS[redis_name]["port"]
	end
	
	-- multi
	if DIED_HOSTS[redis_name]["mode"] == "multi" then
		if not DIED_HOSTS[redis_name]["master"] or #DIED_HOSTS[redis_name]["master"] == 0 then
			only.log("E", "redis[%s] not find", redis_name)
			return false
		end
		
		local index = math.random(1, #DIED_HOSTS[redis_name]["master"])
		
		-- link is ok
		if DIED_HOSTS[redis_name]["master"][index]["link"] then
			return DIED_HOSTS[redis_name]["master"][index]["host"], DIED_HOSTS[redis_name]["master"][index]["port"]
		end

		-- master-redis is not available, then replace it.
		only.log("E", "master:[%s][%d] will be replaced by slave host", DIED_HOSTS[redis_name]["master"][index]["host"],
			DIED_HOSTS[redis_name]["master"][index]["port"])
		if not DIED_HOSTS[redis_name]["slave"] or #DIED_HOSTS[redis_name]["slave"] == 0 then
			only.log("E", "sorry, no slave hosts")
			return false
		end
			
		DIED_HOSTS[redis_name]["master"][index]["host"] = DIED_HOSTS[redis_name]["slave"][1]["host"]
		DIED_HOSTS[redis_name]["master"][index]["port"] = DIED_HOSTS[redis_name]["slave"][1]["port"]
		DIED_HOSTS[redis_name]["master"][index]["link"] = true
		table.remove(DIED_HOSTS[redis_name]["slave"], 1)
		only.log("I", "slave:[%s][%d] have been activated to master", DIED_HOSTS[redis_name]["master"][index]["host"], 
			DIED_HOSTS[redis_name]["master"][index]["port"])

		return DIED_HOSTS[redis_name]["master"][index]["host"], DIED_HOSTS[redis_name]["master"][index]["port"]
	end
	
	-- now not support other mode
	return false
end

-- delete unavailable long host.
local function del_died_host(redis_name, host, port)
	if not DIED_HOSTS[redis_name] then
		return false
	end
	
	only.log("E", "REDIS[%s][%s:%d] is connect failed", redis_name, host, port)
	
	-- single
	if DIED_HOSTS[redis_name]["mode"] == "single" then
		DIED_HOSTS[redis_name]["link"] = false
		return true
	end
	
	-- multi
	if DIED_HOSTS[redis_name]["mode"] == "multi" then
		if not DIED_HOSTS[redis_name]["master"] or #DIED_HOSTS[redis_name]["master"] == 0 then
			return false
		end
		
		local index = 0
		for i,v in ipairs(DIED_HOSTS[redis_name]["master"]) do
			if v["port"] == port and v["host"] == host then
				index = i
				break
			end
		end
		
		if index == 0 then
			only.log("E", "REDIS[%s][%s:%d] is not find.", redis_name, host, port)
			return false
		end
		
		DIED_HOSTS[redis_name]["master"][index]["link"] = false
		return true
	end
	
	-- now not support other mode
	return false
end


local function redis_cmd(link, cmds, ...)
	cmds = string.lower(cmds)
	--local begin = os.time()
	local ok, ret = pcall(link[ cmds ], link, ...)
	if not ok then
		only.log("E", "%s |--->FAILED! %s...", cmds, ret)
		assert(false, ret)
	end
	-- only.log("D", "use time :" .. (os.time() - begin))
	return ret
end

function init()
	-- pool
	for k,v in pairs(REDIS_DIEDS) do
		-- check double define.
		if DIED_HOSTS[k] then
			only.log("E", "link[OWN_DIED][redis][%s] double define.", k)
			return false
		end

		local tab = parser(v)
		if not tab then
			return false
		end
		DIED_HOSTS[k] = tab
	end -- end for

	return true
end

-->|	local args = {...}
-->|	local cmd, kv1, kv2 = unpack(args)
function cmd(redis_name, ...)
	-- only.log('D', string.format("START REDIS CMD |---> %f", socket.gettime()))
	-->> redis_name, cmd, keyvalue1, keyvalue2, ...
	-->> redis_name, {{cmd, keyvalue1, keyvalue2, ...}, {...}, ...}
	
	-- perf.bind("REDIS SHORT SUCCESS", "REDIS SHORT FAILURE")
	-->> fetch link
	local ok, host, port
	local link = nil
	repeat
		host, port = get_died_host(redis_name)
		if not host or not port then
			return false
		end
		ok, link = pcall(redis.connect, host, port)
		if not ok or not link then
			del_died_host(redis_name, host, port)
			link = nil
		end
	until link
	
	-->> do cmd
	local stat,ret,err
	if type(...) == 'table' then
		ret = {}
		for i=1,#... do
			if type((...)[i]) ~= 'table' then
				only.log("E", "error args to call redis_api.cmd(...)")
				break
			end

			stat,ret[i] = pcall(redis_cmd, link, unpack((...)[i]))

			if not stat then err = ret[i] break end
		end
	else
		stat,ret = pcall(redis_cmd, link, ...)

		if not stat then err = ret end
	end
	
	-->> close
	link:quit()
	link = nil

	-- only.log('D', string.format("END REDIS CMD |---> %f", socket.gettime()))
	if not stat then
		only.log("E", "failed in redis_cmd %s", tostring(err))
		-- perf.over("REDIS SHORT FAILURE")
		return false,err
	end
	-- perf.over("REDIS SHORT SUCCESS")
	return true,ret
end

