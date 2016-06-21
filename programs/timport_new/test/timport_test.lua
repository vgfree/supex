local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../../open/lib/?.lua;",
        "../../../open/apply/?.lua;",
        "../../../open/spxonly/?.lua;",
        "../../../open/linkup/?.lua;",
        "../../../open/public/?.lua;",
        "open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")

redis_api.init()

local function get_data(key, cmd, redis_num)
	if key == nil or cmd == nil then
		return nil
	end
	local ok_status, ok_ret = redis_api.cmd('redis' .. tostring(redis_num), '', cmd, key)
        if not (ok_status and ok_ret) then
        	only.log('E', 'Get data failed or no suitable data')
		return nil        
	end
	return ok_ret
end

local function set_data(redis, cmd, key, data)
	if key == nil or data == nil then
		return nil
	end
	
	if type(data) == 'table'then
		for tab_id = 1, #data do
			redis_api.cmd(redis, '', cmd, key, data[tab_id])
		end
	end
	
	if type(data) ~= 'table' then
		local ok_status, ok_ret = redis_api.cmd(redis, '', cmd, key, data)
		if not (ok_status and ok_ret) then
        		only.log('E', 'Set data failed or no suitable data')
			return nil        
		end
	end

	return ok_ret
end

local function creat_user(id)
	local current_time = os.time()
        local time = id .. os.date("%Y%m%d%H", current_time)
	return time
end
 
local function creat_key()
	local current_time = os.time()
        local time = os.date("%Y%m%d%H", current_time)
        local interval = 10
        if interval >= 10 then
                time = time .. math.floor(os.date("%M", current_time)/10)
        end

        if interval < 10 then
                time = time .. math.floor(os.date("%M", current_time)/10) .. interval
                print(time)
        end

        local user_key = "ACTIVEUSER:" ..time

        return user_key, time
end

local user_data = '1466044886|1466044901|508542171690587||460012202737137|160616024126|1234339042|418406815|65|29|0|QnsQkkBlen" "1466044885|1466044901|508542171690587||460012202737137|160616024125|1234339042|418406815|65|29|0|QnsQkkBlen" "1466044884|1466044901|508542171690587||460012202737137|160616024124|1234339042|418406815|65|29|0|QnsQkkBlen" "1466044883|1466044901|508542171690587||460012202737137|160616024123|1234339042|418406815|65|29|0|QnsQkkBlen" "1466044882|1466044901|508542171690587||460012202737137|160616024122|1234339042|418406815|65|29|0|QnsQkkBlen'

function handle()
	local key, time = creat_key()
	local user_tab = {}
	for id = 1, 50 do
		user_tab[id] = creat_user(id)
	end
	only.log("E", "the table = %s", scan.dump(user_tab))
	local redis = 'redis' .. os.time() % 2 
	set_data(redis, 'SADD', key, user_tab)
	
	for tab_id = 1, #user_tab do
		key = 'GPS:' .. user_tab[tab_id] .. ':' .. time
		set_data(redis, 'SET', key, user_data)
	end
end

handle = handle()

