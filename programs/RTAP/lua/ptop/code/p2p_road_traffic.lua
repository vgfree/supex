local only = require('only')
local weibo = require('weibo')
local redis_api = require('redis_pool_api')
local utils = require('utils')
local WORK_FUNC_LIST = require('WORK_FUNC_LIST')
local cjson = require('cjson')
local scan = require('scan')
local supex = require('supex')


module('p2p_road_traffic', package.seeall)

local app_info = {                                                                               
        appKey = "1854678079",
        secret = "439581000FF13D23F4A23D7DD61C2144B10AFA64",
}

function handle()
	-->打印日志
	only.log("I", "p2p_road_traffic_remind working ... ")	
	-->用户订阅权限判断
       	if not (weibo.user_control('p2p_road_traffic_remind')) then
       		return false
       	end
	-->获取私有数据
	local accountID = supex.get_our_body_table()["accountID"]
	local longitude	= supex.get_our_body_table()["longitude"]
	local latitude	= supex.get_our_body_table()["latitude"]
	local direction	= supex.get_our_body_table()["direction"]
	local speed 	= supex.get_our_body_table()["speed"]	
	local interval	= 30
	
	-->通过http从地址获取道路信息
	local wb ={}
	wb['appKey'] 	 = app_info["appKey"]
	wb['uid']  	 = accountID
	wb['longitude']  = longitude[1]
	wb['latitude']   = latitude[1]
	wb['direction']  = direction[1]
	wb['speed']	 = speed[1]
	wb['HICount']	 = 5
	wb['resultType'] = 3

	local sign = utils.gen_sign(wb, app_info['secret'])
	wb['sign'] = sign
	only.log('D',scan.dump(wb))

	local serv = link["OWN_DIED"]["http"]["roadRankapi/v2/getFrontTrafficInfoByLBS"]
	local req = utils.compose_http_kvps_request(serv, "roadRankapi/v2/getFrontTrafficInfoByLBS", nil, wb, "POST")
	local ok, ret = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not ret then 
		only.log("D","post is fail")
		return false 
	end
	only.log('D',ret)
	ret = string.match(ret, "{.+}")
	local ok, res = pcall(cjson.decode, ret) 
	only.log('D',"---------------------------")
	only.log('D',scan.dump(res))
	only.log('D',"---------------------------")
	if not ok then 
		only.log('E', "failed to decode result")
	end  
	if(tonumber(res['ERRORCODE']) ~= 0) then
		only.log('D', "get traffic Information failed")
		return false
	end
	local traffic =  res['RESULT']['traffic'] 
	if not traffic then
		only.log('E', "no traffic")
		return false
	end

	local trafficText = traffic["trafficText"]
	only.log('D',scan.dump(trafficText))

	for i=1,#trafficText do
		local text = trafficText[i]["text"]
   		only.log('D', "text:" .. text)
		-->文本转语音
		WORK_FUNC_LIST["spx_txt_to_url_send_weibo"]( "p2p_road_traffic_remind", text )
	end
end
