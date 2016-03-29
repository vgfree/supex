local only = require('only')
local supex = require('supex')
local luakv_api 	= require('luakv_pool_api')

module('weibo_get_group', package.seeall)

function handle()
	local args = supex.get_our_body_table()
	local GID = args["GID"]
	if GID then
		local ok, users = luakv_api.cmd("owner", GID, "SMEMBERS", GID)
		print(users[1])
	end
end

