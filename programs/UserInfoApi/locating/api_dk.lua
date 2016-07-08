local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local api_redis_locate = require("api_redis_locate")

module("api_dk", package.seeall)

local function parseFirstFrame(table)
	local ret = nil
	if table[1] == 'locating' then
		ret = api_redis_locate.get_data(table)		
	else
		only.log('E', 'First frame is invalid !')
		return nil
	end
	
	return ret
	
end

function handle(table)
	local ret = nil
	ret =  parseFirstFrame(table)
	return ret
end
