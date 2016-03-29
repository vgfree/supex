local only = require('only')
local weibo = require('weibo')
local utils = require('utils')
local supex = require('supex')
local WORK_FUNC_LIST = require("WORK_FUNC_LIST")

module('p2p_over_speed', package.seeall)

function handle()
	-->打印日志
        only.log("I", "p2p_over_speed working ... ")
	-->用户订阅权限判断
        if not (weibo.user_control('p2p_over_speed')) then
                return false
        end
 	-->获取私有数据
	local private_data = supex.get_our_body_table()["private_data"]
        local overSpeedCarry = private_data["overSpeedCarry"]
        if not overSpeedCarry then
                return false
	end
	-->获取标识和限速
	local data_tb = utils.str_split(overSpeedCarry,":")
	local flag = data_tb[1]
	local limit_speed = data_tb[2]
--      local  flag, limit_speed =  string.match(overSpeedCarry, "([^:]+):([%w]+)")
        if not flag or not limit_speed then
                only.log('E', "[flag == nil][limit_speed == nil]")
                return false
	end
	-->文本信息：标识为0，下标为限速，否则下标为1到9的随机值
        local text;
        local txt = {
                [1] = "当前道路限速%s，您已超速",
        }
        local txt_tmp = {
                [1] = "你超速了，还是慢点开吧。",
                [2] = "你一直超速，你家人知道不。",
                [3] = "再不慢点，我就要插播广告了。",
                [4] = "开车不超速，你好，我好，大家好。",
        }
        local index = 0
        if tonumber(flag) == 0 then
                text = string.format(txt[1], limit_speed)
                index = tonumber(limit_speed)
        else
                index = tonumber(os.time() % 9 + 1)
                text = txt_tmp[index]
        end
	-->下标对应的序号表
        local tb = {
                [60]    = 20060,
                [70]    = 20070,
                [80]    = 20080,
                [90]    = 20090,
                [100]   = 20100,
                [110]   = 20110,
                [120]   = 20120,
                [1]     = 20001,
                [2]     = 20002,
                [3]     = 20003,
                [4]     = 20004,
                [5]     = 20005,
                [6]     = 20006,
                [7]     = 20007,
                [8]     = 20008,
                [9]     = 20009,
        }

	-->表中有这条数据
 	if tb[index] then
        	WORK_FUNC_LIST["half_url_random_idx_send_weibo"]( "p2p_over_speed" )
	-->否则，文本转语音
	else
        	WORK_FUNC_LIST["spx_txt_to_url_send_weibo"]( "p2p_over_speed", text )
	end
end
