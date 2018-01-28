local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_chatAmigoRsp', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]

	local chat_tab = {
		[1] = "downstream",
		[2] = "uid",
		[3] = obj["message"]["fromAccountID"],
		[4] = dtab[3]
	}
	local ok = _G.app_lua_send_stream(chat_tab)
end

