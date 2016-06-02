local only = require('only')
local supex = require('supex')
local cjson = require('cjson')
local zmq_api = require('zmq_api')

module('settings', package.seeall)


local function power_on_settings()
	local tag = cjson.encode(
	{
		["powerOn"] = {
			[1] = supex.get_our_body_table()["IMEI"]
		}
	}
	)
	local result_tab = {
		[1] = tag,
		[2] = supex.get_our_body_data()
	}
	local ok = zmq_api.cmd("damR", "send_table", result_tab)
end



local function collect_settings()
	local body_tb	= supex.get_our_body_table()

	local imei =  body_tb["IMEI"]
	local longitude =  body_tb["longitude"][1] or 0
	local latitude  =  body_tb["latitude"][1]  or 0
	local grid = string.format('%d&%d', tonumber(longitude)*100, tonumber(latitude)*100)

	local tag = cjson.encode(
	{
		["collect"] = {
			[1] = imei,
			[2] = grid,
		}
	}
	)
	local result_tab = {
		[1] = tag,
		[2] = supex.get_our_body_data()
	}
	local ok = zmq_api.cmd("damR", "send_table", result_tab)
end

local function power_off_settings()
	local tag = cjson.encode(
	{
		["powerOff"] = {
			[1] = supex.get_our_body_table()["IMEI"]
		}
	}
	)
	local result_tab = {
		[1] = tag,
		[2] = supex.get_our_body_data()
	}
	local ok = zmq_api.cmd("damR", "send_table", result_tab)
end










function handle()
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
end

