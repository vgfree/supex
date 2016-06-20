local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../open/lib/?.lua;",
        "../../open/apply/?.lua;",
        "../../open/spxonly/?.lua;",
        "../../open/linkup/?.lua;",
        "../../open/public/?.lua;",
        "open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local CFG_LIST	= require('cfg')

redis_api.init()

local function getData(key, cmd, redis_num)
	if key == nil or cmd == nil then
		return nil
	end
	local ok_status, ok_ret = redis_api.cmd('redis' .. tostring(redis_num), '', cmd, key)
        if not (ok_status and ok_ret) then
        	only.log('E', 'Get data failed or no suitable data')
		return nil        
	end
	return ok_ret
end

local function setData(key, data)
	if key == nil or data == nil then
		return nil
	end
	local ok_status, ok_ret = redis_api.cmd('tsdb01', '', 'SET', key, data)
	if not (ok_status and ok_ret) then
        	only.log('E', 'Set data failed or no suitable data')
		return nil        
	end
	print(ok_ret)
	return ok_ret
end

local function assembleData(userData)
	if userData == nil then
		only.log('E', "The userData is nil.")
		return nil
	end
	table.sort(userData)
	only.log('E', scan.dump(userData))

	--拿到每一行|出现的次数
	local str = userData[1]
	local xxx, count = string.gsub(str, '|', '|')
	local data = #userData .. '*' .. count + 1 .. '@'
	only.log('E', scan.dump(data))
	
	for i = 1, #userData do
		data = data .. userData[i] .. '|'	
	end

	only.log('E', scan.dump(data))

	return data
end

local function getDataWithUser(user, targetTime, redis_num)
	local dataKey = nil
	if user == nil then
		return nil
        end

	if targetTime == nil then
		return nil
	end
	
	if redis_num == nil then
		return nil
	end

	targetTime = string.gsub(targetTime, CFG_LIST['USER_PART_KEY'] .. ':', '')
	print(targetTime)

	local userData = nil
	dataKey = CFG_LIST['gps_key'] .. user .. ':' .. targetTime
	userData = getData(dataKey, 'SMEMBERS', redis_num)
	only.log('E', scan.dump(userData))
	local data = assembleData(userData)
	setData(dataKey, data)
end

function GetTable(tab)
	print(tab[1])
	print(tab[2])
	print(tab[3])
	getDataWithUser(tab[1], tab[2], tab[3])
end

