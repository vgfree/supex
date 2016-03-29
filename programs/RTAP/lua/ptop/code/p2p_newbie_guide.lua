local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')
local weibo = require('weibo')
module('p2p_newbie_guide', package.seeall)

local function newbie_guide()
	local accountID = supex.get_our_body_table()["accountID"]
	local send_list = {
		{"http://127.0.0.1/productList/newbieGuide/1.amr", 15},
		{"http://127.0.0.1/productList/newbieGuide/2.amr", 16},
		{"http://127.0.0.1/productList/newbieGuide/3.amr", 17},
		{"http://127.0.0.1/productList/newbieGuide/4.amr", 18},
		{"http://127.0.0.1/productList/newbieGuide/5.amr", 19},
		{"http://127.0.0.1/productList/newbieGuide/6.amr", 20},
		{"http://127.0.0.1/productList/newbieGuide/7.amr", 21},
		--{"http://127.0.0.1/productList/newbieGuide/8.amr", 22},
	}
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	for _,v in ipairs( send_list ) do
		local wb = {
			multimediaURL = v[1],
			receiverAccountID = accountID,
			level = v[2],
			interval = 60*10,
			senderType = 2,
		}
		local ok,err = weibo.send_weibo( server, "personal", wb, "2875909808", "2A59FFD121CCC2ECEBB2F1272B5A521E6A938635" )
		if not ok then
			only.log("E", "send weibo failed : " .. err)
		end
	end
end

function handle()
	--> 新手教程	
	local accountID = supex.get_our_body_table()["accountID"]
	local model = supex.get_our_body_table()["model"]
	if model == "SG900" or model == "TG900" then
		if not weibo.check_driview_subscribed_msg(accountID, weibo.DRI_APP_NEWBIE_GUIDE) then
			only.log('D','新手教程，被客户禁止!')
			return false
		else
			newbie_guide()
		end
	end
end
