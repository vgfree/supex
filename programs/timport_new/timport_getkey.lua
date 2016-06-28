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

local function assemble_key(timestamp)
	local time = os.date("%Y%m%d%H", timestamp)
	local interval = tonumber(CFG_LIST['time_interval'])
	if interval >= 10 then
		time = time .. math.floor(os.date("%M", timestamp)/10)
	end
	
	if interval < 10 then
		time = time .. math.floor(os.date("%M", timestamp)/10) .. interval
		print(time)
	end
	
	local user_key = CFG_LIST['USER_PART_KEY'] .. ":" ..time
 
	return user_key
end


function get_key(timestamp)
	--timestamp = timestamp - CFG_LIST['delay_time']
	return assemble_key(timestamp)
end
--[[
function handle()
	local timestamp = 1467093600
	local key = get_key(timestamp)
	print(key)
end

handle = handle()
]]--
