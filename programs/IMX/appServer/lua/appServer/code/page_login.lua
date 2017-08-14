local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('page_login', package.seeall)


function handle( obj )
	local dtab = supex["_DATA_"]
	local cid = dtab[2]
	
	-->bind
	local bind_tab = {
		[1] = "setting",
		[2] = "uidmap",
		[3] = cid,
		[4] = obj["accountID"]
	}
	local ok = zmq_api.cmd("setting", "send_table", bind_tab)

	-->TODO 获取群组 bind
	--
	-->返回结果
	local msg = '{"ERRORCODE":"0","RESULT":"success"}'
	local back_tab = {
		[1] = "downstream",
		[2] = "cid",
		[3] = cid,
		[4] = msg
	}
	local ok = zmq_api.cmd("downstream", "send_table", back_tab)
	-->TODO:通知好友
	local msg = string.format('{
		"action":"fLogin",
		"chatToken":"%s",
		"timestamp":"%s",
		"content":{"online":["%s"]}}',
		gettoken,
		os.time(),
		obj["accountID"]
		)
	local fLogin_tab = {
		[1] = "downstream",
		[2] = "uid",
		[3] = ???,
		[4] = msg
	}
	local ok = zmq_api.cmd("downstream", "send_table", fLogin_tab)
	
end

