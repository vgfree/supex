local only = require('only')
local utils = require('utils')
local link = require('link')
local weibo = require('weibo')
local supex = require('supex')
local scan = require('scan')
module('p2p_fetch_4_miles_ahead_poi', package.seeall)

app_info = {
        appKey = "2491067261",
        secret = "52E8DCDEB8DBBAD220652851AE34339B008F5B48",
}

local poitype_table = {
    ["1123112"] = true,
    ["1123114"] = true,
    ["1117102"] = true,
    ["1118101"] = true,
    ["1123106"] = true,
    ["1126157"] = true,

    --and new poitype -2015-08-01
    ["1131103"] = true,
    ["1131102"] = true,
    ["1131101"] = true,
    ["1131106"] = true,
    ["1118109"] = true,
    ["1123102"] = true,
    ["1122108"] = true,
    ["1122105"] = true,
    ["1122107"] = true,
    ["1122101"] = true,
    ["1115101"] = true,
}


local function check_poi_type(poiType)
	local poiType = tostring(poiType)
    if   poitype_table[poiType] == true then
		return true
	else
		return false
	end
end

local function send_weibo(url_tb,wb)
	local path = "/customizationapp/v2/callbackFetch4MilesAheadPoi"
	local callback_serv = link["OWN_DIED"]["http"][path] 
	local callback_url = utils.gen_url(url_tb)
	wb['callbackURL'] =  "http://" .. callback_serv['host'] .. ":" .. callback_serv['port'] .. path .. "?" .. callback_url 
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	for i, v in pairs(wb or {}) do
		only.log("D", string.format("[%s:%s]", i, v))
	end
	if wb["multimediaURL"] then
		local ok,ret = weibo.send_weibo( server, "personal", wb, app_info['appKey'], app_info['secret'] )
	end
end

function handle()
	local lon           = supex.get_our_body_table()["longitude"][1] 
	local lat           = supex.get_our_body_table()["latitude"][1]
	local accountID     = supex.get_our_body_table()["accountID"]
	local poitype_table = supex.get_our_body_table()["private_data"]

	local amr_host  = link["OWN_DIED"]["http"]["4milesAMR"]["host"]
	local fmt = "http://%s/productList/POIRemind/%s.amr"
	local wb ={
		receiverAccountID = accountID,
		level = 35, 
		senderType = 2,
	}
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
				wb["multimediaURL"] = string.format(fmt,amr_host,poiID )
		
			else
				wb["multimediaURL"] = string.format( fmt,amr_host,poitype )
			end
			wb["receiverLongitude"] = value["receiverLongitude"]
			wb["receiverLatitude"]  = value["receiverLatitude"]
			wb["receiverDirection"] = value["receiverDirection"]
			wb["content"]           = value["context"]
			wb["receiverDistance"]  = value["receiverDistance"]
			wb["interval"]          = value["interval"]

			local position_type = string.sub(poitype,0,7)
			local url_tb  = {
				POILongitude    = wb["receiverLongitude"],
				POILatitude     = wb["receiverLatitude"],
				POIDirection    = wb["receiverDirection"],
				POIContent      = wb["content"],
				positionType    = position_type,
				POIID           = nil,
				roadID          = nil,
			}
			send_weibo(url_tb,wb)
		else
			local context = value["context"]
			value["context"] = nil
			for k,v in pairs(value) do
				only.log('E',scan.dump(v))
				wb["multimediaURL"] = string.format( fmt,amr_host,poitype )
				wb["receiverLongitude"] = v["receiverLongitude"]
				wb["receiverLatitude"]  = v["receiverLatitude"]
				wb["receiverDirection"] = v["receiverDirection"]
				wb["content"]           = context
				wb["receiverDistance"]  = v["receiverDistance"]
				wb["interval"]          = v["interval"]

				local position_type = string.sub(poitype,0,7)
				local url_tb  = {
					POILongitude    = wb["receiverLongitude"],
					POILatitude     = wb["receiverLatitude"],
					POIDirection    = wb["receiverDirection"],
					POIContent      = wb["content"],
					positionType    = position_type,
					POIID           = nil,
					roadID          = nil,
				}
				send_weibo(url_tb,wb)
			end
		end

	end
end
