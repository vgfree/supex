local only = require('only')
local supex = require('supex')

module('ashman_test', package.seeall)


function handle()
	only.log("E", "??????")
	local data = 'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.baidu.com\r\n' ..
	'Accept: */*\r\n\r\n'

	local ok, info = supex.http("www.baidu.com", 80, data, #data)
	print(ok, info)
	--print(ok)
	--os.execute("sleep 2")
	--print(supex.get_our_info_data())
end

