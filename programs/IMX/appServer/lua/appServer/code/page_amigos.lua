local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_amigos', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]
	local uid = obj["accountID"]

        local ok, frds = redis_api.cmd("save_store", '', "smembers", uid .. ":allFriends")
        if not ok then
                only.log('E', 'failed get allFriends!')
	else
		-->返回结果
		local amigos = table.concat(frds or {}, '","')
		if #amigos ~= 0 then
			amigos = '"' .. amigos .. '"'
		end
		local msg = string.format('{"action":"amigosRsp","amigos":[%s]}', amigos)
		local back_tab = {
			[1] = "downstream",
			[2] = "uid",
			[3] = uid,
			[4] = msg
		}
		local ok = zmq_api.cmd("downstream", "send_table", back_tab)
	end
end

