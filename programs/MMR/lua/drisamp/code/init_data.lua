local redis_api         = require('redis_pool_api')
local pool          = require('pool')
local only              = require('only')
local socket            = require('socket')
local supex             = require('supex')
local scan              = require('scan')
local cfg               = require('cfg')
local judge             = require('judge')
local user_freq_cntl	= require("user_freq_cntl")
local luakv_api 	= require('luakv_pool_api')
local _fun_point_match_road = require ('_fun_point_match_road')
local road_traffic_handle = require('road_traffic_handle')

module('init_data', package.seeall)



local function get_road_info(lon, lat, dir, accountID)
	local ok, ret = _fun_point_match_road.entry(dir, lon, lat, accountID)
	if ok and ret then
		local roadID = ret['roadID']
		local lineID = ret['lineID']
		local kv_tab
		local key = roadID .. ":roadInfo" 
		local ok, result = redis_api.cmd("mapRoadInfo", accountID, "hgetall", key)
		if ok and result then
			kv_tab = result
			kv_tab['roadID'] = roadID
			kv_tab['lineID'] = lineID
			only.log("D", string.format("[result:%s]", scan.dump(kv_tab)))
		else
			only.log('E', "get mapRoadInfo failed");
		end
		only.log('D',"[[Function:get_road_info]] over")
		return kv_tab
	else
		only.log('I', "Point Match Road ERROR")
		return nil
	end
end

local function power_on_settings()
	local accountID = supex.get_our_body_table()["accountID"]
	--频率控制中开机记录数据初始化 
	user_freq_cntl.init( )

	--is_over_speed 模块初始化数据
	judge.is_over_speed_init( accountID )
	
	-->>road traffic 数据初始化
	road_traffic_handle.init()
	
end

local function collect_settings()
	local body_tb	= supex.get_our_body_table()

	local accountID =  body_tb["accountID"]
	local lon       =  body_tb["longitude"][1] or 0
	local lat       =  body_tb["latitude"][1]  or 0
	local speed     =  body_tb["speed"][1]     or 0
	local dir       =  body_tb["direction"][1] or '-1'
	local alt       =  body_tb["altitude"][1]  or 0
	local gpstime   =  body_tb["GPSTime"][1]   or 0


	pool["point_match_road_result"] = get_road_info(lon, lat, dir, accountID)
	--only.log('D', string.format("[point_match_road_result:%s]", scan.dump(pool["point_match_road_result"])))
end

local function power_off_settings()
end


function handle ( skip )
	if skip then
		return
	end
	local t1 = socket.gettime()
	if supex.get_our_body_table()["powerOn"] then
		local ok,result = pcall( power_on_settings )
		if not ok then
			only.log("E", result)
		end
	end
	if supex.get_our_body_table()["collect"] then
		local ok,result = pcall( collect_settings )
		if not ok then
			only.log("E", result)
		end
	end
	if supex.get_our_body_table()["powerOff"] then
		local ok,result = pcall( power_off_settings )
		if not ok then
			only.log("E", result)
		end
	end
	local t2 = socket.gettime()
	if(cfg["OWN_INFO"]["SYSLOGLV"]) then
		only.log('S', string.format("MODULE : init_data ===>  total [%f]", t2 - t1))
	end
end
