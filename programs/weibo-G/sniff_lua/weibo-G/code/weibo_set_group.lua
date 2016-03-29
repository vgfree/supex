local only = require('only')
local supex = require('supex')
local luakv_api 	= require('luakv_pool_api')

module('weibo_set_group', package.seeall)

function handle()
	local args = supex.get_our_body_table()
	local GID = args["GID"]
	local UID = args["UID"]
	if GID and UID then
		luakv_api.cmd("owner", GID, "SADD", GID, UID)
	end
end

