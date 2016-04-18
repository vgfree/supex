-- author       : 刘勇跃 
-- date         : 2016-04-10

local only			= require ('only')
local redis_api			= require ('redis_pool_api')
local mysql_api                 = require ('mysql_pool_api')
local http_api			= require ('http_short_api')
local utils			= require ('utils')
local scan			= require ('scan')
local utils                     = require ('utils')    
local json			= require ('cjson')
local link			= require ("link")
local socket			= require ('socket')
local gosay                     = require ('gosay')
local msg                       = require ('msg')
local get_export_road_by_road	= _G.get_export_road_by_road
local supex			= require ('supex')

local TEN_MINUTES	        = 600	--  tsdb数据存储以10分钟为单位
local FRAME_LEN			= 12	--  每一个frame有FRAME_LEN个10分钟的长度
local FRAME_TIME		= FRAME_LEN * TEN_MINUTES
local MAX_TRAVEL_TIME		= 3600 * 48
local AFFIRM_ROAD_GPSCOUNT	= 4
local AFFIRM_NOROADID		= 5

local GPSFrameModule            = require('gps_frame')
local GPSFrame                  = GPSFrameModule.GPSFrame
local RoadRecordModule 		= require('road_record')
local RoadRecord		= RoadRecordModule.RoadRecord

module('kkb', package.seeall)

local function memory_analyse( tag )
	local msize = collectgarbage("count")
	local ldata = string.format("%s : memory use \t[%d]KB \t[%d]M", tag or "", msize, msize/1024)
	print(ldata)
	only.log('I', ldata)
end

--功  能: 检查参数 imei, tritime
--参  数: args
--返回值: true:参数正常;false:参数错误 
local function check_args(args)
	if not args['imei'] or not args['tritime'] then
		only.log('E', string.format("args ERROR(IMEI:%s, triTime:%s)", 
		args['imei'] or "nil", args['tritime'] or "nil"))
		return false
	end
	return true
end

--功  能: 根据触发时间计算获取历史数据的前后节点
--参  数: 触发时间戳
--返回值: 返回历史数据的前后节点时间戳
local function calTime(triTime)
	local endDay    = os.date('*t',triTime - (1 * 24 * 3600))
	local endTime   = os.time({
				year=endDay.year, 
				month=endDay.month, 
				day=endDay.day, 
				hour='23', 
				min='59', 
				sec='59'
			})                          -- 结束时间为1天前的23:59:59 
	local startDay  = os.date('*t',triTime - (30 * 24 * 3600))
	local startTime = os.time({
				year=startDay.year, 
				month=startDay.month, 
				day=startDay.day, 
				hour='00', 
				min='00', 
				sec='00'
			})                          -- 开始时间为90天前的00:00:00
	endDay, startDay = nil, nil
	return startTime, endTime
end

--功  能: 加密imei
--参  数: imei
--返回值: imei_en
local function encryption(imei)
	local imei_str = tostring(imei)
	--18位
	local imei_0   = string.sub(imei_str,1,5)..'0'
	local imei_2   = string.sub(imei_str,6,10)..'2'
	local imei_5   = string.sub(imei_str,11,15)..'5'
	--转置
	local imei_rev_0 = string.reverse(imei_0)
	local imei_rev_2 = string.reverse(imei_2)
	local imei_rev_5 = string.reverse(imei_5)
	--组装
	local imei_en = tonumber(imei_rev_5..imei_rev_2..imei_rev_0)
	imei_0,imei_2,imei_5,imei_rev_0,imei_rev_2,imei_rev_5 = nil,nil,nil,nil,nil,nil
	
	return imei_en
end

--功  能: 初始化文件，写入列名
--参  数: imei
--返回值: true: 初始化成功; false: 初始化失败
--	和文件名
local function initFile(imei)
	--加密imei
	local imei_en = encryption(imei)
	--初始化
	local p = string.format("/home/liu/user_%d.txt", tonumber(imei_en))
	local f = assert(io.open(p,'w'))
	local init = 'CollectTime,Longitude,Latitude,Altitude,Direction,Speed,Mileage,A,Uid,Name,RoadType,OverSpeed,OverPercent,countyName,Pname,cityName,Weather,Temperatur'
	f:write(init,'\n')
	f:close()
	local bool,_ = pcall(io.open(p,'r'))
	imei_en,f,init = nil,nil,nil,nil

	return bool and false or true, p
end


function handle()
	local args = supex.get_our_body_table()

	if not check_args(args) then
		return
	end

	local t1 = socket.gettime()

	local IMEI      = args['imei']
	local triTime   = tonumber(args['tritime'])      --触发时间
	--计算前后节点
	local fromTime, toTime = calTime(triTime)

	only.log('E', string.format("Taskinfo IMEI %s tokenCode %s from %s to %s", 
		IMEI, tokenCode, fromTime, toTime))
	--获取tokenCode
	local select_tokenCode_sql = string.format(
		"SELECT tokenCode, startTime, endTime FROM 用户驾驶里程数据 WHERE imei = %s AND (startTime > %d AND endTime < %d);",
		IMEI, fromTime, toTime
	)
	local ok,ret = mysql_api.cmd('kkb_sql','SELECT',select_tokenCode_sql)
	if not ok or not ret then
		only.log('E','select tokenCode failed!'..scan.dump(select_tokenCode_sql))
	end
	args,triTime,fromTime,toTime,ok = nil,nil,nil,nil,nil		
	--初始化文件
	local ok,filename = initFile(IMEI)
	if not ok or not filename then
		only.log('E','File initialize ERROR!')
		return
	end
	ok = nil
	collectgarbage("collect")
	--遍历tokenCode
	local gps_cnt = 0
	for i, v in ipairs(ret) do
		local tokenCode, startTime, endTime = v['tokenCode'], v['startTime'], v['endTime']
		if startTime > endTime then
			only.log('E', string.format("ERROR todo task IMEI %s tokenCode %s from %s to %s", 
				IMEI, tokenCode, startTime, endTime))
		end
		only.log('D', string.format("IMEI %s tokenCode %s from %s to %s", IMEI, tokenCode, startTime, endTime))
		gps_cnt = gps_cnt + (endTime - startTime)  --统计GPS点数

		local st_time = tonumber(startTime)
		local ed_time = tonumber(endTime)
		local frame_start_time = st_time
		local frame_array = {}
		local frame_idx = 1

		--循环计算每个frame数据
		--以两个小时时长作为一个frame单位进行计算
		repeat
			local frame_end_time = (math.floor(frame_start_time / FRAME_TIME)  + 1) * (FRAME_TIME) - 1
			if frame_end_time >= ed_time then	--最后一个frame
				frame_end_time = ed_time
			end

			local gps_frame = GPSFrame:new(IMEI, tokenCode, frame_start_time, frame_end_time, filename)
			frame_array[frame_idx] = gps_frame
			only.log('D', string.format('frame idx:%s, start:%s, end:%s', 
				frame_idx, frame_start_time, frame_end_time))
			
			--处理数据
			gps_frame:process()

			frame_start_time = frame_end_time + 1
			frame_idx = frame_idx + 1
			memory_analyse("frame repeat")
			collectgarbage("collect")
		until frame_end_time >= ed_time

		local t_end = socket.gettime()
		only.log('I', "IMEI:%s, tokenCode:%s, end cost:%s", IMEI, tokenCode, t_end - t1)
		memory_analyse("the end")
	end
	
	--打包文件
	local name = utils.str_split(filename,'.')[1]
	os.execute(string.format('tar zcvf %s.tar.gz %s',name,filename))

	--上传文件
	local name = utils.str_split(name,'/')[3]
	os.execute(string.format('nohup bash /home/liu/data/myfile/put2kkb.sh %s.tar.gz', name))
	local file = io.open('nohup.out','r')
	if file then
		for line in file:lines() do
			gosay.resp_msg(msg['MSG_DO_FTP_FAILED'],line)
		end
	end	
	name = nil
	collectgarbage("collect")
	
	gosay.resp_msg(msg['MSG_SUCCESS'])
end
