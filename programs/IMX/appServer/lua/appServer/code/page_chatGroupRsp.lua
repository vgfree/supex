local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_chatGroupRsp', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]

	print("chatGroupRsp")
end

