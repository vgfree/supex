local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local cjson = require('cjson')
local scan = require('scan')

module('upstream', package.seeall)

function handle()
	-- 打印上行数据
	print("调用upstream")
	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])
	print(supex["_DATA_"][4])

	local frame = {}

	if supex["_DATA_"][3] == 'bind' then
		print("数据下发, 绑定uidmap")
		local uid_tab = cjson.decode(supex["_DATA_"][4])
		frame[1] = 'setting'
		frame[2] = 'uidmap'
		frame[3] = supex["_DATA_"][2]
		frame[4] = uid_tab['uid']

		local ok = zmq_api.cmd("setting", "send_table", frame)
		print("uidmap绑定完成")

	else
		-- 测试时第三帧只有字符串, 接收到的数据直接下发
		print("downstream 下发个人消息数据")
		frame[1] = 'downstream'
		frame[2] = 'cid'
		frame[3] = supex["_DATA_"][2]
		frame[4] = supex["_DATA_"][3]

		print("frame = ", scan.dump(frame))

		local ok = zmq_api.cmd("downstream", "send_table", frame)
		print("downstream下发完成")
	end


	--[[
	local recv_tab = cjson.decode(supex["_DATA_"][3])
	local content = recv_tab['content']
	local frame = {}

	if recv_tab['action'] == 'chat' then
	print("downstream下发个人消息数据")
	frame[1] = 'downstream'
	frame[2] = 'uid'
	frame[3] = content['toAccountID']
	frame[4] = content['fileUrl']

	print("frame = ", scan.dump(frame))

	local ok = zmq_api.cmd("downstream", "send_table", frame)
	print("downstream下发完成")

	elseif recv_tab['opt'] == 'bind' then
	printf('下发数据, 绑定uidmap')
	frame[1] = 'setting'
	frame[2] = 'uidmap'
	frame[3] = supex["_DATA_"][2]
	frame[4] = recv_tab['uid']

	print("frame = %s", scan.dump(frame))

	local ok = zmq_api.cmd("setting", "send_table", frame)
	print('绑定操作完成')

	elseif recv_tab['opt'] == 'chatGroup' then
	print("downstream下发群消息数据")
	frame[1] = 'downstream'
	frame[2] = 'gid'
	frame[3] = content['chatGroupID']
	frame[4] = content['fileUrl']

	print("frame = ", scan.dump(frame))

	local ok = zmq_api.cmd("downstream", "send_table", frame)
	print("downstream下发完成")


	end
	--]]
end

