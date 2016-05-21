local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local zmq	= require("zmq")

module("api_redis_save", package.seeall)

local function status_process(status, cid)
	if status == nil or cid == nil then
		only.log('D', 'Invalid parameter !')
	end

	local ok_status, ok_ret
	--每次连接或断开，都将以CID_Status为key，‘connected’和‘closed’为value存储。
	if string.find(status,'connected') ~= nil or string.find(status,'closed') ~= nil then
                ok_status, ok_ret = redis_api.cmd('IdKey', '', 'set', 'CID_Status:' .. cid, status)
                if not (ok_status and ok_ret) then
                        only.log('D', 'redis save failed')
                end
	end
	if string.find(status,'closed') ~= nil then
		--通过cid拿到uid
		ok_status, ok_ret = redis_api.cmd('IdKey', '', 'GET', 'CID:' .. cid)
        	if not (ok_status and ok_ret) then
        		only.log('D', 'redis GET data failed, or empty in redis')
        	end
		local uid = ok_ret
        	--删除以CID:$[cid]为key的值
        	ok_status, ok_ret = redis_api.cmd('IdKey', '', 'del', 'CID:' .. cid)
        	if not (ok_status and ok_ret) then
        		only.log('D', 'redis del data failed, or empty in redis')
        	end
		--删除以UID:$[uid]为key的值
		if uid ~= nil then
        		ok_status, ok_ret = redis_api.cmd('IdKey', '', 'del', 'UID:' .. uid)
                	if not (ok_status and ok_ret) then
                		only.log('D', 'redis del data failed, or empty in redis')
                	end
        	end
		--删除以GID:$[GID]为key的值, 首先得用userGID:$[CID]得到gidTable
		ok_status, ok_ret = redis_api.cmd('IdKey', '', 'SMEMBERS', 'userGID:' .. cid)
		only.log('D', 'ok_ret = %s', scan.dump(ok_ret))
		if not (ok_status and ok_ret) then
                	only.log('D', 'redis SMEMBERS data failed, or empty in redis')
                end
		local gidTable = ok_ret
		for i=1, #gidTable do
			ok_status, ok_ret = redis_api.cmd('IdKey', '', 'del', 'GID:' .. gidTable[i])
                        if not (ok_status and ok_ret) then
                                only.log('D', 'redis delete GID failed, or empty in redis')
                        end
		end
		--删除以userGID:$[CID]为key的值
		ok_status, ok_ret = redis_api.cmd('IdKey', '', 'del', 'userGID:' .. cid)
		if not (ok_status and ok_ret) then
                	only.log('D', 'redis delete GID failed, or empty in redis')
                end
		
	end
end

function loginServerInfoSave(table)
	local status = table[2]
	local cid = table[3]
	status_process(status, cid)
			
end

function appServerInfoSave(table)
	local actionStr = table[2]
	local cid = table[3]
	local ok_status, ok_ret
	--以UID:$[uid]为key存cid， 以CID:$[cid],为key存uid
	if string.find(actionStr,'uidmap') ~= nil then
		local uid = table[4]
		ok_status, ok_ret = redis_api.cmd('IdKey', '', 'set', 'UID:' .. uid, cid)
		if not (ok_status and ok_ret) then
			only.log('D', 'redis save failed')
		end
		ok_status, ok_ret = redis_api.cmd('IdKey', '', 'set', 'CID:' .. cid, uid)
                if not (ok_status and ok_ret) then
                        only.log('D', 'redis save failed')
                end
	end
	--如果gidmap敢来，其对应的cid先清掉所有gid，然后再重新加入gid，可以处理用户切换群组的事件。
	if string.find(actionStr,'gidmap') ~= nil then
		status_process('closed', cid)
		local gid = table[4]
		local gidTable = utils.str_split(gid, ',')
		only.log('D', 'gid table = %s', scan.dump(gidTable))
		for i=1, #gidTable do
	--以GID$[gid]存cid,以userGID$[cid]为key存GID
			ok_status, ok_ret = redis_api.cmd('IdKey', '', 'SADD', 'GID:' .. gidTable[i], cid)
                	if not (ok_status and ok_ret) then
                        	only.log('D', 'redis save failed')
                	end
			ok_status, ok_ret = redis_api.cmd('IdKey', '', 'SADD', 'userGID:' .. cid, gidTable[i])
                        if not (ok_status and ok_ret) then
                                only.log('D', 'redis save failed')
                        end
		end
	end
	
	if string.find(actionStr,'status') ~= nil then
		local status = table[4]
		status_process(status, cid)
	end
	
end

