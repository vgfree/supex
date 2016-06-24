local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../open/lib/?.lua;",
        "../../open/apply/?.lua;",
        "../../open/spxonly/?.lua;",
        "../../open/linkup/?.lua;",
        "../../open/public/?.lua;",
        "open/?.lua;",
}

local CFG_LIST          = require('cfg')

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local math	= require("math")

redis_api.init()

local function get_data(key, cmd, redis_cnt)
        if key == nil or cmd == nil then
                return nil
        end
        local ok_status, ok_ret = redis_api.cmd('redis' .. tostring(redis_cnt), '', cmd, key)
        if not (ok_status and ok_ret) then
                only.log('E', 'Get data failed or no suitable data')
                return nil
        end
        return ok_ret
end

local function set_data(key, cmd, redis_cnt, expire_time)
        if cmd == nil or redis_cnt == nil or expire_time == nil then
                return nil
        end
	print('set_data')
        local ok_status, ok_ret = redis_api.cmd('redis' .. tostring(redis_cnt), '',cmd, key, expire_time)
        if not (ok_status and ok_ret) then
                only.log('E', 'Set data failed or no suitable data')
                return nil
        end
	print(ok_ret)
        return ok_ret
end

local function set_expire_time(key, expire_time, redis_cnt)
	if key == nil then
		return nil
	end

	local user = get_data(key, 'SMEMBERS', redis_cnt)
	only.log('E', 'user = %s', scan.dump(user))
	local time = string.gsub(key, CFG_LIST['USER_PART_KEY'] .. ':', '') 
	local user_key
	for idx = 1, #user do
		for dk_idx = 1, #CFG_LIST['timport'] do
			if CFG_LIST['timport'][dk_idx]['key'] ~= nil then
				user_key = CFG_LIST['timport'][dk_idx]['key'] .. user[idx] .. ':' .. time
                        	set_data(user_key, 'EXPIRE', redis_cnt, expire_time)
			end
		end
	end

	set_data(key, 'EXPIRE', redis_cnt, expire_time)
end


function set_time(tab)
	print(tab[1])
	print(tab[2])
	local expire_time = CFG_LIST['expire_time']
	if #CFG_LIST['timport'] > 0 then
		set_expire_time(tab[1], expire_time, tab[2])
	end
end

--[[
function handle()
	local tab = {}
	tab[1] = 'ACTIVEUSER:20160622140'
	tab[2] = 1
	set_time(tab)
end

handle = handle()
]]--
