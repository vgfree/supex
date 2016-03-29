local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')
local weibo_api = require('weibo')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')
local luakv_api = require('luakv_pool_api')

module('p2p_mon_driving_mileage_remind', package.seeall)

local app_info = {
	appKey = "3619608887",
	secret = "AEFA154AA9890EC4A9E0E49A3E9FE08C859D5844",
}

function handle()
	only.log('D', string.format("[p2p_mon_driving_mileage_remind]"))
	local idx_key = "monDriveOnlineMileagePoint"
	local accountID = supex.get_our_body_table()["accountID"]
	local lon       = supex.get_our_body_table()["longitude"] and supex.get_our_body_table()["longitude"][1]
	local lat       = supex.get_our_body_table()["latitude"] and supex.get_our_body_table()["latitude"][1]
	local speed     = supex.get_our_body_table()["speed"] and supex.get_our_body_table()["speed"][1]
	local direction = supex.get_our_body_table()["direction"] and supex.get_our_body_table()["direction"][1]
	local altitude  = supex.get_our_body_table()["altitude"] and supex.get_our_body_table()["altitude"][1]
	if accountID == nil then
		return false;
	end
	local ok, ret = redis_api.cmd('private',accountID, 'get', accountID .. ':' .. idx_key)
	if (not ok) or (not ret) then
		return false
	end
	only.log('D', string.format("[ret = %s]", ret))

	local index, base_mileage, now_mileage =  string.match(ret, "(%w+):(%w+):(%w+)")
	if index == nil or base_mileage == nil then
		only.log('E', string.format("[index == nil or base_mileage == nil]"))
		return false;
	end
	-- check for index, base_mileage, now_mileage
	only.log('D', string.format("[index:%s]", index))
	only.log('D', string.format("[base_mileage:%s]", base_mileage))
	only.log('D', string.format("[now_mileage:%s]", now_mileage))

	base_mileage        = tonumber(base_mileage) or 300;
	now_mileage         = tonumber(now_mileage) or 0;

	if index ~= "1" and index ~= "2"  then
		only.log('E', ("[Wrong index no.]"))
		return false;
	end

	local text;
	local txt = {
		[1] = "恭喜您本月有效里程已经达到%s公里，继续加油就能获取更多里程奖励",
		[2] = "温馨提醒, 你本月的有效驾驶里程为%s.",
		[3] = "温馨提醒，您本月的有效驾驶里程为%s，距离达标还有%d，继续加油哦",
	}
	--- 正常条件
	if index == "1" then
		text = string.format(txt[1], base_mileage)
	end

	if index == "2" then
		if now_mileage > base_mileage then
			text = string.format(txt[2], now_mileage);
		end
		if now_mileage < base_mileage then
			text = string.format(txt[3], now_mileage, 
			base_mileage - now_mileage);
		end
	end

	--> send weibo
	local wb = {
		appKey = app_info["appKey"],
		text = text,
	}
	local secret = app_info["secret"]
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign

	local serv = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
	local req = utils.compose_http_kvps_request(serv, "spx_txt_to_voice", nil, wb, "POST")
	only.log('D', req)
	local fileurl
	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then
		only.log('D',string.format("fileurl is nil"))
		fileurl = nil
	end
	if ok and resp then
		only.log('D', resp)
		local jo = utils.parse_api_result( resp, "spx_txt_to_voice")
		fileurl =  jo and jo["url"] or nil
	end

	if fileurl then
		only.log('D',string.format("fileurl:%s",fileurl))
	else
		return false
	end
	local wb = {
		multimediaURL = fileurl,
		receiverAccountID = accountID,
		interval = 86400,
		level = 50,
		content = text,
		senderType = 2,
		--sourceId = fileID,
	}
	if lon and lat then
		wb['senderLongitude'] = lon
		wb['senderLatitude'] = lat
	end
	wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	wb["senderSpeed"] =  string.format('[%s]', speed and math.ceil(speed) or 0)
	if altitude then
		wb["senderAltitude"] =  altitude
	end
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok, ret = weibo_api.send_weibo( server, "personal", wb, app_info["appKey"], app_info["secret"] )

	if ok then
		local bizid = ret['bizid']
		local time = os.time()
		local travelID  = nil
		local appKey = app_info["appKey"]
		local ok, imei =  redis_api.cmd('private',accountID,'get', accountID .. ':IMEI')
		if ok and imei then
			ok, travelID = redis_api.cmd('private',accountID,'get', imei .. ':accountID')
		end
		local ok_date, cur_date = luakv_api.cmd('private',accountID,'get', accountID .. ':speedDistribution')
		if not ok_date or not cur_date then
			cur_date = os.date("%Y%m")
			luakv_api.cmd('private',accountID,'set', accountID .. ':speedDistribution', cur_date)
		end 
	end
end
