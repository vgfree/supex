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

local str = 'ssssssssssssa;dsadasfqwwwwwwwww99999999999999999999bbbbbbbbbbbbbkkkkkkkkkkkdddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm2222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333300000000000000000000000---------------------------======================================================[[[[[[[[[[[[[[[[[[[[[[[[[[[[dddddddddddddddddddddddddddddddd,,,,,,,,,,,,,,,,,,,,,,,,,,,cccccccccccc.................///////////////////////////fffffffffffffffffffffffffffddddddddddddddddddddddddddddd'


local function setData(key, data)
	print(key)
	local ok_status, ok_ret = redis_api.cmd('IdKey', '', 'SET', key, data)
        if not (ok_status and ok_ret) then
        	print('Get key failed')
		return nil
	end
end

local function getData(key)
	print(key)
	local ok_status, ok_ret = redis_api.cmd('IdKey', '', 'GET', key)
        if not (ok_status and ok_ret) then
        	print('Get key failed')
		return nil        
	end
	return ok_ret
end

local function set_task_redis(usr)
	assert(usr)
	for i = 0, 100000 do
		setData('lawrence:' .. usr .. ':' .. tostring(i), str)
	end
end

local function get_task_redis(usr)
	assert(usr)
	local ret_str
        for i = 0, 100000 do
                ret_str = getData('lawrence:' .. usr .. ':' .. tostring(i))
		if ret_str ~= str then
			print('check data failed')
			return
		end
        end
end

function handle(num)
	get_task_redis(num)
	--set_task_redis(num)
end
