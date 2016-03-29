local only          = require('only')
local weibo         = require('weibo')
local supex         = require('supex')
local WORK_FUNC_LIST = require("WORK_FUNC_LIST")


module('p2p_offsite_remind', package.seeall)

-- 触发逻辑：归属地到异地只触发一次，异地再到异地也只触发一次
function handle()
        -->打印日志
	only.log("I", "p2p_offsite_remind working ... ")
        -->用户订阅权限判断
        if not (weibo.user_control('p2p_offsite_remind')) then
        	return false
        end
        -- 取出异地城市的citycode 
        local citycode = supex.get_our_body_table()["private_data"]["offsiteRemindCityCode"] 
    	-- 1. 市之间的异地城市介绍 2. 电台 3. 特产 4. 特色菜 5. 区县之间的异地提醒
    	-- 市之间的异地有可能触发前四类语音，区县之间只触发第五类提醒
	local remind_table = {
		"CityIntroduction",
		"BroadcastingStation",
		"Specialty",
		"FeaturedDishes",
		"County"
	}
	for i=1,#remind_table do   
		local str = remind_table[i] .. "/" .. citycode
		WORK_FUNC_LIST["half_url_unknow_str_send_weibo"]("p2p_offsite_remind", str)	                             
	end
end
