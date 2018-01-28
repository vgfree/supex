local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_groups', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]
	local uid = obj["accountID"]

        local ok, grps = redis_api.cmd("save_store", '', "smembers", uid .. ":allChatGroup")
        if not ok then
                only.log('E', 'failed get allFriends!')
	else
		-->返回结果
		local groups = table.concat(grps or {}, '","')
		if #groups ~= 0 then
			groups = '"' .. groups .. '"'
		end
		local msg = string.format('{"action":"groupsRsp","groups":[%s]}', groups)
		local back_tab = {
			[1] = "downstream",
			[2] = "uid",
			[3] = uid,
			[4] = msg
		}
		local ok = _G.app_lua_send_stream(back_tab)
	end
end

