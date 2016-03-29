--
-- 版权声明: 暂无
-- 文件名称: a_app_solarcalendar.lua
-- 创建者  : 
-- 创建日期: 
-- 文件描述: 本文件主要功能是给用户发送青年小阳历。
-- 历史记录: 无
--

local only           = require("only")
local weibo              = require("weibo")
local WORK_FUNC_LIST = require("WORK_FUNC_LIST")

module("p2p_solarcalendar", package.seeall)


function handle()
        -->打印日志
	only.log("I", "a_app_solarcalendar working ... ")
        -->用户订阅权限判断
	if not weibo.check_subscribed_msg(accountID, weibo["DRI_APP_SOLARCALENDER"]) then
		only.log('D','青年小阳历，被客户禁止!')
		return false
	end
	--获取当天日期
	local cur_date = os.date("%Y%m%d")

	WORK_FUNC_LIST["half_url_power_on_unknow_str_send_weibo"]("p2p_solarcalendar", cur_date)
end

