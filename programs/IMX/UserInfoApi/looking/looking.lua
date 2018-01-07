local utils     = require('utils')
local only      = require('only')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local api_redis_looking = require("api_redis_looking")

module("looking", package.seeall)

function handle(table)
	local ret = nil
	if table[1] == 'looking' then
		ret = api_redis_looking.get_data(table)		
	else
		only.log('E', 'First frame is invalid !')
		return nil
	end
	
	return ret
end
