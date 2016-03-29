local luakv_api 	= require('luakv_pool_api')
local scene 		= require('scene')
local supex		=require('supex')


--keyName:onceStepKeysSet   	place:driview  		des:记录种类递增key的集合  action:r
function reach_another_step_init(accountID)
	local ok, keys = luakv_api.cmd("driview", accountID, "smembers", accountID .. ":onceStepKeysSet")
	if ok then
		for i, v in ipairs(keys) do
			luakv_api.cmd("driview", accountID, "del", keys[i])
		end
		luakv_api.cmd("driview", accountID, "del", accountID .. ":onceStepKeysSet")
	end
end

local function reach_another_step(app_name, accountID, idx_key, index)
	local keyct1 = string.format("%s:onceStepKeysSet", accountID)
	local keyct2 = string.format("%s:%s:onceStepSet", accountID, app_name)
	local accountID = supex.get_our_body_table()["accountID"]
	
	luakv_api.cmd("driview", accountID, "sadd", keyct1, keyct2)
	local ok,val = luakv_api.cmd("driview", accountID, "sismember", keyct2, index)
	if not ok then return false end
	if not val then
		--[[
		if idx_key then
			local keyct0 = string.format("%s:%s", accountID, idx_key)
			redis_api.cmd("driview", accountID, "set", keyct0, index)
		end
		]]--
		luakv_api.cmd("driview", accountID, "sadd", keyct2, index)
		return true
	else
		return false
	end
        
end


function drive_online_point_init(accountID)
	luakv_api.cmd('driview', accountID, 'set', accountID .. ':sysInternalConfigTimestamp', os.time())
end

--函数:drive_online_point
--功能:
--说明:目前只有只有疲劳驾驶应用在使用
function drive_online_point(app_name)
	local increase = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["drive_online_point"]["increase"]
	local accountID = supex.get_our_body_table()["accountID"]
	local online = get_current_online_time(accountID)
	local divisor = math.floor(online / increase)
	local remainder = online % increase
	if divisor == 0 then
		return false
	end
	only.log("D", string.format("==drive_online_point== %d:%d", divisor, remainder))
	if (remainder > 0) and (remainder < 60) then
		local ok = reach_another_step(app_name, accountID, nil, divisor)
		if ok then
			scene.push( app_name, { ["driveOnlineHoursPoint"] = divisor } )
		end
		return ok
	else
		return false
	end
end
