local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_groupJoin', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]
	local uid = obj["accountID"]
	local gid = obj["chatGroupID"]
	
	-->bind gid
        local ok, gids = redis_api.cmd("save_store", '', "sadd", uid .. ":allChatGroup", gid)
        if not ok then
                only.log('E', 'failed get allChatGroup!')
	else
		local bind_tab = {
			[1] = "setting",
			[2] = "gidmap",
			[3] = cid,
			[4] = gid
		}
		local ok = zmq_api.cmd("setting", "send_table", bind_tab)
        end


	-->返回结果
	local msg = '{"action":"groupJoinRsp","ERRORCODE":"0","RESULT":"success"}'
	local back_tab = {
		[1] = "downstream",
		[2] = "cid",
		[3] = cid,
		[4] = msg
	}
	local ok = zmq_api.cmd("downstream", "send_table", back_tab)
end

