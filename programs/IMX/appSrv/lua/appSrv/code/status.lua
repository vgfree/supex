local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('status', package.seeall)


function handle( )
	local dtab = supex["_DATA_"]
	local ctl = dtab[2]
	local cid = dtab[3]

	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])

	if ctl == "connected" then
	end
	if ctl == "closed" then
		-->TODO:通知好友
		local gettoken = "11111111"
		local uid = "baoxue"--TODO
		-->通知好友
		local ok, frds = redis_api.cmd("save_store", '', "smembers", uid .. ":allFriends")
		if not ok then
			only.log('E', 'failed get allFriends!')
		else
			for _, frd in ipairs(frds or {}) do
				local gettoken = "1111111"
				local msg = string.format('{"action":"fLogin","chatToken":"%s","timestamp":"%s","content":{"offline":["%s"]}}',
					gettoken,
					os.time(),
					uid)
				local fLogin_tab = {
					[1] = "downstream",
					[2] = "uid",
					[3] = frd,
					[4] = msg
				}
				local ok = _G.app_lua_send_stream(fLogin_tab)
			end
		end
	end
	
end

