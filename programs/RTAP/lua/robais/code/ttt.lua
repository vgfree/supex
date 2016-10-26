local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('ttt', package.seeall)


function handle()
	only.log("E", "??????")
	for i=1, 20000000 do
		local ok, ret = redis_api.cmd("tsdb", "", "set", "key---is---" .. i, "value---is---" .. i)
	end
end

