local utils     = require('utils')
local only      = require('only')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')

module("looking", package.seeall)

local function get_status_from_redis(tab)
	if tab[3] == "cid" then
		local ok, ret = redis_api.cmd('IMX', '', 'GET', 'CID_Status:' .. tab[4])
		if not (ok and ret) then
			only.log('E', 'Get data failed or no suitable data')
		end
		return ret and 'connected' or 'closed'
	end

	if tab[3] == "uid" then
		local ok, cid = redis_api.cmd('IMX', '', 'GET', 'UID_CID:' .. tab[4])
		if not (ok and cid) then
			only.log('E', 'Get data failed or no suitable data')
		end
		return cid and 'connected' or 'closed'
	end
end

local function get_cid_with_group(tab)
	if tab[3] == "gid" then
		local ok, res = redis_api.cmd('IMX', '', 'SMEMBERS', 'GID_CID:' .. tab[4])
		if not (ok and res) then
			only.log('E', 'Get data failed or no suitable data')
			return nil
		end
		return table.concat(res or {}, '|')
	end
end

function appSrvInfoLoad(table)
	if table[2] == "status" then
		return get_status_from_redis(table)
	end
	
	if table[2] == "gidmap" then
		return get_cid_with_group(table)
	end
end
