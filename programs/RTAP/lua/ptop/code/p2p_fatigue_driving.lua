local only		= require("only")
local weibo		= require("weibo")
local WORK_FUNC_LIST		= require("WORK_FUNC_LIST")


module("p2p_fatigue_driving", package.seeall)

function handle()
	only.log("I", "p2p_fatigue_driving working ... ")

	if not (weibo.user_control('p2p_fatigue_driving')) then
		return false 
	end
	WORK_FUNC_LIST["half_url_incr_idx_send_weibo"]( "p2p_fatigue_driving" )
end

