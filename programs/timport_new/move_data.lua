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

redis_api.init()

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


function handle(num)
	print('enter lua handle')
end

--handle = handle()
