local only = require('only')
local weibo = require('weibo')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local utils = require('utils')
local WORK_FUNC_LIST = require('WORK_FUNC_LIST')

module('p2p_continuous_driving_mileage_remind', package.seeall)

function handle()
	-->打印日志
        only.log("I", "p2p_continuous_driving_mileage working ... ")
       	-->用户订阅权限判断
       	if not (weibo.user_control('p2p_continuous_driving_mileage')) then
       		return false
       	end
	-->获取私有数据
        local carry_data = supex.get_our_body_table()["private_data"]["continuousDrivingCarryData"]
        only.log('D', "[carry data  = %s]", carry_data)
	-->从私有数据中获取所需信息
	local data_tb 		= utils.str_split(carry_data,":")
	local data_type 	= data_tb[1]
	local useless_argument  = data_tb[2]
	local actual_mileage 	= data_tb[3]
	local max_speed 	= tonumber(data_tb[4]) or 0
	local avg_speed 	= tonumber(data_tb[5]) or 0
	local stop_time 	= tonumber(data_tb[6]) or 0
        if not data_type or not actual_mileage or not max_speed or not avg_speed or not stop_time then
                only.log('E', "[data_type == nil  == nil or actual_mileage == nil or max_speed ==nil or avg_speed ==nil or stop_time == nil]")
                return false
        end
        only.log('D',"[data_type:%s][actual_mileage:%s][max_speed:%s][avg_speed:%s][stop_time:%s]", data_type,actual_mileage,max_speed,avg_speed,stop_time)
	-->获取文本信息
        local text;
        local stop_time_hour = math.modf(stop_time / 3600)
        local stop_time_minute = math.modf((stop_time - stop_time_hour * 3600) / 60)
        local stop_time_second = stop_time % 3600 % 60
        local txt = { 
        	[1] = " 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s小时%s分钟%s秒",
                [2] = " 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s小时%s分钟%s秒",
                [3] = " 你已经驾驶了%s公里, 最高时速%s, 平均时速%s，停止时长%s小时%s分钟%s秒，开这么久休息一下吧",
        }

        if max_speed < 125 and actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
        	text = string.format(txt[1], nickname, actual_mileage, max_speed, avg_speed, stop_time_hour, stop_time_minute, stop_time_second)
       	elseif max_speed > 125 and actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
               	text = string.format(txt[2], nickname, actual_mileage, max_speed, avg_speed, stop_time_hour, stop_time_minute, stop_time_second)
       	elseif actual_mileage == 100 or actual_mileage == 200 or actual_mileage == 300 then
               	text = string.format(txt[3], nickname, actual_mileage, max_speed, avg_speed, stop_time_hour, stop_time_minute, stop_time_second)
       	else
               	return false
       	end
	-->不播报 0小时0分钟 和 0小时
	text = string.gsub(text, "0小时0分钟", "")
	text = string.gsub(text, "0小时", "")	
   	only.log('D', "text:" .. text)
	-->文本转语音
        WORK_FUNC_LIST["spx_txt_to_url_send_weibo"]( "p2p_continuous_driving_mileage_remind", text )

end
                                                      	
