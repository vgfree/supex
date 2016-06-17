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

local Key = '0'

local function assembleKey(timestamp, timeInterval)
	local time = os.date("%Y%m%d%H", timestamp)
	if timeInterval >= 10 then
		time = time .. os.date("%M", timestamp)/10
	end
	
	if timeInterval < 10 then
		time = time .. math.floor(os.date("%M", timestamp)/10) .. timeInterval
		print(time)
	end
	
	local userKey = CFG_LIST['USER_PART_KEY'] .. ":" ..time
 
	return userKey
end


function GetKey(tab)
	--local userPart = CFG_LIST['USER_PART_KEY']
	--print(tab[1])
	--print(tab[2])
	Key = assembleKey(tab[1], tab[2])
	--print(Key)
	return Key
end

