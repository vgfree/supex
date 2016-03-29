local only = require('only')
local weibo = require('weibo')
local utils = require('utils')
local supex = require('supex')
local redis_api = require("redis_pool_api") 
local scan = require('scan')
local WORK_FUNC_LIST = require('WORK_FUNC_LIST')
local p2p_assemble_scene = require('p2p_assemble_scene')

module('p2p_weather_forcast', package.seeall)

function handle()
        -->打印日志
        only.log("I", "p2p_weather_forcast working ... ")
        -->用户订阅权限判断
        if not (weibo.user_control('p2p_weather_forcast')) then
        	return false
        end
	-->获取当前信息
	local accountID = supex.get_our_body_table()["accountID"]
	if accountID == nil then
		return false;
	end
	local ok,currentBL = redis_api.cmd('private', accountID, "get", accountID .. ":currentBL")
	if not currentBL then
		only.log('E',"user currentBL is nil")
		return false
	end
	local info_tabe = utils.str_split(currentBL,',')
	local lon = info_tabe[1]
	local lat = info_tabe[2]
    	if not lon or not lat then
        	only.log('E',"grid is nil")
        	return false
    	end
	local grid = string.format('%d&%d', tonumber(lon) * 100, tonumber(lat) * 100)
	only.log("D", "redis grid key:" .. grid)
                                
	local ok, code_jo = redis_api.cmd('mapGridOnePercent01',accountID, 'hgetall', grid)
	if (not ok) or (not code_jo) then
		only.log("W", "can't get code from redis!")
		return false
	end
	only.log('D', "[code_jo = %s]", scan.dump(code_jo))
	-->获取provinceCode,cityCode
	local provinceCode = code_jo['provinceCode']
	local cityCode = code_jo['cityCode']
	if not cityCode and not provinceCode then
		only.log("E", "can't get cityCode from redis!")
		return false
	end
	-->获取天气信息
	local key = cityCode .. ":weatherForecast"
	only.log('D', "[key:%s]", key)
	local ok, ret = redis_api.cmd("public",accountID, "hgetall", cityCode .. ":weatherForecast")
	if (not ok) or (not ret) then
		-->直辖市的处理
		if code_jo['provinceName'] == '上海市' or 
			code_jo['provinceName'] == '北京市' or 
			code_jo['provinceName'] == '重庆市' or 
			code_jo['provinceName'] == '天津市' then
				ok, ret = redis_api.cmd("public",accountID, "hgetall", provinceCode .. ":weatherForecast")
				if (not ok) or (not ret) then
					only.log('D', accountID .. ':cityCode is NULL')		
					return false	
				end
		else
			only.log('D', accountID .. ':cityCode is NULL')	
			return false
		end
	
	end
	only.log('D', "[ret[text]:%s]", ret['text'])
	only.log('D', "[ret[cityName]:%s]", ret['cityName'])
	only.log('D', "[ret[temperature]:%s]", ret['temperature'])
	if not ret['text'] or not ret['cityName'] or not ret['temperature'] then
		return false
	end
	local cityName      = ret['cityName'] or nil
	local state         = ret['text'] or nil
	local temperature   = ret['temperature'] or nil
	local lastUpdate    = ret['lastUpdate'] or nil
	local today         = os.date("%Y-%m-%d")
	
	if not cityName or not state or not state or not temperature or not lastUpdate then
		only.log("D","get infomation fail")
		return false
	end
	if lastUpdate < today then
		only.log('D', string.format("[當前的天氣信息不是today的]"))
		return false
	end
	local text;
	local txt = {
		[1] = "%s当前天气%s，温度%s度",
	}
	text = string.format(txt[1], cityName, state, temperature)
	-->文本转语音
	WORK_FUNC_LIST["spx_txt_to_url_send_weibo"]( "p2p_weather_forcast", text )
	--p2p_assemble_scene.handle(state,"p2p_weather_forcast")
end
