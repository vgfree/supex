-- auth: baoxue
-- time: 2014.04.27

local redis_api 	= require('redis_pool_api')
local supex 		= require('supex')
local only		= require('only')
local cachekv 		= require('cachekv')
local luakv_api 	= require('luakv_pool_api')

module('judge', package.seeall)


--keyName:onceStepKeysSet   	place:owner  		des:记录种类递增key的集合  action:r
function reach_another_step_init(accountID)
	local ok, keys = luakv_api.cmd("owner", accountID, "smembers", accountID .. ":onceStepKeysSet")
	if ok then
		for i, v in ipairs(keys) do
			luakv_api.cmd("owner", accountID, "del", keys[i])
		end
		luakv_api.cmd("owner", accountID, "del", accountID .. ":onceStepKeysSet")
	end
end

local function reach_another_step(app_name, accountID, idx_key, index)
	local keyct1 = string.format("%s:onceStepKeysSet", accountID)
	local keyct2 = string.format("%s:%s:onceStepSet", accountID, app_name)
	local accountID = supex.get_our_body_table()["accountID"]
	
	luakv_api.cmd("owner", accountID, "sadd", keyct1, keyct2)
	local ok,val = luakv_api.cmd("owner", accountID, "sismember", keyct2, index)
	if not ok then return false end
	if not val then
		--[[
		if idx_key then
			local keyct0 = string.format("%s:%s", accountID, idx_key)
			redis_api.cmd("owner", accountID, "set", keyct0, index)
		end
		]]--
		luakv_api.cmd("owner", accountID, "sadd", keyct2, index)
		return true
	else
		return false
	end
end

