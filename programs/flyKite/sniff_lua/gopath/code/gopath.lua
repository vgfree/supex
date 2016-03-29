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
local raidus_find_poi_func	= require ('raidus_find_poi_func')
local socket			= require('socket')
local get_export_road_by_road	= _G.get_export_road_by_road
local supex			= require('supex')

local TEN_MINUTES	        = 600	--  tsdb数据存储以10分钟为单位
local FRAME_LEN			= 12	--  每一个frame有FRAME_LEN个10分钟的长度
local FRAME_TIME		= FRAME_LEN * TEN_MINUTES
local MAX_TRAVEL_TIME		= 3600 * 48
local AFFIRM_ROAD_GPSCOUNT	= 4
local AFFIRM_NOROADID		= 5

local GPSFrameModule = require('gps_frame')
local GPSFrame = GPSFrameModule.GPSFrame
local RoadRecordModule 		= require('road_record')
local RoadRecord		= RoadRecordModule.RoadRecord
local RoadSetModule 		= require('road_set')
local RoadSet		= RoadSetModule.RoadSet

module('gopath', package.seeall)

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

--功  能: 通过http回调给java端
--参  数: road_set; 计算途径的道路数据
local function post_road_data(road_set)
	local ok, res = pcall(json.encode, road_set)
	if not ok or not res then
		only.log('E', "decode error")
	end

	res_str =string.format('{"ERRORCODE":"0","RESULT":%s}',res)

	local tab = setmetatable({
		jsonRoad = res_str,
		appKey = "3619608887" ,
	}, {__mode = 'k'});

	local body_data =  utils.table_to_kv(tab)
	local app_server = link["OWN_DIED"]["http"]["DataCore/autoGraph/addTravelRoadInfo"]
	local data = utils.post_data("DataCore/autoGraph/addTravelRoadInfoTest", app_server, body_data)

	only.log('D', "post data length:%s", #data)
	supex.http(app_server['host'], app_server['port'], data, #data)
end

--添加record2到record1
--若record1为nil，则返回record2
--若record2不为nil，则合并并返回
local function merge_reocrd(record1, record2)
	if not record1 then
		return record2
	else
		record1:addRecord(record2)
		return record1
	end
end

--功  能: 判断两个road_id是否存在拓扑关系
--参  数: road_id_prev: 前一个roadID
--	  road_id_next: 后一个roadID
--返回值: true : 两个roadID有拓扑关系
--	  false: 两个roadID无拓扑关系
local function is_roads_link(road_id_prev, road_id_next)
	local ok , link_roads = get_export_road_by_road(tonumber(road_id_prev))

	if not ok or #link_roads == 0 then
		return false
	end

	for i,v in ipairs(link_roads) do
		if tonumber(v) == tonumber(road_id_next) then
			return true
		end
	end

	return false
end

--功  能: 根据道路连接关系判断途径，并添加到road_set中
--参  数: road_set: 存放途径数据集合
--	  last_record2:向前追溯第二个road
--	  last_record1:向前追溯第一个road
--	  road_record: 当前road

local function review_road_id(road_set, last_record2, last_record1, road_record)
	if not last_record2 then
		road_set:addRoadRecord(last_record1)
		return
	end

	local link_last2_last1 = is_roads_link(last_record2['d_roadID'], last_record1['d_roadID'])
	local link_last1_cur = is_roads_link(last_record1['d_roadID'], road_record['d_roadID'])
	local link_last2_cur = is_roads_link(last_record2['d_roadID'], road_record['d_roadID'])

--	only.log('D', string.format("link_last2_last1:%s, link_last1_cur:%s, link_last2_cur:%s", link_last2_last1, link_last1_cur, link_last2_cur))
--	only.log('D', string.format("last2:%s,%s, last1:%s,%s,cur:%s,%s", 
--		last_record2['roadID'], last_record2['gpsCount'], 
--		last_record1['roadID'], last_record1['gpsCount'],
--		road_record['roadID'], road_record['gpsCount']))

	if link_last2_last1 and link_last1_cur then	--顺序相连 last2--->last1--->cur
		road_set:addRoadRecord(last_record1)
	elseif not link_last2_last1 and not link_last1_cur then	--不是顺序相连
		if link_last2_cur then	--首尾相连，中间舍弃 last2--->cur
			road_set:remove(last_record2['roadID'])	-- 将last1与last2合并，重新添加
			last_record2:addRecord(last_record1)
			road_set:addRoadRecord(last_record2)
			return
		elseif last_record1['gpsCount'] >= AFFIRM_ROAD_GPSCOUNT then  --全部不相连，根据gps点数判断
			road_set:addRoadRecord(last_record1)
		end
	elseif link_last2_last1 then	--  last2--->last1...cur
		road_set:addRoadRecord(last_record1)
	elseif link_last1_cur then	
		if link_last2_cur then	--Y形状路或丁子路, 上两条路取gps点多的
			if last_record2['gpsCount'] < last_record1['gpsCount'] then
				road_set:remove(last_record2['roadID'])	-- last2已经添加 需要移除
				road_set:addRoadRecord(last_record1)
			else
				road_set:remove(last_record2['roadID'])	-- 将last1与last2合并，重新添加
				last_record2:addRecord(last_record1)
				road_set:addRoadRecord(last_record2)
				return
			end	
		elseif last_record2['gpsCount'] < AFFIRM_ROAD_GPSCOUNT then
			road_set:remove(last_record2['roadID'])
		end
		if last_record1['gpsCount'] >= AFFIRM_ROAD_GPSCOUNT then 
			road_set:addRoadRecord(last_record1)
		end
	else
		road_set:addRoadRecord(last_record1)
	end

end

--功  能: 组装frame数据
--参  数: IMEI: IMEI
--	  tokenCode: tokenCode
--	  accountID: accountID
--	  frame_array: 途径计算获得frame数组
--返回值: frame个数
function add_travel_info(IMEI, tokenCode, accountID, frame_array)
	local road_set = RoadSet:new(IMEI, tokenCode, accountID)
	local last_record1	--向前追溯第一个roadID
	local last_record2	--向前追溯第二个roadID
	local no_roadid_record
	local road_record

	for i=1,#frame_array do --遍历frame数组
		local gps_frame = frame_array[i]
		for road_record_idx,road_record in ipairs(gps_frame['recordSet']) do
			repeat	-- break instead of continue

				if road_record['roadID'] == '0' then
					no_roadid_record = merge_reocrd(no_roadid_record, road_record)
					break --continue
				end	

				-- 对相邻且roadID相同的road_record进行聚合
				if not last_record1 then
					last_record1 = road_record
					if not no_roadid_record then
						break --continue
					end

					-- 刚开始就出现无roadID情况处理
					if no_roadid_record['gpsCount'] >= AFFIRM_NOROADID then
						road_set:addRoadRecord(no_roadid_record)
					else
						last_record1:addRecord(no_roadid_record)
					end
					no_roadid_record = nil
					break	--continue
				elseif road_record['roadID'] == last_record1['roadID'] and (not no_roadid_record) then	
					last_record1 = merge_reocrd(last_record1, road_record)	--有roadID的合并到 last_record1中
					break	--continue
				elseif road_record['roadID'] == '0' then			--无roadID的合并到 no_roadid_record中
					no_roadid_record = merge_reocrd(no_roadid_record, road_record)
					break	--continue
				end

				if road_record['roadID'] == last_record1['roadID'] then
					if no_roadid_record['gpsCount'] < AFFIRM_NOROADID then	--夹在两个相同roadID之间，且gps点小于5，则与前后合并
						last_record1:addRecord(road_record)
						last_record1:addRecord(no_roadid_record)
						no_roadid_record = nil

						break	--continue
					else	--夹在两个相同roadID之间， 且gps点>= 5 ，则认为确定是一个未知道路
						road_set:addRoadRecord(last_record1)
						road_set:addRoadRecord(no_roadid_record)
						no_roadid_record = nil

						goto AFFIRM
					end
				else --与上一个roadID不相同
					if not no_roadid_record then	--根据道路连接关系判断
						goto REVIEW
					elseif no_roadid_record['gpsCount'] < AFFIRM_NOROADID then	--根据道路连接关系判断
						last_record1:addRecord(no_roadid_record)
						no_roadid_record = nil
						goto REVIEW
					else	--连续5个点以上没有roadID，则确定是一条未知道路
						road_set:addRoadRecord(last_record1)
						road_set:addRoadRecord(no_roadid_record)
						no_roadid_record = nil
						goto AFFIRM
					end
				end

				::REVIEW::	--根据道路连接关系判断途径
				review_road_id(road_set, last_record2, last_record1, road_record)

				::AFFIRM::	--判定一条道路更新 last record
				last_record2 = last_record1
				last_record1 = road_record
			until true
		end
	end

	--若没有gps数据，返回一条记录
	if #road_set['roadIDSet'] == 0 then
		local road_info = {}

		--添加空的road_info
		road_set:addRoadInfo(road_info)
	end

	return road_set
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

		local gps_frame = GPSFrame:new(IMEI, tokenCode, frame_start_time, frame_end_time)
		frame_array[frame_idx] = gps_frame
		only.log('D', string.format('frame idx:%s, start:%s, end:%s', frame_idx, frame_start_time, frame_end_time))
		gps_frame:process()

		frame_start_time = frame_end_time + 1
		frame_idx = frame_idx + 1
		memory_analyse("frame repeat")
		collectgarbage("collect")
	until frame_end_time >= ed_time

	local t2 = socket.gettime()
	only.log('D', "gopath handle process frame time:%s", t2 - t1)
	--组装frame并发送数据
	local road_set = add_travel_info(IMEI, tokenCode, accountID, frame_array)
	--road_set:save("/data/travel_info")
	local t3 = socket.gettime()
	only.log('D', "gopath handle addTravelRoadInfo time:%s", t3 - t2)
	frame_array = nil
	memory_analyse("after addTravelRoadInfo")
	collectgarbage("collect")
	local t4 = socket.gettime()
	only.log('D', "gopath handle collectgarbage time:%s", t4 - t3)
	post_road_data(road_set['roadIDSet'])
	local t5 = socket.gettime()
	only.log('D', "gopath handle post_road_data time:%s", t5 - t4)
	road_set = nil
	memory_analyse("after post_road_data")
	collectgarbage("collect")
	local t_end = socket.gettime()
	only.log('I', "IMEI:%s, tokenCode:%s, end cost:%s", IMEI, tokenCode, t_end - t1)
	memory_analyse("the end")
end
