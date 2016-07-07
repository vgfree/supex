local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('upstream', package.seeall)


function handle()
	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])
	only.log("E", "??????")
	local data = 'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.baidu.com\r\n' ..
	'Accept: */*\r\n\r\n'

	local ok, info = supex.http("www.baidu.com", 80, data, #data)
	print(ok, info)
end

