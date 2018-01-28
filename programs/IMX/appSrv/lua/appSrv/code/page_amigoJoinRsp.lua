local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_amigoJoinRsp', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]

	local join_tab = {
		[1] = "downstream",
		[2] = "uid",
		[3] = obj["accountID"],
		[4] = dtab[3]
	}
	local ok = _G.app_lua_send_stream(join_tab)
	-->join aid
	if obj["ERRORCODE"] == "0" then
		local ok = redis_api.cmd("save_store", '', "sadd", uid .. ":allFriends", aid)
		if not ok then
			only.log('E', 'failed get allFriends!')
		end
	end
end

