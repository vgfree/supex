-- author       : 耿玄玄
-- date         : 2015-8-20
-- Feature No   : 2163

local only			= require ('only')
local redis_api			= require ('redis_pool_api')
local http_api			= require ('http_short_api')
local utils			= require ('utils')
local scan			= require ('scan')
local json			= require ('cjson')
local link			= require ("link")
local socket			= require('socket')
local supex			= require('supex')

local TEN_MINUTES	        = 600	--  tsdb数据存储以10分钟为单位
local FRAME_LEN			= 12	--  每一个frame有FRAME_LEN个10分钟的长度
local FRAME_TIME		= FRAME_LEN * TEN_MINUTES
local MAX_TRAVEL_TIME		= 3600 * 48
local AFFIRM_ROAD_GPSCOUNT	= 4
local AFFIRM_NOROADID		= 5

module('kkb', package.seeall)

local function memory_analyse( tag )
	local msize = collectgarbage("count")
	local ldata = string.format("%s : memory use \t[%d]KB \t[%d]M", tag or "", msize, msize/1024)
	print(ldata)
	only.log('I', ldata)
end

--功  能: 检查参数 imei, tokenCode, startTime, endTime
--参  数: args
--返回值: true:参数正常;false:参数错误 
local function check_args(args)
	if not args['imei'] or not args['tokenCode'] or not args['startTime'] or not args['endTime'] then
		only.log('E', string.format("args ERROR(IMEI:%s, tokenCode:%s, startTime:%s, endTime:%s)", 
		args['imei'] or "nil", args['tokenCode'] or "nil", args['startTime'] or "nil", args['endTime'] or "nil"))
		return false
	end

	return true
end

function handle()
	local args = supex.get_our_body_table()

	if not check_args(args) then
		return
	end

	local t1 = socket.gettime()
	local accountID = args['accountID']
	local IMEI      = args['imei']
	local tokenCode = args['tokenCode']
	local startTime = tonumber(args['startTime'])
	local endTime   = tonumber(args['endTime'])

	if (startTime > endTime) or (endTime > (startTime + MAX_TRAVEL_TIME + 3600)) then
		only.log('E', string.format("ERROR todo task IMEI %s tokenCode %s from %s to %s", IMEI, tokenCode, startTime, endTime))
		return 
	end

	if not accountID then --若没有绑定accountID,则使用IMEI代替
		local ok, accountID = redis_api.cmd('private', IMEI, 'get', string.format('%s:accountID', IMEI))
		if not ok or not accountID then
			only.log('I', "get accountID failed!")
			accountID = IMEI
		end
	end

	--由于数据包延时，tsdb数据存储time key可能比collect time晚, 前后拓宽20s
	local st_time = (startTime % TEN_MINUTES > 20) and startTime or (startTime - 20)
	local ed_time = (endTime % TEN_MINUTES <= (TEN_MINUTES - 20)) and endTime or (endTime + 20)

	only.log('D', string.format("extend start %s to %s;end %s to %s", startTime, st_time, endTime, ed_time))

	local frame_start_time = st_time
	local frame_array = {}
	local frame_idx = 1

	--循环计算每个frame途径
	--以两个小时时长作为一个frame单位进行计算
	repeat
		local frame_end_time = (math.floor(frame_start_time / FRAME_TIME)  + 1) * (FRAME_TIME) - 1
		if frame_end_time >= ed_time then	--最后一个frame
			frame_end_time = ed_time
		end

		frame_start_time = frame_end_time + 1
		frame_idx = frame_idx + 1
		memory_analyse("frame repeat")
		collectgarbage("collect")
	until frame_end_time >= ed_time

	memory_analyse("the end")
end
