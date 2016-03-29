local only = require('only')
local weibo = require('weibo')
local supex = require('supex')
local scan = require('scan')
local WORK_FUNC_LIST = require('WORK_FUNC_LIST')

module('p2p_fetch_4_miles_ahead_poi', package.seeall)

app_info = {
        appKey = "2491067261",
        secret = "52E8DCDEB8DBBAD220652851AE34339B008F5B48",
}

local poitype_table = {
    ["1065"] = true,
    ["1024"] = true,
    ["1009"] = true,
    ["1081"] = true,
    ["1008"] = true,
    ["1025"] = true,

    --and new poitype -2015-08-01
    ["1135"] = true,
    ["1192"] = true,
    ["1036"] = true,
    ["1236"] = true,
    ["1292"] = true,
    ["1136"] = true,
    ["1067"] = true,
    ["1190"] = true,
    ["1191"] = true,
    ["1003"] = true,
    ["1018"] = true,
    ["1022"] = true,
    ["1034"] = true,
    ["1031"] = true,
}


local function check_poi_type(poiType)
	local poiType = tostring(poiType)
    if   poitype_table[poiType] == true then
		return true
	else
		return false
	end
end


function handle()
	-->打印日志
        only.log("I", "p2p_fetch_4_miles_ahead_poi working ... ")
	-->用户订阅权限判断
        if not (weibo.user_control('p2p_fetch_4_miles_ahead_poi')) then
                return false
        end
	local poitype_table = supex.get_our_body_table()["private_data"]

	for poitype,value in pairs(poitype_table) do
		local poitype  = string.sub(poitype,0,7)
		if not weibo.check_subscribed_msg( accountID, weibo["DRI_APP_4_MILES"][poitype]["no"] ) then
			poitype_table[poitype] = nil
		end
	end
	if not next(poitype_table) then
		return false
	end
	only.log('D',scan.dump(poitype_table))
	for poitype,value in pairs(poitype_table) do
		if poitype ~= "1123110" and poitype ~= "1123111" then
			local ok = check_poi_type( poitype )          
			if ok then
				local poiID = value["poiID"] 
				WORK_FUNC_LIST["half_url_unknow_str_send_weibo"]("p2p_fetch_4_miles_ahead_poi", poiID)	                             
		
			else
				WORK_FUNC_LIST["half_url_unknow_str_send_weibo"]("p2p_fetch_4_miles_ahead_poi", poitype)	                             
			end
		else
			for k,v in pairs(value) do
				only.log('D',scan.dump(v))
				WORK_FUNC_LIST["half_url_unknow_str_send_weibo"]("p2p_fetch_4_miles_ahead_poi", poitype)

			end
		end

	end
end
