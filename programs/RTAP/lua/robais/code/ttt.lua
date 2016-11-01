local only = require('only')
local supex = require('supex')
local luakv_api = require('luakv_pool_api')
local redis_api = require('redis_pool_api')

module('ttt', package.seeall)


function handle()
	only.log("E", "??????")
	local i = 1
	--for i=1, 20000000 do
		local ok, ret = redis_api.cmd("tsdb", "", "set", "key---is---" .. i, "value---is---" .. i)
		local ok, ret = luakv_api.cmd("tsdb", "", "set", "key---is---" .. i, "value---is---" .. i)
	--end
end

