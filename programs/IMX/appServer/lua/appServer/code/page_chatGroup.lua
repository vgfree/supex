local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_chatGroup', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]


	local chat_tab = {
		[1] = "downstream",
		[2] = "gid",
		[3] = obj["message"]["chatGroupID"],
		[4] = dtab[3]
	}
	local ok = zmq_api.cmd("downstream", "send_table", chat_tab)




	local msg = string.format('{"action":"chatGroupRsp","message":{"chatID":"%s","fromAccountID":"%s","chatGroupID":"%s"}}',
		obj["message"]["chatID"],
		obj["message"]["fromAccountID"],
		obj["message"]["chatGroupID"])

	local resp_tab = {
		[1] = "downstream",
		[2] = "uid",
		[3] = obj["message"]["fromAccountID"],
		[4] = msg
	}
	local ok = zmq_api.cmd("downstream", "send_table", resp_tab)
end

