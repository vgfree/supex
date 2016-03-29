-- auth: baoxue
-- time: 2014.04.27

local redis_api 	= require('redis_pool_api')
local cjson 		= require('cjson')
local supex 		= require('supex')
local only		= require('only')
local cachekv		= require("cachekv")

module("app_utils", package.seeall)



--名  称：get_city_code
--功  能：通过经纬度获取城市到代码
--参  数：accountID
--返回值：城市代码
function get_city_code()
	local accountID = supex.get_our_body_table()["accountID"]
        local longitude = supex.get_our_body_table()["altitude"][1]
        local latitude  = supex.get_our_body_table()["altitude"][1]
	local grid = string.format('%d&%d', tonumber(lon) * 100, tonumber(lat) * 100)
        only.log('D',"Grid string :" .. tostring(grid))
        local ok,jo =  redis_api.cmd('mapGridOnePercent',accountID,'get', grid)
        only.log('D',"cityCode :" .. tostring(jo))
        if ok and jo then
            local ok,info = pcall(cjson.decode,jo)
            if not ok or not info then
                only.log('E',"json result error!-->" .. info)
            else
                local cityCode = info["cityCode"]
                if cityCode then
                    return cityCode
                end
            end
        end
end



function get_user_msg_subscribed (accountID)
	local ok, value = cachekv.pull('private', accountID, 'get', accountID .. ':userMsgSubscribed');

	only.log('D', string.format("[get value][key:%s][ok:%s][value:%s]", 
	accountID .. ':userMsgSubscribed', ok, value))
	if ok then
		return value
	end
end

--函数:get_current_online_time
--功能:计算当前用户的在线时间
local function get_current_online_time(accountID)
	local ok, cfg_time = cachekv.pull('driview', accountID, 'get', accountID .. ':configTimestamp')

	if ok and cfg_time then
		local timestamp = os.time()
		local value = timestamp - cfg_time
		return value
	end
	return 0
end
