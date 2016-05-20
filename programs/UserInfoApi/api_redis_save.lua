local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local zmq	= require("zmq")

module("api_redis_save", package.seeall)

local ctx = zmq.init(1)
local s = ctx:socket(zmq.PUSH)

local function loginServerInfoSave(table)
	local status = table[2]
	local cid = table[3]
	local ok_status, ok_ret

	if string.find(status,'connected') ~= nil or string.find(status,'closed') ~= nil then
		ok_status, ok_ret = redis_api.cmd('IdKey', '', 'set', 'CID_Status:' .. cid, status)
		if not (ok_status and ok_ret) then
                        only.log('D', 'redis save failed')
                end
	end
			
end

local function appServerInfoSave(table)
	local actionStr = table[2]
	local cid = table[3]
	local ok_status, ok_ret

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

	if string.find(actionStr,'gidmap') ~= nil then
		local gid = table[4]
		local gidTable = utils.str_split(gid, ',')
		--local gidStr = ''
		only.log('D', 'gid table = %s', scan.dump(gidTable))

		for i=1, #gidTable do
			ok_status, ok_ret = redis_api.cmd('IdKey', '', 'SADD', 'GID:' .. gidTable[i], cid)
                	if not (ok_status and ok_ret) then
                        	only.log('D', 'redis save failed')
                	end
		end
	end
	
end

local function parseFirstFrame(table)
	if table[1] == 'status' then	
		loginServerInfoSave(table)
	end
	if table[1] == 'setting' then
		appServerInfoSave(table)
	end
end

local function sendToSettingServer(table)
	s:connect("tcp://localhost:5559")
	s:send_table(table)
end

function handle(table)
	parseFirstFrame(table)
	sendToSettingServer(table)
end
