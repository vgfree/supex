local http_api = require("http_short_api")
local APP_CONFIG_LIST = require('CONFIG_LIST')
local common_cfg = require("cfg")
local mysql_api = require("mysql_pool_api")
local redis_api = require("redis_pool_api")
local cjson = require("cjson")
local only = require('only')
local link = require("link")
local utils = require("utils")
local supex = require("supex")
local link = require("link")
local weibo = require('weibo')
module("WORK_FUNC_LIST", package.seeall)



OWN_ARGS = {
	full_url_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		fullURL = 'http://127.0.0.1/productList/xxxxx/xxxx.amr',
	},

	half_url_unknow_str_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		fill_key = "arrivePoiFileUrl",		
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
	},
	half_url_incr_idx_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		idx_base = "30000",
		idx_max = 1,
		idx_key = "driveOnlineMileagePoint",
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
	},

	half_url_random_idx_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		idx_base = "30000",
		idx_max = 1,
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
	},
}
--功	能：对完整的url发送微博
--参	数：app_name,应用名字
function full_url_send_weibo( app_name )
	local body_tb	= supex.get_our_body_table()
	local accountID = body_tb["accountID"]
	local lon       = body_tb["longitude"][1]
	local lat       = body_tb["latitude"][1]
	local speed     = body_tb["speed"][1]
	local direction = body_tb["direction"][1]
	local altitude  = body_tb["altitude"][1]
	
	local info_tb	= APP_CONFIG_LIST["OWN_LIST"][app_name]["full_url_send_weibo"]
	local interval = info_tb["interval"]
	local level = info_tb["level"]
	local app_key = info_tb["app_key"]
	local secret = info_tb["secret"]
	local fileURL = info_tb["fullURL"]

	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}
	if not wb["senderDirection"] then
		wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	end
	if not wb["senderSpeed"] then
		wb["senderSpeed"]  =  string.format('[%s]',speed and math.ceil(speed) or 0)
	end
	if not wb["senderAltitude"]  and  altitude then
		wb["senderAltitude"]  =  altitude
	end
	if lon and lat and not wb["senderLongitude"] and not wb["senderLatitude"] then
		wb["senderLongitude"] = lon
		wb["senderLatitude"] = lat
	end

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	if wb["multimediaURL"] then
		local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	end
	if not ok then
		only.log("E", "send weibo failed : " .. err)
	end
end
--功	能：对未知字符串的gps信号url进行拼接后发送微博
--参	数：app_name,应用名字; str,预处理生成的字符串
function half_url_unknow_str_send_weibo( app_name, str)
	local body_tb	= supex.get_our_body_table()
	local accountID = body_tb["accountID"]
	local lon       = body_tb["longitude"][1]
	local lat       = body_tb["latitude"][1]
	local speed     = body_tb["speed"][1]
	local direction = body_tb["direction"][1]
	local altitude  = body_tb["altitude"][1]

	local info_tb	= APP_CONFIG_LIST["OWN_LIST"][app_name]["half_url_unknow_str_send_weibo"]
	local interval  = info_tb["interval"]
	local level     = info_tb["level"]
	local app_key 	= info_tb["app_key"]
	local secret 	= info_tb["secret"]

--	local fill_key 	= info_tb["fill_key"]
	local halfURL 	= info_tb["halfURL"]
	
--        local str       = body_tb["private_data"][fill_key]

--	if not str then 
--		return false
--	end	
        local fileURL = string.format(halfURL, str)
	
	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}

	if not wb["senderDirection"] then
		wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	end
	if not wb["senderSpeed"] then
		wb["senderSpeed"]  =  string.format('[%s]',speed and math.ceil(speed) or 0)
	end
	if not wb["senderAltitude"]  and  altitude then
		wb["senderAltitude"]  =  altitude
	end
	if lon and lat and not wb["senderLongitude"] and not wb["senderLatitude"] then
		wb["senderLongitude"] = lon
		wb["senderLatitude"] = lat
	end

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	if not ok then
		only.log("E", "send weibo failed : " .. err)
	end
end
--功	能：对未知字符串的开机信号url进行拼接后发送微博
--参	数：app_name,应用名字; str,预处理生成的字符串
function half_url_power_on_unknow_str_send_weibo( app_name, str)
	local body_tb	= supex.get_our_body_table()
	local accountID = body_tb["accountID"]

	local info_tb	= APP_CONFIG_LIST["OWN_LIST"][app_name]["half_url_power_on_unknow_str_send_weibo"]
	local interval  = info_tb["interval"]
	local level     = info_tb["level"]
	local app_key 	= info_tb["app_key"]
	local secret 	= info_tb["secret"]

	local halfURL 	= info_tb["halfURL"]
	
        local fileURL = string.format(halfURL, str)
	
	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}
--[[
	if not wb["senderDirection"] then
		wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	end
	if not wb["senderSpeed"] then
		wb["senderSpeed"]  =  string.format('[%s]',speed and math.ceil(speed) or 0)
	end
	if not wb["senderAltitude"]  and  altitude then
		wb["senderAltitude"]  =  altitude
	end
	if lon and lat and not wb["senderLongitude"] and not wb["senderLatitude"] then
		wb["senderLongitude"] = lon
		wb["senderLatitude"] = lat
	end
]]--
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	if not ok then
		only.log("E", "send weibo failed : " .. err)
	end
end
--功	能：对自增长的url进行处理后发送微博
--参	数：app_name, 应用名字
function half_url_incr_idx_send_weibo( app_name)
	local body_tb	= supex.get_our_body_table()
	local accountID = body_tb["accountID"]
	local lon       = body_tb["longitude"][1]
	local lat       = body_tb["latitude"][1]
	local speed     = body_tb["speed"][1]
	local direction = body_tb["direction"][1]
	local altitude  = body_tb["altitude"][1]

	local info_tb	= APP_CONFIG_LIST["OWN_LIST"][app_name]["half_url_incr_idx_send_weibo"]
	local app_key	= info_tb["app_key"]
	local secret	= info_tb["secret"]
	local interval	= info_tb["interval"]
	local level	= info_tb["level"]

        local idx_base  = info_tb["idx_base"]
        local idx_max   = info_tb["idx_max"]
        local idx_key   = info_tb["idx_key"]
        local halfURL   = info_tb["halfURL"]

        local idx       = body_tb["private_data"][idx_key]
        
        if not idx then
                return false
        end
        idx = tonumber(idx)
        if idx > idx_max then
                idx = idx_max
        end
        idx = idx_base + idx
        local fileURL = string.format(halfURL, idx)

	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}
	if not wb["senderDirection"] then
		wb["senderDirection"]	= string.format('[%s]', direction and math.ceil(direction) or -1)
	end
	if not wb["senderSpeed"] then
		wb["senderSpeed"]	= string.format('[%s]', speed and math.ceil(speed) or 0)
	end
	if not wb["senderAltitude"] and altitude then
		wb["senderAltitude"]	= altitude
	end
	if lon and lat and not wb["senderLongitude"] and not wb["senderLatitude"] then
		wb["senderLongitude"]	= lon
		wb["senderLatitude"]	= lat
	end
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	if not ok then
		only.log("E", "send weibo failed : " .. err)
	end
end
--功	能：对随机的url进行处理后发送微博
--参	数：app_name, 应用名字
function half_url_random_idx_send_weibo( app_name )
	local body_tb	= supex.get_our_body_table()
	local accountID = body_tb["accountID"]
	
	local info_tb	= APP_CONFIG_LIST["OWN_LIST"][app_name]["half_url_random_idx_send_weibo"]
	local app_key 	= info_tb["app_key"]
	local secret 	= info_tb["secret"]
	local interval 	= info_tb["interval"]
	local level 	= info_tb["level"]
	local idx_base 	= info_tb["idx_base"]
	local idx_max 	= info_tb["idx_max"]
	local halfURL 	= info_tb["halfURL"]
	
	idx = utils.random_among(idx_base + 1, idx_base + idx_max)
	if not idx then 
		return false
	end
	local fileURL = string.format(halfURL, idx)

	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	if not ok then
		only.log("E", "send weibo failed : " .. err)
	end
end
--功	能：文本调用spxapi转url再发送微博
--参	数：app_name,应用名称 ；text,文本信息
function spx_txt_to_url_send_weibo( app_name, text )
        local body_tb   = supex.get_our_body_table()
        local accountID = body_tb["accountID"]

        local info_tb   = APP_CONFIG_LIST["OWN_LIST"][app_name]["spx_txt_to_url_send_weibo"]
        local app_key 	= info_tb["app_key"]
        local secret 	= info_tb["secret"]
        local interval 	= info_tb["interval"]
        local level 	= info_tb["level"]

	local wb = {
		appKey = app_key,
		text = text,
	}
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign
	
	local serv = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
	local req = utils.compose_http_kvps_request(serv, "spx_txt_to_voice", nil, wb, "POST")
	only.log('D', req)
 	local fileurl
	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then
                only.log('D',"fileurl is nil")
                fileurl = nil
	end
	if ok and resp then
	        only.log('D', resp)
                local jo = utils.parse_api_result( resp, "spx_txt_to_voice")
                fileurl =  jo and jo["url"] or nil
	end
        if fileurl then
                only.log('D',"fileurl:%s",fileurl)
        else
                return false
        end


        local wb = {
                multimediaURL = fileurl,
                receiverAccountID = accountID,
                level = level,
                interval = interval,
                senderType = 2,
        }
--[[
        if not wb["senderDirection"] then
                wb["senderDirection"]   = string.format('[%s]', direction and math.ceil(direction) or -1)
        end
        if not wb["senderSpeed"] then
                wb["senderSpeed"]       = string.format('[%s]', speed and math.ceil(speed) or 0)
        end
        if not wb["senderAltitude"] and altitude then
                wb["senderAltitude"]    = altitude
        end
        if lon and lat and not wb["senderLongitude"] and not wb["senderLatitude"] then
                wb["senderLongitude"]   = lon
                wb["senderLatitude"]    = lat
        end
]]--
        local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
        local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
        if not ok then
                only.log("E", "send weibo failed : " .. err)
        end	
end

