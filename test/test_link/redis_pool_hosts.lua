-- 
-- file: redis_pool_hosts.lua
-- date: 2015/01/14
-- desc: redis host
--

-- for test
package.path = "../api/?.lua;./api/?.lua;" .. package.path

local only = require("only")
local utils = require("utils")
local link = require("link")
local redis = require("redis")
local socket = require("socket")
-- local perf = require('perf')
local REDIS_POOLS = link["OWN_POOL"]["redis"]

module("redis_pool_hosts", package.seeall)

-- global var.
POOL_HOSTS = {}

-- for redis pool
local MAX_RECNT = 3
local MAX_DELAY = 20
local function new_connect(host, port)
	local ok, link = nil
	local nb = 0
	repeat
		nb = nb + 1
		ok,link = pcall(redis.connect, host, port)
		if not ok then
			-- FIXME: sleep 1s ?
			socket.select(nil, nil, 0.05 * MAX_DELAY)
			only.log("E", "REDIS: [%s:%d] Tcp:connect: FAILED: %s", host, port, link)
			link = nil
		end
		if nb >= MAX_RECNT then
			return false
		end
	until link 

	only.log("I", "REDIS: [%s:%d] Tcp:connect: SUCCESS!", host, port)
	return link 
end

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

-- get a available long host.
local function get_pool_host(redis_name)
	if not POOL_HOSTS[redis_name] then
		return false
	end
	
	-- single
	if POOL_HOSTS[redis_name]["mode"] == "single" then
		local link = nil
		if not POOL_HOSTS[redis_name]["link"] then
			link = new_connect(POOL_HOSTS[redis_name]["host"], POOL_HOSTS[redis_name]["port"])
			if not link then
				return false
			end
			POOL_HOSTS[redis_name]["link"] = link
			-- FIXME: not necessary.
			-- POOL_HOSTS[redis_name]["link"]["mt_host"] = POOL_HOSTS[redis_name]["host"]
			-- POOL_HOSTS[redis_name]["link"]["mt_port"] = POOL_HOSTS[redis_name]["port"]
		end
		return POOL_HOSTS[redis_name]["link"]
	end
	
	-- multi
	if POOL_HOSTS[redis_name]["mode"] == "multi" then
		if not POOL_HOSTS[redis_name]["master"] or #POOL_HOSTS[redis_name]["master"] == 0 then
			return false
		end
		
		local index = math.random(1, #POOL_HOSTS[redis_name]["master"])
		
		-- link is ok.
		if POOL_HOSTS[redis_name]["master"][index]["link"] then
			return POOL_HOSTS[redis_name]["master"][index]["link"]
		end

		-- link was deleted, then reconnect
		-- if reconnect failed, then replace it
		-- if replace failed, then return false
		local link = nil
		local host = POOL_HOSTS[redis_name]["master"][index]["host"]
		local port = POOL_HOSTS[redis_name]["master"][index]["port"]
		repeat
			link = new_connect(host, port)
			if not link then
				-- replace it.
				only.log("E", "master:[%s][%d] will be replaced by slave host", host, port)
				if not POOL_HOSTS[redis_name]["slave"] or #POOL_HOSTS[redis_name]["slave"] == 0 then
					-- TODO: log
					only.log("E", "sorry, no slave hosts")
					return false
				end
				host = POOL_HOSTS[redis_name]["slave"][1]["host"]
				port = POOL_HOSTS[redis_name]["slave"][1]["port"]
				table.remove(POOL_HOSTS[redis_name]["slave"], 1)
				only.log("I", "slave:[%s][%d] have been activated to master", host, port)
			end
		until link
		POOL_HOSTS[redis_name]["master"][index]["host"] = host
		POOL_HOSTS[redis_name]["master"][index]["port"] = port
		POOL_HOSTS[redis_name]["master"][index]["link"] = link
		POOL_HOSTS[redis_name]["master"][index]["link"]["mt_host"] = host 
		POOL_HOSTS[redis_name]["master"][index]["link"]["mt_port"] = port
		
		return POOL_HOSTS[redis_name]["master"][index]["link"]
	end
		
	-- now not support other mode
	return false
end

-- delete unavailable long host.
local function del_pool_host(redis_name, link)
	if not POOL_HOSTS[redis_name] then
		return false
	end
	
	-- single
	if POOL_HOSTS[redis_name]["mode"] == "single" then
		POOL_HOSTS[redis_name]["link"].network.socket:close()
		POOL_HOSTS[redis_name]["link"] = nil
		return true
	end
	
	-- multi
	if POOL_HOSTS[redis_name]["mode"] == "multi" then
		if not POOL_HOSTS[redis_name]["master"] or #POOL_HOSTS[redis_name]["master"] == 0 then
			return false
		end
		
		local index = 0
		for i,v in ipairs(POOL_HOSTS[redis_name]["master"]) do
			if v["port"] == link["port"] and v["host"] == link["host"] then
				index = i
				break
			end
		end
		
		if index == 0 then
			only.log("E", "link[OWN_POOL][redis][%s] not find.", redis_name)
			return false
		end
		
		POOL_HOSTS[redis_name]["master"][index]["link"].network.socket:close()
		POOL_HOSTS[redis_name]["master"][index]["link"] = nil
		return true
	end
	
	-- now not support other mode
	return false
end

local function redis_cmd(redis_name, cmds, ...)
	----------------------------
	-- start
	----------------------------
	cmds = string.lower(cmds)
	local link, stat, ret
	repeat
		link = get_pool_host(redis_name)
		if not link then
			only.log("E", "REDIS[%s] is dead all", redis_name)
			assert(false, string.format("cmd:[%s] failed.", cmds))
		end
		
		stat, ret = pcall(link[cmds], link, ...)
		if not stat then
			only.log("E", "REDIS[%s]: %s |--->FAILED! %s . . .", redis_name, cmds, ret)
			del_pool_host(redis_name, link)
		end
	until stat
	----------------------------
	-- end
	----------------------------
	-- only.log("D", "use time :" .. (os.time() - begin))
	return ret
end

function init()
	-- pool
	for k,v in pairs(REDIS_POOLS) do
		-- check double define.
		if POOL_HOSTS[k] then
			only.log("E", "link[OWN_POOL][redis][%s] double define.", k)
			return false
		end

		local tab = parser(v)
		if not tab then
			return false
		end
		POOL_HOSTS[k] = tab
	end -- end for

	return true
end

-->|	local args = {...}
-->|	local cmd, kv1, kv2 = unpack(args)
function cmd(redis_name, ...)
	-- only.log('D', string.format("START REDIS CMD |---> %f", socket.gettime()))
	-->> redname, cmd, keyvalue1, keyvalue2, ...
	-->> redname, {{cmd, keyvalue1, keyvalue2, ...}, {...}, ...}
	
	-- no need.
	-- perf.bind("REDIS POOL SUCCESS", "REDIS POOL FAILURE")
	-- if not fetch_pool(redname) then
	--	perf.over("REDIS POOL FAILURE")
	--	return false,nil
	-- end

	local stat,ret,err
	if type(...) == "table" then
		ret = {}
		for i=1,#... do
			if type((...)[i]) ~= "table" then
				only.log("E", "error args to call redis_api.cmd(...)")
				break
			end

			stat,ret[i] = pcall(redis_cmd, redis_name, unpack((...)[i]))
			if not stat then err = ret[i] break end
		end
	else
		stat,ret = pcall(redis_cmd, redis_name, ...)
		if not stat then err = ret end
	end

	if not stat then
		-- perf.over("REDIS POOL FAILURE")
		return false,err
	end
	-- perf.over("REDIS POOL SUCCESS")
	return true,ret
end

