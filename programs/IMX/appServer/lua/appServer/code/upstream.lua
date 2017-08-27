local only = require('only')
local supex = require('supex')
local cjson = require('cjson')

module('upstream', package.seeall)

local ACTION_LIST = {
	login		= require("page_login").handle,
	heart		= require("page_heart").handle,

	amigos		= require("page_amigos").handle,
	groups		= require("page_groups").handle,

	amigoJoin	= require("page_amigoJoin").handle,
	groupJoin	= require("page_groupJoin").handle,

	chatAmigo	= require("page_chatAmigo").handle,
	chatAmigoRsp	= require("page_chatAmigoRsp").handle,
	chatGroup	= require("page_chatGroup").handle,
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

