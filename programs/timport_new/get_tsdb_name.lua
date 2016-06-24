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
local math	= require("math")

local CFG_LIST  = require('cfg')

module("get_tsdb_name", package.seeall)

local function get_tsdb_from_imei(imei, dk_idx)
	key = tonumber(imei) % 8192
	scan.dump("E", "key = %d", key)
	if #CFG_LIST['timport'][dk_idx]['tsdb'] == 0 then
		scan.dump("E", "Can not find any tsdb")
		return nil
	end
	
	for idx = 1, #CFG_LIST['timport'][dk_idx]['tsdb'] do
		if key >= CFG_LIST['timport'][dk_idx]['tsdb'][idx]['key_set'][1] and key < CFG_LIST['timport'][dk_idx]['tsdb'][idx]['key_set'][2] then
			return CFG_LIST['timport'][dk_idx]['tsdb'][idx]['name']
		end
	end
	
	scan.dump("E", "Can not find suitable tsdb")
	return nil
end

local function get_tsdb_from_time(time, dk_idx)
	local day = os.date("%d", time)
	print(day)
	if #CFG_LIST['timport'][dk_idx]['tsdb'] == 0 then
                scan.dump("E", "Can not find any tsdb")
                return nil
        end
	
	local idx = tonumber(time) % (#CFG_LIST['timport'][dk_idx]['tsdb'])
	return CFG_LIST['timport'][dk_idx]['tsdb'][idx]['name']
end

function get_tsdb_name(imei, time, dk_idx)
	if dk_idx == nil or time == nil or imei == nil then
		scan.dump("E", "paramter is nil")
		return nil
	end
	
	local tsdb_name = nil
	local func = CFG_LIST['timport'][dk_idx]['hash_filter'] 
	if func == 'imei' then
		tsdb_name = get_tsdb_from_imei(imei, dk_idx)
	end

	if func == 'time' then
		--tsdb_name = get_tsdb_from_time(time, dk_idx)
	end 
	
	scan.dump('E', 'tsdb_name = %s', tsdb_name)	
	return tsdb_name
end
--[[
function handle()
	local time = os.time()
	local imei = 111111111111111
	print(get_tsdb_name(imei, time, 1))
	print(get_tsdb_name(imei, time, 2))
end

handle = handle()
]]--
