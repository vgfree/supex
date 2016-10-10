-- auth: baoxue
-- time: 2014.04.29

local utils 		= require('utils')
local only 		= require('only')
local redis_api 	= require('redis_pool_api')
local mysql_api 	= require('mysql_pool_api')
local supex 		= require('supex')
local luakv_api 	= require('luakv_pool_api')

module('l_config', package.seeall)

function bind()
	return '["powerOn", "accountID", "tokenCode"]'
end

function match()
	return true
end

function drive_is_variable_speed_init()
	local accountID         = supex.get_our_body_table()["accountID"]

	luakv_api.cmd("owner", accountID, "del", accountID .. ":variableSpeed")
end


function drive_is_sharp_turn_init()
	local accountID         = supex.get_our_body_table()["accountID"]

	luakv_api.cmd("owner", accountID, "del", accountID .. ":sharpTurn")
end




function drive_mile_init()
	local accountID         = supex.get_our_body_table()["accountID"]

	luakv_api.cmd("owner", accountID, "del", accountID .. ":driveLastLon")
	luakv_api.cmd("owner", accountID, "del", accountID .. ":driveLastLat")
	luakv_api.cmd("owner", accountID, "del", accountID .. ":driveLastTime")
	luakv_api.cmd("owner", accountID, "del", accountID .. ":driveLastSpeed")
	luakv_api.cmd("owner", accountID, "del", accountID .. ":driveMile")
end




function drive_online_init()
	local accountID         = supex.get_our_body_table()["accountID"]

	luakv_api.cmd("owner", accountID, "del", accountID .. ":goonTime")
	luakv_api.cmd("owner", accountID, "del", accountID .. ":stopTime")
end

function drive_is_over_speed_init()
	local accountID         = supex.get_our_body_table()["accountID"]

	luakv_api.cmd("owner", accountID, "del", accountID .. ":overSpeed")
end

function work()
	drive_is_variable_speed_init()
	drive_is_sharp_turn_init()
	drive_mile_init()
	drive_online_init()
	drive_is_over_speed_init()

	local mirrtalkID         = supex.get_our_body_table()["mirrtalkID"]
	local accountID         = supex.get_our_body_table()["accountID"]
	local timestamp         = supex.get_our_body_table()["timestamp"]
	local tokenCode         = supex.get_our_body_table()["tokenCode"]
	local sql = string.format("INSERT INTO daoke_AccelerationInfo SET mirrtalkID='%s', accountID='%s', powerOnTime=%d, powerOffTime=0, createTime=%d, SAcceleration=0, RAcceleration=0, Sharpturn=0, miles=0, overspeed=0, tired=0, tokenCode='%s', isValid=0",
		mirrtalkID, accountID, timestamp, os.time(), tokenCode )
	print(sql)
	local ok, ret = mysql_api.cmd('app_driving___dataTest','INSERT', sql)
end

