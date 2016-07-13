local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('status', package.seeall)


function handle()
	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])
	
	if supex["_DATA_"][1] == "status" then
		if supex["_DATA_"][2] == "connected" then
		local tab = {
			[1] = "setting",
			[2] = "gidmap",
			[3] = supex["_DATA_"][3],
			[4] = "gid0",
		}
		local ok = zmq_api.cmd("setting", "send_table", tab)

		if ok then
			print("send to setting successful")
		end
		end
	end

end

