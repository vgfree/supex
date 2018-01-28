local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")

module("setting", package.seeall)

local function status_process(status, cid)
	if status == nil or cid == nil then
		only.log('E', 'Invalid parameter !')
	end

	local ok, ret
	--每次连接，都将以CID_Status为key，‘connected’为value存储。
	if status == 'connected' then
		ok, ret = redis_api.cmd('IMX', '', 'SET', 'CID_Status:' .. cid, status)
		if not (ok and ret) then
			only.log('E', 'redis save status failed')
		end
	end
	--每次断开，都将删除CID_Status的key。	
	if status == 'closed' then
		ok, ret = redis_api.cmd('IMX', '', 'DEL', 'CID_Status:' .. cid)
		if not (ok and ret) then
			only.log('E', 'redis delete status failed')
		end

		--通过CID拿到UID
		local ok, uid = redis_api.cmd('IMX', '', 'GET', 'CID_UID:' .. cid)
		if not ok then
			only.log('E', 'redis get uid failed')
		end
		--删除CID对应的UID
		ok, ret = redis_api.cmd('IMX', '', 'DEL', 'CID_UID:' .. cid)
		if not (ok and ret) then
			only.log('E', 'redis del CID failed, or empty in redis')
		end

		--删除UID对应的CID
		if uid ~= nil then
			ok, ret = redis_api.cmd('IMX', '', 'DEL', 'UID_CID:' .. uid)
			if not (ok and ret) then
				only.log('E', 'redis del UID failed, or empty in redis')
			end
		end

		--通过CID拿到GID
		ok, gids = redis_api.cmd('IMX', '', 'SMEMBERS', 'CID_GID:' .. cid)
		only.log('D', 'gid = %s', scan.dump(gids))
		if not ok then
			only.log('E', 'redis SMEMBERS data failed, or empty in redis')
		end
		--删除CID对应的GID
		ok, ret = redis_api.cmd('IMX', '', 'DEL', 'CID_GID:' .. cid)
		if not (ok and ret) then
			only.log('D', 'redis delete GID failed, or empty in redis')
		end

		for _, gid in pairs(gids or {}) do
			--删除GID里面的CID
			ok, ret = redis_api.cmd('IMX', '', 'SREM', 'GID_CID:' .. gid, cid)
			if not (ok and ret) then
				only.log('E', 'redis delete GID failed, or empty in redis')
			end
		end

	end
end

function statusInfoSave(table)
	local status = table[2]
	local cid = table[3]

	status_process(status, cid)
end

function appSrvInfoSave(table)
	local action = table[2]
	local cid = table[3]

	local ok, ret
	--绑定CID与UID
	if action == 'uidmap' then
		local uid = table[4]
		ok, ret = redis_api.cmd('IMX', '', 'SET', 'UID_CID:' .. uid, cid)
		if not (ok and ret) then
			only.log('E', 'redis save failed')
		end
		ok, ret = redis_api.cmd('IMX', '', 'SET', 'CID_UID:' .. cid, uid)
		if not (ok and ret) then
			only.log('E', 'redis save failed')
		end
	end
	--绑定CID与GID
	if action == 'gidmap' then
		local gid = table[4]
		ok, ret = redis_api.cmd('IMX', '', 'SADD', 'GID_CID:' .. gid, cid)
		if not (ok and ret) then
			only.log('E', 'redis save failed')
		end
		ok, ret = redis_api.cmd('IMX', '', 'SADD', 'CID_GID:' .. cid, gid)
		if not (ok and ret) then
			only.log('E', 'redis save failed')
		end
	end

	--强制下线
	--if action == 'status' then
	--	local status = table[4]
	--	status_process(status, cid)
	--end
end


