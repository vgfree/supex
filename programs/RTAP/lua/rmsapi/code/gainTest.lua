local only = require('only')
local supex = require('supex')
local redis_api = require("redis_pool_api")

module('gainTest', package.seeall)


local data = [[
#EXTM3U
#EXTINF:10,
http://127.0.0.1:2222/gainTS?file=%03d.ts
#EXTINF:10,
http://127.0.0.1:2222/gainTS?file=%03d.ts
#EXTINF:10,
http://127.0.0.1:2222/gainTS?file=%03d.ts
#EXT-X-STREAM-INF:
http://127.0.0.1:3333/gainTest
]]
--#EXT-X-ENDLIST

function handle()
	local time = os.time()
	local idx = (time / 10) % 21
	local data = string.format(data, idx, idx + 1, idx + 2)
	data = string.gsub(data, "\n", "\r\n")
	
	local afp = supex.rgs(200)
	supex.say(afp, data)
	return supex.over(afp)
end

