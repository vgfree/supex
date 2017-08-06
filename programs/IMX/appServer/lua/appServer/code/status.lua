local only = require('only')
local supex = require('supex')
local cjson = require('cjson')
local scan = require('scan')

module('status', package.seeall)

function handle()
	-- 打印上行数据
	-- 0 status
	-- 1 connected
	-- 2 CID
	print("调用setting")
	print(supex["_DATA_"][1])
	print(supex["_DATA_"][2])
	print(supex["_DATA_"][3])

	local CID = supex["_DATA_"][3]

	local frame = {}

	if supex["_DATA_"][2] == 'connected' then
		-- 下发数据, 获取uid值
		print('下发数据, 获取uid值')
		-- 第一帧 setting
		frame[1] = 'downstream'
		-- 第二帧 status/uidmap/gidmap, 暂时写死
		frame[2] = 'cid'
		-- 第三帧 CID
		frame[3] = CID
		-- 第四帧 bind
		frame[4] = 'bind'

		print("frame = ", scan.dump(frame))

		local ok = zmq_api.cmd("downstream", "send_table", frame)
		print('下发完成')

	end
end

--[[
1. 在接收到客户端登录时发送的数据时, server需要获取uid
2. 在获取uid后, 将cid与uid执行绑定操作
--]]
