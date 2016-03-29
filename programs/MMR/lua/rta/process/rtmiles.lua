local utils = require('utils')
local only = require('only')
local mysql_api = require('mysql_pool_api')
local redis_api = require('redis_pool_api')
local cutils  = require('cutils')
local cjson   = require('cjson')
local cachekv = require('cachekv')
local luakv_api = require('luakv_pool_api')

module('rtmiles', package.seeall)

--从redis中取出数据
local function mile_from_redis(data_package)

	mile_table = {}
	local redis_key = data_package['IMEI'] .. ":" .. data_package['tokenCode'] .. ":mileage"
	local ok,ret = redis_api.cmd('RTmiles',data_package['IMEI'] or '',"hmget",redis_key,"startLongitude","startLatitude","startDirection","startTime","endLongitude","endLatitude","endDirection","endTime","createTime","sumMileage","actualMileage","GPSLossRate","normal","maxSpeed","avgSpeed","speedsum","gpscont","driveTime","runningTime","stopTime","stopNum","lowSpeedTime","stop_switch","drive_switch")
	--local ok,mile_table = luakv_api.cmd('RTmiles',data_package['IMEI'] or '',"hgetall",redis_key)
	
	if not ok or not ret then
		only.log('E',"data from redis error")
		return
	end

	if next(ret) == nil then
		mile_table['startLongitude'] = data_package['points'][1]['longitude']
		mile_table['startLatitude'] = data_package['points'][1]['latitude']
		mile_table['startDirection'] = data_package['points'][1]['direction']
		mile_table['startTime'] = data_package['startTime']
		mile_table['endLongitude'] = 0
		mile_table['endLatitude'] = 0
		mile_table['endDirection'] = 0
		mile_table['endTime'] = 0
		mile_table['createTime'] = os.time()
		mile_table['sumMileage'] = 0
		mile_table['actualMileage'] = 0
		mile_table['GPSLossRate'] = 0
		mile_table['normal'] = 0	--0 正常停车 1 异常停车
		mile_table['maxSpeed'] = 0
		mile_table['avgSpeed'] = 0
		mile_table['speedsum'] = 0	--总速度
		mile_table['gpscont'] = 0	--总gps点
		mile_table['driveTime'] = 0
		mile_table['runningTime'] = 0
		mile_table['stopTime'] = 0
		mile_table['stopNum'] = 0
		mile_table['lowSpeedTime'] = 0
		mile_table['stop_switch'] = 0
		mile_table['drive_switch'] = 0
	else
		
		mile_table['startLongitude'] = ret[1]
		mile_table['startLatitude'] = ret[2]
		mile_table['startDirection'] = ret[3]
		mile_table['startTime'] = ret[4]
		mile_table['endLongitude'] = ret[5]
		mile_table['endLatitude'] = ret[6]
		mile_table['endDirection'] = ret[7]
		mile_table['endTime'] = ret[8]
		mile_table['createTime'] = ret[9]
		mile_table['sumMileage'] = ret[10]
		mile_table['actualMileage'] = ret[11]
		mile_table['GPSLossRate'] = ret[12]
		mile_table['normal'] = ret[13]		--0 正常停车 1 异常停车
		mile_table['maxSpeed'] = ret[14]
		mile_table['avgSpeed'] = ret[15]
		mile_table['speedsum'] = ret[16]	--总速度
		mile_table['gpscont'] = ret[17]		--总gps点
		mile_table['driveTime'] = ret[18]
		mile_table['runningTime'] = ret[19]
		mile_table['stopTime'] = ret[20]
		mile_table['stopNum'] = ret[21]
		mile_table['lowSpeedTime'] = ret[22]
		mile_table['stop_switch'] = ret[23]
		mile_table['drive_switch'] = ret[24]

	end

	for k,v in pairs(mile_table) do 
		mile_table[k] = tonumber(mile_table[k])
	end

	--only.log('D',"start mile" .. scan.dump(mile_table))

	return mile_table

end

--将计算后的结果存入redis
local function mile_to_redis(mile_table)

	local hash_key = mile_table['IMEI'] .. ":" .. mile_table['tokenCode'] .. ":mileage"

	local ok,ret = redis_api.cmd('RTmiles',mile_table['IMEI'] or '',"HMSET",hash_key,
	--local ok,ret = luakv_api.cmd('RTmiles',mile_table['IMEI'] or '',"HMSET",hash_key,
			"startLongitude",mile_table['startLongitude'],
			"startLatitude",mile_table['startLatitude'],
			"startDirection",mile_table['startDirection'],
			"startTime",mile_table['startTime'],
			"endLongitude",mile_table['endLongitude'],
			"endLatitude",mile_table['endLatitude'],
			"endDirection",mile_table['endDirection'],
			"endTime",mile_table['endTime'],
			"createTime",mile_table['createTime'],
			"sumMileage",mile_table['sumMileage'],
			"actualMileage",mile_table['actualMileage'],
			"GPSLossRate",mile_table['GPSLossRate'],
			"normal",mile_table['normal'],
			"maxSpeed",mile_table['maxSpeed'],
			"avgSpeed",mile_table['avgSpeed'],
			"speedsum",mile_table['speedsum'],
			"gpscont",mile_table['gpscont'],		
			"driveTime",mile_table['driveTime'],
			"runningTime",mile_table['runningTime'],
			"stopTime",mile_table['stopTime'],
			"stopNum",mile_table['stopNum'],
			"lowSpeedTime",mile_table['lowSpeedTime'],
			"stop_switch",mile_table['stop_switch'],
			"drive_switch",mile_table['drive_switch']
			)


	if not ok or not ret then
		only.log('E',"mile to redis error")
		return 
	end
end

--计算里程
local function com_mileage(startpoint,endpoint)

	total_mileage = 0
	local GPSTime_diff = endpoint['GPSTime'] - startpoint['GPSTime']
	if GPSTime_diff > 15 then
		return false,total_mileage
	end
	
	local speed1 = ((startpoint['direction'] == -1) and 0) or startpoint['speed']
	local speed2 = ((endpoint['direction'] == -1) and 0) or endpoint['speed']
	total_mileage = ((speed1 + speed2) / 2) * (1000 / 3600) * GPSTime_diff

	return true,total_mileage

end


--里程及其他信息的计算
local function calcmil(mile_table,data_table)

	mile_table['IMEI'] = data_table['IMEI']			--存储imei做为key
	mile_table['tokenCode'] = data_table['tokenCode']	--存储tokenCode做为key
	
	redis_value = mile_table['IMEI'] .. ":" .. mile_table['tokenCode'] .. ":mileage"

	if data_table['isFirst'] then
		mile_table['startLongitude'] = data_table['points'][1]['longitude']
		mile_table['startLatitude'] = data_table['points'][1]['latitude']
		mile_table['startDirection'] = data_table['points'][1]['direction']
		mile_table['startTime'] = data_table['startTime']
		mile_table['createTime'] = os.time()

		local time_key = os.date("%Y-%m-%d",mile_table['startTime'])
		local ok,_ = redis_api.cmd('owner',data_table['IMEI'] or '','sadd',time_key,redis_value)
		if not ok then
			only.log('D',"sadd error")
			return 
		end
	end

	
		n = #(data_table['points'])
	
		--是否异常停车 1异常 0正常
		if data_table['points'][n]['direction'] ~= -1 and data_table['points'][n]['speed'] > 5 then
			mile_table['normal'] = 1
		else
			mile_table['normal'] = 0
		end
	
		mile_table['endLongitude'] = data_table['points'][n]['longitude']
		mile_table['endLatitude'] = data_table['points'][n]['latitude']
		mile_table['endDirection'] = data_table['points'][n]['direction']
		mile_table['endTime'] = data_table['endTime']
		

	for var = 1,n do

		if data_table['points'][var]['direction'] == -1 or data_table['points'][var]['speed'] == 0 then
			--only.log ('D','@@@stopTime')
			mile_table['stopTime'] = mile_table['stopTime'] + 1
		--行车时间
		elseif data_table['points'][var]['direction'] ~= -1 and data_table['points'][var]['speed'] ~= 0  then
			--only.log ('D','@@@runningTime')
			mile_table['runningTime'] = mile_table['runningTime'] + 1
			--低速行驶时长
			if data_table['points'][var]['speed'] < 5 then
				--only.log ('D','@@@lowSpeedTime')
				mile_table['lowSpeedTime'] = mile_table['lowSpeedTime'] + 1
			end
	        end


      		--计算停车次数
		if not data_table['isDelay'] then
			if data_table['points'][var]['direction'] ~= -1 and data_table['points'][var]['speed'] ~= 0 then
				mile_table['stop_switch'] = 0 
				mile_table['drive_switch'] =  1 
				--only.log ('D','trigger stop_switch')
			elseif mile_table['stop_switch'] == 0 and mile_table['drive_switch'] == 1 and (data_table['points'][var]['direction'] == -1 or data_table['points'][var]['speed'] == 0)  then
				mile_table['drive_switch'] = 0 
				mile_table['stop_switch'] = 1 
				mile_table['stopNum'] =  mile_table['stopNum'] +1
				--only.log ('D','trigger drive_switch')
			end
		end


		--找出最大速度
		if data_table['points'][var]['speed'] > mile_table['maxSpeed'] then
			mile_table['maxSpeed'] = data_table['points'][var]['speed']
		end
		--求出速度和与gps总点数，以便求出平均速度
		mile_table['speedsum'] = mile_table['speedsum'] + (((data_table['points'][var]['direction'] == -1) and 0) or data_table['points'][var]['speed'])
		mile_table['gpscont'] = mile_table['gpscont'] + 1

	--计算里程
		if var == 1 then
			--没有lastpoint或这个包是延迟包则不参与计算
			if not data_table['lastPoint'] or data_table['isDelay'] then
				goto CONTINE
			end
			ok,total_mileage = com_mileage(data_table['lastPoint'],data_table['points'][var])
			if not ok then
				goto CONTINE
			end
			total_mileage = total_mileage - total_mileage%0.01
			mile_table['actualMileage'] = mile_table['actualMileage'] + total_mileage
			mile_table['sumMileage'] = mile_table['sumMileage'] + total_mileage
		else	
			ok,total_mileage = com_mileage(data_table['points'][var-1],data_table['points'][var])
			if not ok then
				goto CONTINE
			end
			total_mileage = total_mileage - total_mileage%0.01
			mile_table['actualMileage'] = mile_table['actualMileage'] + total_mileage		
			if not data_table['points'][var-1]['isExtra'] or not data_table['points'][var]['isExtra'] then
				mile_table['sumMileage'] = mile_table['sumMileage'] + total_mileage
			end
		end
		::CONTINE::
	end --for

		--算出gps丢失率与平均速度
		local lossrate = ((mile_table['endTime'] - mile_table['startTime'] + 1 - mile_table['gpscont']) / (mile_table['endTime'] - mile_table['startTime'] + 1))*100
		mile_table['GPSLossRate'] = math.floor(lossrate)
		mile_table['avgSpeed'] = math.floor(mile_table['speedsum'] / mile_table['gpscont'])
		--计算驾驶时长
		mile_table['driveTime'] = mile_table['runningTime'] + mile_table['stopTime']

		
		--only.log('D',"cail mile " .. scan.dump(mile_table))
		return mile_table
end




function start(data_package)

		mile_table = {}
		--only.log('D',scan.dump(data_package))
		mile_table = mile_from_redis(data_package)
		if next(mile_table) == nil then
			return
		end
		calcmil(mile_table,data_package)
		only.log('D',"handle mile " .. scan.dump(mile_table))
		mile_to_redis(mile_table)
end


