local utils              = require("utils")
local only           = require("only")
local redis_api      	 = require("redis_pool_api")
local link               = require("link")
local weibo              = require("weibo")

-->新系统上线后，模块设计增加一条系统已升级的通知提示音，一个用户只能听一次

module("p2p_system_update")

function handle()
	-->打印日志
	only.log("D", "p2p_system_update working ... ")
	-->用户订阅权限判断
	if not (weibo.user_control('p2p_system_update')) then
		return false
	end
	-->编写文本
	local text = "版本更新成功，微密2.1.3主要更新：实时路况的播报，版本升级提醒. 祝您使用愉快"
	-->文本转语音	
	WORK_FUNC_LIST["spx_txt_to_url_send_weibo"]( "p2p_system_update", text )
end

