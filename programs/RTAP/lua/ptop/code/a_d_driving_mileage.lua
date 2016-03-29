local utils		= require("utils")
local only		= require("only")
local redis_api		= require("redis_pool_api")
local APP_CFG		= require("cfg")
local judge		= require("judge")
local APP_CONFIG_LIST		= require("CONFIG_LIST")
local BOOL_FUNC_LIST		= require("BOOL_FUNC_LIST")
local WORK_FUNC_LIST		= require("WORK_FUNC_LIST")


module("a_d_driving_mileage", package.seeall)

function bind()
	return '{}'
end

function match()
	return true
end

function work()
	only.log("I", "a_d_driving_mileage working ... ")
	WORK_FUNC_LIST["half_url_incr_idx_send_weibo"]( "a_d_driving_mileage" )
end

