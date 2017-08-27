local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_login', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]
	local uid = obj["accountID"]
	
	-->bind uid
	local bind_tab = {
		[1] = "setting",
		[2] = "uidmap",
		[3] = cid,
		[4] = uid
	}
	local ok = zmq_api.cmd("setting", "send_table", bind_tab)

	-->bind gid
        local ok, gids = redis_api.cmd("save_store", '', "smembers", uid .. ":allChatGroup")
        if not ok then
                only.log('E', 'failed get allChatGroup!')
	else
		for _, gid in pairs(gids or {}) do
			local bind_tab = {
				[1] = "setting",
				[2] = "gidmap",
				[3] = cid,
				[4] = gid
			}
			local ok = zmq_api.cmd("setting", "send_table", bind_tab)
		end
        end


	-->返回结果
	local msg = '{"action":"loginRsp","ERRORCODE":"0","RESULT":"success"}'
	local back_tab = {
		[1] = "downstream",
		[2] = "cid",
		[3] = cid,
		[4] = msg
	}
	local ok = zmq_api.cmd("downstream", "send_table", back_tab)


	-->通知好友
        local ok, frds = redis_api.cmd("save_store", '', "smembers", uid .. ":allFriends")
        if not ok then
                only.log('E', 'failed get allFriends!')
	else
		for _, frd in pairs(frds or {}) do
			local gettoken = "1111111"
			local msg = string.format('{"action":"fLogin","chatToken":"%s","timestamp":"%s","content":{"online":["%s"]}}',
				gettoken,
				os.time(),
				uid)
			local fLogin_tab = {
				[1] = "downstream",
				[2] = "uid",
				[3] = frd,
				[4] = msg
			}
			local ok = zmq_api.cmd("downstream", "send_table", fLogin_tab)
		end
	end
end

