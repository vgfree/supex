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

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local CFG_LIST	= require('cfg')
local dk_utils	= require('get_tsdb_name')

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

local function set_data(key, data, cmd, tsdb_name)
	if key == nil or cmd == nil or data == nil or tsdb_name == nil then
		return nil
	end
	
	local ok_status, ok_ret = redis_api.cmd(tsdb_name, '', cmd, key, data)
	if not (ok_status and ok_ret) then
        	only.log('E', 'Set data failed or no suitable data')
		return nil        
	end
	
	print(ok_ret)
	return ok_ret
end

local function assemble_data(user_data)
	if user_data == nil or #user_data == 0 then
		only.log('E', "The userData is nil.")
		return nil
	end
	table.sort(user_data)
	only.log('E', scan.dump(user_data))

	--拿到每一行|出现的次数
	local str = user_data[1]
	local xxx, count = string.gsub(str, '|', '|')
	local data = #user_data .. '*' .. count + 1 .. '@'
	only.log('E', scan.dump(data))
	
	for i = 1, #user_data do
		data = data .. user_data[i] .. '|'	
	end

	only.log('E', scan.dump(data))

	return data
end

local function get_data_with_user(user, target_time, redis_num)
	local data_key = nil
	if user == nil then
		return nil
        end

	if target_time == nil then
		return nil
	end
	
	if redis_num == nil then
		return nil
	end
	local time_key = target_time
	target_time = string.gsub(target_time, CFG_LIST['USER_PART_KEY'] .. ':', '')
	print(target_time)

	local user_data = nil
	local data = nil
	local tsdb_name = nil
	for idx = 1, #CFG_LIST['timport'] do
		data_key = CFG_LIST['timport'][idx]['key'] .. user .. ':' .. target_time
		user_data = get_data(data_key, 'SMEMBERS', redis_num)
		only.log('E', scan.dump(user_data))
		data = assemble_data(user_data)
		tsdb_name = dk_utils.get_tsdb_name(user, os.time(), idx)
		set_data(data_key, data, 'SET', tsdb_name)
		set_data(time_key, user, 'SADD', tsdb_name)
	end
end

function get_table(tab)
	print(tab[1])
	print(tab[2])
	print(tab[3])
	if #CFG_LIST['timport'] > 0 then
		get_data_with_user(tab[1], tab[2], tab[3])
	end
end

