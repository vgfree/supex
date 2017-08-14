local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local cjson = require('cjson')
local scan = require('scan')

module('upstream', package.seeall)

local function set_gidmap(cid, gid)
	local frame = {}
	print('下发数据, 绑定gid')
	frame[1] = 'setting'
	frame[2] = 'gidmap'
	frame[3] = cid
	frame[4] = 'gid0' -- TODO: 通过接口获取gid

	print("frame = ", scan.dump(frame))
	local ok = zmq_api.cmd("setting", "send_table", frame)
	print('***** 绑定gidmap,下发完成 *****')
end

local function set_uidmap(cid, uid)
	frame[1] = 'setting'
	frame[2] = 'uidmap'
	frame[3] = cid
	frame[4] = uid

	print("frame = ", scan.dump(frame))
	local ok = zmq_api.cmd("setting", "send_table", frame)
	print('***** 绑定uidmap,下发完成 *****')
end

local function send_single_msg(content)
	frame[1] = 'downstream'
	frame[2] = 'uid'
	frame[3] = content['toUser']

	local msg = {}
	msg['opt'] = 'msg'
	msg['fromUser'] = content['fromUser']
	msg['groupID'] = '' -- TODO: 个人消息是否需要?
	msg['msgTime'] = os.time()
	msg['msgID'] = '' -- TODO: 服务器生成ID
	msg['msgType'] = content['msgType']
	msg['msgObj'] = content['msgObj']

	frame[4] = cjson.encode(msg)

	print("frame = ", scan.dump(frame))
	local ok = zmq_api.cmd("downstream", "send_table", frame)
	print('***** 个人消息,下发完成 *****')
end

local function send_group_msg(content)
	frame[1] = 'downstream'
	frame[2] = 'gid'
	frame[3] = content['toGroup']

	local msg = {}
	msg['opt'] = 'msg'
	msg['fromUser'] = content['fromUser']
	msg['groupID'] = content['toGroup']
	msg['msgTime'] = os.time()
	msg['msgID'] = '' -- TODO: 服务器生成ID
	msg['msgType'] = content['msgType']
	msg['msgObj'] = content['msgObj']

	frame[4] = cjson.encode(msg)

	print("frame = ", scan.dump(frame))
	local ok = zmq_api.cmd("downstream", "send_table", frame)
	print('***** 群组消息,下发完成 *****')
end

function handle()
	-- 打印上行数据
	print("调用upstream")
	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])
	print(supex["_DATA_"][4])

	local frame = {}
	local content = cjson.decode(supex["_DATA_"][3])
	local cid = supex["_DATA_"][2]

	if content['opt'] == 'bind' then
		set_uidmap(cid, content['accountID'])
		set_gidmap(cid, content['accountID'])

	elseif content['opt'] == 'single_msg' then
		send_single_msg(content)

	elseif content['opt'] == 'group_msg' then
		send_group_msg(content)

	end

	--[[
	if supex["_DATA_"][3] == 'bind' then
		print("数据下发, 绑定uidmap")
		local uid_tab = cjson.decode(supex["_DATA_"][4])
		frame[1] = 'setting'
		frame[2] = 'uidmap'
		frame[3] = supex["_DATA_"][2]
		frame[4] = uid_tab['uid']

		local ok = zmq_api.cmd("setting", "send_table", frame)
		print("uidmap绑定完成")

		set_gidmap(supex["_DATA_"][2], uid_tab['uid'])
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

	--]]

end

