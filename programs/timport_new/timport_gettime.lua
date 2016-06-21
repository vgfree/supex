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

local function get_next_starttime(timestamp)
	if timestamp == nil then
		only.log('E', 'timestamp is nil')
		return nil
	end
	
	local start_time = timestamp + 60 * CFG_LIST['time_interval']
	return start_time
end


function get_time(timestamp)
	return get_next_starttime(timestamp)
end

