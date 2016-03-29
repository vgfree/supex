local redis_api = require('redis_pool_api')
local luakv_api = require("luakv_pool_api")
local only = require('only')
local conf = require("four_miles_conf")
local scan = require("scan")

module('count_four_miles', package.seeall)

--写入redis
local function write_in_redis(luakv_key)
	luakv_api.cmd("public", "", "incr", luakv_key )	
	local ok, start_time = luakv_api.cmd("public", "", "get", "start_time")	
	only.log("D", "start_time:" .. start_time)
	only.log("D", "os_time:" .. os.time())
	only.log("D", "时间差:".. os.time() - start_time) 
	if os.time() - start_time >= 60 then
		local ok, tb = luakv_api.cmd("public", "", "hgetall", "SCENE_KEY_LIST")
		if not ok then
			only.log("E", "luakv failed")
			return false
		end   
		only.log('D',scan.dump(tb))
		for i=1, #tb/2 do
			redis_api.cmd("public", "", "hmset", "SCENE_KEY_LIST", tb[2*i-1], tb[2*i])   
		end
		for i=1, #tb/2 do
			local redis_key = string.format("%s:%s:DRIVIEW_SCENE_COUNTS", os.date("%Y%m%d"),tb[2*i-1] )
			only.log("D", "redis_key:" .. redis_key)
			local ok, counts = luakv_api.cmd("public", "", "get", redis_key)
			if not ok then
				only.log("E", "luakv failed")
				return false
			end
			if not counts then
				counts = 0
			end
			only.log("D", "counts:" .. counts)
			redis_api.cmd("public", "", "incrby", redis_key, counts)
			luakv_api.cmd("public", "", "set", redis_key, 0)
			luakv_api.cmd("public", "", "set", "del", "start_time" )					
		end
	end
end

--统计每种poitype下发次数
function count_times(poitype)
	local poitype = string.sub(poitype, 0, 7)
	poitype = tonumber(poitype)
	local value = conf["POI_LIST"][poitype]["txt"]
	only.log("D", "poitype:" .. poitype)
	only.log("D", "value:" .. value)
	luakv_api.cmd("public", "", "hmset", "SCENE_KEY_LIST", poitype, value)   
	local luakv_key = string.format("%s:%s:DRIVIEW_SCENE_COUNTS", os.date("%Y%m%d"),poitype ) 
	local ok, start_time = luakv_api.cmd("public", "", "get", "start_time")
	if not ok then 
		only.log("E", "luakv failed")
		return false
	end
	if not start_time then
		only.log("D", "333333333333")
		luakv_api.cmd("public", "", "set", "start_time", os.time() )
	end
--	if poitype == 1123110 or poitype == 1123111 then
--		only.log('D',scan.dump(value))
--		for i=1, #value do
--			write_in_redis(luakv_key)		
--		end
--	else
		write_in_redis(luakv_key)		
--	end
end
