local only      = require('only')
local redis_api = require('redis_pool_api')

module("api_redis_looking", package.seeall)

local function get_redis_data(key, cmd)
        if key == nil or cmd == nil then
                return nil
        end
        local ok_status, ok_ret = redis_api.cmd('IdKey', '', cmd, key)
        if not (ok_status and ok_ret) then
                only.log('E', 'Get data failed or no suitable data')
                return nil
        end
        return ok_ret
end

local function get_status_from_redis(tab)
	local redis_ret = nil
	if tab[3] == "cid" then
		redis_ret = get_redis_data('CID_Status:' .. tab[4], 'GET')
		if redis_ret ~= 'connected' then
			redis_ret = 'closed'
		end

		return redis_ret
	end
	
	if tab[3] == "uid" then
		local cid = get_redis_data('UID:' .. tab[4], 'GET')
		if cid == nil then
			redis_ret = 'closed'
		end
		
		redis_ret = get_redis_data('CID_Status:' .. cid, 'GET')
		if redis_ret ~= 'connected' then
                        redis_ret = 'closed'
                end

		return redis_ret
	end
end

local function get_uid_with_group(tab)
	local uid = ''
	if tab[3] == "gid" then
		local redis_ret = get_redis_data('GID:' .. tab[4], 'SMEMBERS')
		if redis_ret ~= nil and #redis_ret > 0 then
			for idx = 1, #redis_ret do
				uid = uid .. redis_ret[idx] .. '|'
			end
		end

                return uid
	end
end

function get_data(table)
	local ret_tab = {}
	local ret_str = nil
	if table[2] == "status" then
		ret_str =  get_status_from_redis(table)
		return ret_str
	end
	
	if table[2] == "uidmap" then
		ret_tab = get_uid_with_group(table)
		return ret_tab
	end
end
