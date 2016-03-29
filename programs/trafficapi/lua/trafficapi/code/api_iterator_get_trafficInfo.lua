--Author	: dujun
--Date		: 2015-12-08
--Function	: get front speed info by given longitude, latitude and direction

local luakv		= require('luakv_pool_api')
local utils		= require('utils')
local only		= require('only')
local scan		= require('scan')
local msg		= require('api_msg')
local safe		= require('safe')
local func_get_traffic	= require('func_get_traffic_msg')
local socket		= require('socket')
local supex		= require('supex')
local func_text		= require('func_get_trafficText')

module("api_iterator_get_trafficInfo", package.seeall)

local count = 20
local stats = 'failure'
local bool = 'false'

local function check_parameter(res)
	if not res['longitude'] or not tonumber(res['longitude']) then
		func_get_traffic.resp_msg(stats, bool, msg['MSG_ERROR_REQ_ARG'], 'longitude')
		return false
	end
	
	if not res['latitude'] or not tonumber(res['latitude']) then
		func_get_traffic.resp_msg(stats, bool, msg['MSG_ERROR_REQ_ARG'], 'latitude')
		return false
	end
	
	if not res['direction'] or not tonumber(res['direction']) then
		func_get_traffic.resp_msg(stats, bool, msg['MSG_ERROR_REQ_ARG'], 'direction')
		return false
	end

	if not res['HICount'] or not tonumber(res['HICount']) then
		func_get_traffic.resp_msg(stats, bool, msg['MSG_ERROR_REQ_ARG'], 'HICount')
		return false
	end
	
	if not res['speed'] or not tonumber(res['speed']) then
		func_get_traffic.resp_msg(stats, bool, msg['MSG_ERROR_REQ_ARG'], 'speed')
		return false
	end
	
	if not res['uid'] then --or string.len(res['uid']) ~= 35 
		func_get_traffic.resp_msg(stats, bool, msg['MSG_ERROR_REQ_ARG'], 'uid')
		return false
	end
	
	if not res['resultType'] or not tonumber(res['resultType']) or (tonumber(res['resultType'])~=1 and tonumber(res['resultType'])~=2 and tonumber(res['resultType'])~=3) then
		func_get_traffic.resp_msg(stats, bool, msg['MSG_ERROR_REQ_ARG'], 'resultType')
		return false
	end
	return true
end

local function operate_luakv(key)

	local ok, tab = luakv.cmd('luakv', '', 'get', key)
	local traffic_tab
	if not ok or not tab then
		only.log('W', string.format("Failed to get speedInfo by : key[%s]", key))
		traffic_tab = {-1, -1, -1, -1}
	else
		traffic_tab = func_get_traffic.str_split(tab, ':')
		only.log('D', 'traffic_tab = ' .. scan.dump(traffic_tab))
	end
	if not traffic_tab or #traffic_tab ~= 4 then
		traffic_tab = {-1, -1, -1, -1}
	end
	
	return traffic_tab
end

local function get_iterator_info(RRID,SGID,CNT,interval)

	local pushptr
	local totalLength, totalTime = 0, 0
	local segmentInfo = {}
	pushptr = iterator_init(RRID,SGID,CNT)

	local num = 0
	while num < count do
		local result = iterator_next(pushptr)
		if not result then
			break
		end

		local key = string.format("%d%03d:trafficInfo",result[6],result[7])
		local traffic_tab = operate_luakv(key)

		local isHistory, trafficType, pass_time
		if tonumber(traffic_tab[3]) and (tonumber(traffic_tab[3]) + tonumber(interval) >= os.time()) then
			isHistory = 0
		else
			isHistory = 1
			traffic_tab = {-1, -1, -1, -1}
		end

		trafficType = func_get_traffic.get_trafficType(isHistory, traffic_tab[2], result[8], traffic_tab[1])
		if tonumber(trafficType) ~= 4 then
			stats = 'success_1'
		end
		pass_time = func_get_traffic.get_pass_time(traffic_tab[2], result[8], result[1])
		local data = {
			MS = tonumber(traffic_tab[1]) or -1,
			AS = tonumber(traffic_tab[2]) or -1,
			CT = tonumber(traffic_tab[3]) or -1,
			PT = tonumber(pass_time),
			RL = tonumber(result[1]) or -1,
			IN = result[5],
			IR = tonumber(result[2]) or -1,
			IL = tonumber(string.format("%.6f", result[3])),
			IB = tonumber(string.format("%.6f", result[4])),
			HT = 0,
			TT = tonumber(trafficType) or 4
		}

		if data['IN'] and data['IN'] ~= '' then
			table.insert(segmentInfo, data)
			if data['RL'] ~= -1 then
				totalLength = totalLength + data['RL']
			end

			if data['PT'] then
				totalTime = totalTime + data['PT']
			end
		end
		num = num + 1
	end

	iterator_destory(pushptr)
	local trafficInfo = {
		roadSegment = segmentInfo,
		trafficText = {},
		totalPassTime = totalTime,
		totalLength = totalLength
	}

	return trafficInfo
end

local function get_table(res, tab)

	local segmentID = tab['segmentID']
	local roadRootID = tab['roadRootID']
	local CNT = tonumber(res['HICount'])
	if tonumber(CNT) > 5 then CNT = 5 end
	local currentInfo = func_get_traffic.get_currentInfo(res, roadRootID, segmentID)
	if not currentInfo then
		return false
	end

	local trafficInfo = get_iterator_info(roadRootID, segmentID, CNT, res['interval'])

	local stat = {
		current = currentInfo,
		traffic = trafficInfo
	}

	only.log('D', 'trafficInfo = ' .. scan.dump(stat))
	return stat
end

function handle()
	local args = supex.get_our_body_table()
	if not args or not next(args) then
		return func_get_traffic.resp_msg(stats, bool, msg['SYSTEM_ERROR'])
	end
	
	if not check_parameter(args) then
		return false
	end
	args['interval'] = args['interval'] or 600
	local resultType = tonumber(args['resultType']) or 1

	local segmentID_tab = func_get_traffic.get_segmentID(args)
	if not segmentID_tab then
		return false
	end
	local speedInfo = get_table(args, segmentID_tab)
	if not speedInfo then
		return false
	end
	local result = func_text.get_result(speedInfo, resultType)

	local status, json_data = utils.json_encode(result)
	if not status then
		only.log('E', "Failed to json_encode !")
		stats = 'failure'
		func_get_traffic.resp_msg(stats, bool, msg['SYSTEM_ERROR'])
		return false
	end
	if stats ~= 'success_1' then
		stats = 'success_0'
	end
	bool = 'true'
	func_get_traffic.resp_msg(stats, bool, msg['MSG_SUCCESS_WITH_RESULT'], json_data)

	return true
end


