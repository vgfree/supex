-- auth: baoxue
-- time: 2014.04.27

local redis_api = require('redis_pool_api')
local supex = require('supex')
local only = require('only')
local four_miles_handle = require("four_miles_handle")


module('judge', package.seeall)





--前方4公里
function is_4_miles_ahead_have_poi(app_name)
	return  four_miles_handle.handle()
end
