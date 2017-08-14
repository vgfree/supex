local only = require('only')
local supex = require('supex')

module('upstream', package.seeall)

local ACTION_LIST = {
	login		= require("page_login").handle,
	heart		= require("page_heart").handle,
	chat		= require("page_chat").handle,
	chatRsp		= require("page_chatRsp").handle,
	chatGroup	= require("page_chatGroup").handle,
	chatGroupRsp	= require("page_chatGroupRsp").handle,
	chatCommunion	= require("page_chatCommunion").handle,
	chatCommunionRet= require("page_chatCommunionRet").handle,
	frieds		= require("page_frieds").handle,

}

function handle()
	local dtab = supex["_DATA_"]
	local cid = dtab[2]
	local msg = dtab[3]

	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])
	
	local obj = cjson.decode(msg)

	local cmd = obj["action"]
	if ACTION_LIST[ cmd ] then
		local ok, err = pcall(ACTION_LIST[ cmd ], obj)
		if not ok then
			only.log('E', "%s", err)
		end
	end
end

