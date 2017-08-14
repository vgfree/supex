local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('status', package.seeall)


function handle( )
	local dtab = supex["_DATA_"]
	local ctl = dtab[2]
	local cid = dtab[3]

	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])

	if ctl == "closed" then
		-->TODO:通知好友
		local usr = redis_api.cmd()
		local msg = string.format('{
			"action":"fLogin",
			"chatToken":"%s",
			"timestamp":"%s",
			"content":{"offline":["%s"]}}',
			gettoken,
			os.time(),
			usr
		)
		local fLogin_tab = {
			[1] = "downstream",
			[2] = "uid",
			[3] = ???,
			[4] = msg
		}
		local ok = zmq_api.cmd("downstream", "send_table", fLogin_tab)
	end
	
end

