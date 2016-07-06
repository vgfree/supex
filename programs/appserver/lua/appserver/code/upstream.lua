local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('upstream', package.seeall)


function handle()
	only.log("E", "??????")
	local data = 'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.baidu.com\r\n' ..
	'Accept: */*\r\n\r\n'
	local DATA = '/robaisFetch.json HTTP/1.1\r\nHost: 127.0.0.1:80\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: 0\r\n\r\n'

	for i=1, 100000 do
		local ok, ret = redis_api.cmd("damServer","lpushx", "0", DATA)
	end
	--local ok, info = supex.http("www.baidu.com", 80, data, #data)
	--print(ok, info)
	--local ok, info = redis_api.cmd('private', "", 'set', "key", "value")
	--print(ok, info)
	--local ok, info = redis_api.cmd('private', "", 'get', "key")
	--print(ok, info)
end

