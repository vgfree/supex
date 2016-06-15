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

redis_api.init()

local key1 = link["OWN_DIED"]["key"]["KEY1"]
local key2 = link["OWN_DIED"]["key"]["KEY2"] 

local function getData(key, cmd)
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

local function getUserWithKey1(key1, targetTime)
	local userKey = nil
	if key1 ~= nil and targetTime ~= nil then
		userKey = key1 .. targetTime
	end
	return getData(userKey, 'SMEMBERS')
end

local function getDataWithUser(user, targetTime)
	local dataKey = nil
	if user == nil or #user == 0 then
		return nil
        end

	if targetTime == nil then
		return nil
	end
	
	local userData = nil
	for i = 1, #user do
		dataKey = key2 .. user[i] .. ':' .. targetTime
		userData = getData(dataKey, 'GET')
		only.log('E', scan.dump(userData))
	end
end

local function sendDataToClient(table)
end

function GetTable(tab)
	--local targetTime = '20160612171'
	--local user = getUserWithKey1(key1, targetTime)
	--only.log('E', scan.dump(user))
	--if user ~= nil then
	--	getDataWithUser(user, targetTime)
	--endi
	print(tab[1])
	print(tab[2])
end

