local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_chatCommunionRet', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]


	local chat_tab = {
		[1] = "downstream",
		[2] = "uid",
		[3] = obj["content"]["accountID"],
		[4] = dtab[3]
	}
	local ok = zmq_api.cmd("downstream", "send_table", chat_tab)

end

