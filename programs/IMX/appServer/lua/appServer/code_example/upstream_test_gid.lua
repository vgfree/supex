local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('upstream', package.seeall)


function handle()
	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])
--[[	
	only.log("E", "??????")
	local data = 'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.baidu.com\r\n' ..
	'Accept: */*\r\n\r\n'

	local ok, info = supex.http("www.baidu.com", 80, data, #data)
	print(ok, info)
--]]
	if supex["_DATA_"][1] == "upstream" then
		local tab = {
			[1] = "downstream",
			[2] = "gid",
			[3] = "gid0",
			[4] = supex["_DATA_"][3]


		--	[2] = "cid"
		--	[3] = supex["_DATA_"][3],
		--	[4] = "hello",
		}
	
		local ok = zmq_api.cmd("downstream", "send_table", tab)
		if ok then
			print("send msg to client successful!")
		end
	end

end

