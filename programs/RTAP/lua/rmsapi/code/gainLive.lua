local only = require('only')
local supex = require('supex')
local redis_api = require("redis_pool_api")

module('gainLive', package.seeall)


local data1 = "#EXTM3U\r\n#EXT-X-VERSION:3\r\n#EXT-X-MEDIA-SEQUENCE:%d\r\n#EXT-X-PLAYLIST-TYPE:EVENT\r\n#EXT-X-TARGETDURATION:10\r\n"
--#EXT-X-ENDLIST

function handle1()
	local ok, idx = redis_api.cmd("private", "", "get", "index")

	local append = "\r\n#EXTINF:10,\r\nhttp://127.0.0.1:2222/gainTS?file=%03d.ts\r\n"
	local tbs = {}
	for i = 0, 1 do
		table.insert(tbs, string.format(append, idx + i))
		redis_api.cmd("private", "", "incrby", "index", 1)
	end
	local data = string.format(data1, idx) .. table.concat(tbs, "")

	print(data)
	local afp = supex.rgs(200)
	supex.say(afp, data)
	return supex.over(afp)
end



local data2 = "#EXTM3U\r\n#EXT-X-VERSION:3\r\n#EXT-X-MEDIA-SEQUENCE:0\r\n#EXT-X-PLAYLIST-TYPE:EVENT\r\n#EXT-X-TARGETDURATION:10\r\n"
--#EXT-X-ENDLIST

function handle2()
	local ok, idx = redis_api.cmd("private", "", "get", "index")
	redis_api.cmd("private", "", "incrby", "index", 1)

	local append = "\r\n#EXTINF:10,\r\nhttp://127.0.0.1:2222/gainTS?file=%03d.ts\r\n"
	local tbs = {}
	for i = 0, idx do
		table.insert(tbs, string.format(append, i))
	end
	local data = data2 .. table.concat(tbs, "")

	print(data)
	local afp = supex.rgs(200)
	supex.say(afp, data)
	return supex.over(afp)
end



handle = handle1
