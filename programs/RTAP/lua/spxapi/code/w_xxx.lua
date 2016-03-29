local only = require('only')
local supex = require('supex')
local tts = require('tts')

module('w_xxx', package.seeall)

tts.init(supex["__TASKER_NUMBER__"])

function handle()
	local data = 'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.sina.com\r\n' ..
	'Accept: */*\r\n\r\n'

--	local ok, info = supex.http("www.sina.com", 80, data, #data)
	only.log("D", info)
--	print(ok, info)
	only.log("E", "1111111111111111")
	tts.main(lua_default_switch, supex["__TASKER_SCHEME__"])
	only.log("E", "2222222222222222")
end

