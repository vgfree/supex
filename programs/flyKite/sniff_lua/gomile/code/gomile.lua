local only = require('only')
local utils = require('utils')
local link  = require ("link")
local msg = require('msg')
local map = require('map')
local json = require('cjson')
local supex = require('supex')
local lua_decoder = require('libluadecoder')
local http_api = require('http_short_api')
local redis_pool_api = require('redis_pool_api')
local utils = require('utils')
local socket = require('socket')
local cfg = require('cfg')
local func_search_poi = require('func_search_poi')
local fun_point_match_road = require('fun_point_match_road')

local mapServer = link.OWN_DIED.http.getgps
local time_sub_frame = cfg.time_sub_frame
local packet_time = cfg.packet_time
local min_sub_speed = cfg.min_sub_speed
local min_count = cfg.min_count 
local max_aver_speed = cfg.max_aver_speed
local addTravelInfo = link["OWN_DIED"]["http"]["addTravelInfo"]
local TravelInfo, ret = {}
local EARTH_RADIUS = 6378.137
local G_time, G_lat, G_lon, G_dir, G_speed = cfg.collectTime, cfg.latitude, cfg.longitude, cfg.direction, cfg.onespeed --注意取过来的数据如果字段位置对应修改，则其他赋值nil的字段位置也要更改
local G_redis_time = cfg.redis_gps_data_min_time
local one_range_time = cfg.one_range_time
local TEN_MINUTES = 600
local TWO_HOURS = 3600*2
local url_tab = { 
        type_name = 'map',
        app_key = '',
        client_host = '',
        client_body = '',
}

module('gomile', package.seeall)

local function check_parameter(args)
	if not args['accountID']  then
		only.log('E', "requrie json have nil of \"accountID\"")
		return false
        elseif string.len(args['accountID']) == 10 then
		local ok, imei = redis_pool_api.cmd('private', '','get', string.format('%s:IMEI', args['accountID']))
		if not ok or not imei then
			only.log('E', "get IMEI error")
			return false
		end
		args['imei'] = imei
	elseif string.len(args['accountID']) == 15 then
		local ok, accountID = redis_pool_api.cmd('private', '','get', string.format('%s:accountID', args['accountID']))
		if not ok or not accountID then
			only.log('W', "get accountID error")
			accountID = ''
		end
		args['imei'] = args['accountID']
		args['accountID'] = accountID	
	end
        if not  args['tokenCode'] then
                only.log('E', "requrie json have nil of \"tokenCode\"")
		return false
        end
        args['startTime'] = tonumber(args['startTime'])
        args['endTime'] = tonumber(args['endTime'])
        if not args['startTime'] then
                only.log('E', "requrie json have nil of \"time\"")
		return false
        end
	TravelInfo['startTime'] = args['startTime']
	TravelInfo['tokenCode'] = args['tokenCode']
	TravelInfo['accountID'] = args['accountID']
	TravelInfo['imei'] = args['imei']
	TravelInfo['sumMileage'] = 0
	TravelInfo['actualMileage'] = 0

	return true
end

--排序原则，按时间顺序排列,a代表（两个数组元素中排序在前面的点），时间小的排前面, 
local function cmp(a,b)
        return a[G_time] < b[G_time]
end


local function get_time_dist(m, n) ---先判断是否相同
	local F_lon, F_lat, T_lon, T_lat = ret[m][G_lon], ret[m][G_lat], ret[n][G_lon], ret[n][G_lat]  --start,end
	--only.log('D', string.format("flon %.7f flat %.7f tlon %.7f tlat %.7f", F_lon, F_lat, T_lon, T_lat))
        if F_lon == T_lon and  F_lat == T_lat then
		return 0
	end
	local dist = EARTH_RADIUS*math.acos(math.sin(F_lat/57.2958)*math.sin(T_lat/57.2958)+math.cos(F_lat/57.2958)*math.cos(T_lat/57.2958)*math.cos((F_lon-T_lon)/57.2958)) *1000
	if not dist or tostring(dist) == "nan" then 
	        dist = 0
	end
	if dist / (ret[n][G_time]- ret[m][G_time]) >= max_aver_speed then
		dist = 0
	end
	--only.log('D', string.format("dist %d", dist))
        return dist
end

local function get_real_mileage()
        local sum, dist = 0, 0
	local k = 1
	while ret[k] do
		if not ret[k]['status'] then
			local start = k
                        --取下一个点（用于算两点距离）
			k = k + 1
			--如果一下一个是无效点则跳过
	                while ret[k] and ret[k]['status'] do
	                        k = k + 1
	                end
	                --循环结束直接推出
	                if not ret[k] then
	                        break
	                end
			dist = 	get_time_dist(start, k)
			sum = sum + dist
		else
		        k = k + 1
		end
		
	end
        only.log('D', "real_sum  == " .. sum)
        return sum
end

local function get_vaild_mileage()
        local sum, dist = 0, 0
	local k = 2
	while ret[k] do
		if not ret[k]['status'] then
			if ret[k-1] and not ret[k-1]['status'] and  (ret[k][G_time] - ret[k-1][G_time]) < packet_time  then
				local get_sign = 0 
				if (ret[k][G_time] - ret[k-1][G_time]) == 1 and math.abs(ret[k][G_speed] - ret[k-1][G_speed]) < min_sub_speed then
					get_sign = 1
				elseif (ret[k][G_time] - ret[k-1][G_time]) >= 2 then
					get_sign = 1
				end
				if get_sign == 1 then
					dist = 	get_time_dist(k-1, k)
					sum = sum + dist
				end
			end 
		end
		k = k + 1
	end
        only.log('D', "vaild_sum  == " .. sum)
        return sum
end
local function data_initialize(last_final_point, tokenCode)
	if last_final_point then
                table.insert(ret, 1, last_final_point)
        end
	local k, v = 1, ret[G_time] 
	while ret[k] do
		v = ret[k] --FIXME ret{1,2,3,4,5,6,7,8,9,10,11,12}
		if #(v) ~= 12 then
			return false
		end
		if v[12] ~= tokenCode then
                        table.remove(ret, k)
                else
                        ret[k][G_time] = tonumber(v[G_time])
                        ret[k][2] = nil 
                        ret[k][3] = nil
                        ret[k][4] = nil
                        ret[k][5] = nil
                        ret[k][6] = nil
                        ret[k][G_lon] = tonumber(v[G_lon])/10000000
                        ret[k][G_lat] = tonumber(v[G_lat])/10000000
                        ret[k][9] = nil
                        ret[k][G_dir] = tonumber(v[G_dir])
                        ret[k][G_speed] = tonumber(v[G_speed])
                        --ret[k][12] = nil --FIXME
			k = k + 1
                end
	end
	if #(ret) == 0 then
		return false
	end
        only.log('D',string.format("latitude %f longitude %f direction %d collectTime %d tokenCode %s",ret[1][G_lat],ret[1][G_lon],ret[1][G_dir],ret[1][G_time], ret[1][12]))
	return true
end


local function delete_all(ret, start_num, end_number)
        while ret[start_num] do
                ret[start_num]['status'] = 1
                if start_num == end_number then
                        break
                end
                start_num = start_num + 1
        end
end

function add_break_status_range(ret, k)         
	local lat_min, lat_max = ret[k][G_lat] - 0.002, ret[k][G_lat] + 0.002        
	local lon_min, lon_max = ret[k][G_lon] - 0.002, ret[k][G_lon] + 0.002        
	local first_point, v = k, 0        
	local time_start = ret[k][G_time]	
	--only.log('D', "start Time" .. time_start)        
	local error_count, sign = 0, false        
	while ret[k] and not ret[k]['status'] do
                v = ret[k]
                if v[G_time] - time_start > one_range_time then
                        break
                end
                if v[G_lat] >= lat_min and  v[G_lat] <= lat_max and v[G_lon] >= lon_min and v[G_lon] <= lon_max then
                        error_count = error_count + 1
                end
                 k = k + 1
        end
        if not ret[k] then
                k = k - 1
        end
        if error_count/(k - first_point) > 0.8 then
                delete_all(ret, first_point, k)
        else
                sign = true
        end
        return k, sign
end

local function get_part_mileage()
	local k, data_sign, vaild_data_sign = 1, false, false
	while ret[k] do
		k, data_sign = add_break_status_range(ret, k)
                if data_sign then --判断是否取得点存在有效点
                        vaild_data_sign = true 
                end
		k = k + 1
	end
        local tmp_vaild_mileage, tmp_real_mileage
        if not vaild_data_sign then
                tmp_vaild_mileage, tmp_real_mileage = 0, 0 
        else
                tmp_real_mileage = get_real_mileage()
		k = 1
		while ret[k] do
		        if ret[k][G_dir] < 0 then
		                ret[k]['status'] = 1
		        end
		        k = k + 1 
		end
                tmp_vaild_mileage = get_vaild_mileage()
        end
	
	return tmp_vaild_mileage, tmp_real_mileage 
end

local function get_sum_mileage(tmp_vaild_mileage, tmp_real_mileage, do_number)
	--记录最后一个点，然后将最后一个点作为下一次包的开始点
        local count = #(ret)
        ret[count][G_lat], ret[count][G_lon] = ret[count][G_lat]*10000000, ret[count][G_lon]*10000000
        TravelInfo['sumMileage'] = TravelInfo['sumMileage'] + tmp_vaild_mileage
        only.log('D', string.format("new sum_mileage  ==  %d", TravelInfo['sumMileage']))
        TravelInfo['actualMileage'] = TravelInfo['actualMileage'] + tmp_real_mileage
	TravelInfo['endLongitude'] = ret[count][G_lon] 
	TravelInfo['endLatitude'] = ret[count][G_lat]  
	TravelInfo['endTime'] = ret[count][G_time]
        if do_number == 1 then
                TravelInfo['startLongitude'] = ret[1][G_lon] * 10000000
	        TravelInfo['startLatitude'] = ret[1][G_lat] * 10000000
       	end
	local last_final_point = ret[count]
        ret = nil
	return last_final_point, true
end
local function do_part_line_mileage(do_number, last_final_point, tokenCode)
	local true_data_sign = data_initialize(last_final_point, tokenCode)
	if not true_data_sign then
		return last_final_point, false
	end
	local tmp_vaild_mileage, tmp_real_mileage =  get_part_mileage()
	return get_sum_mileage(tmp_vaild_mileage, tmp_real_mileage, do_number)
end

local function lrange_data_handle(data)
        for k in pairs (data) do
		table.remove(data, k)
        end
        ret = lua_decoder.decode(#(data), data)
	data = nil
        if not ret then
                print("url decode error")
                return false 
        end
        --table.sort(ret, cmd)
        return true 
end

-->通过tsearchapi接口取出数据
local function get_data_from_tsearchapi(imei, st_frame, ed_frame)
	local body_info = {
		imei = imei,
		startTime = st_frame,
		endTime = ed_frame,
	}    
	local body = utils.gen_url(body_info)
	local data = "POST /tsearchapi/v2/getgps HTTP/1.0\r\n" ..
				"User-Agent: curl/7.33.0\r\n" ..
				--"HOST:" .. serv['host'] .. ":" .. tostring(serv['port']) .. '\r\n' ..
				"HOST:" .. mapServer.host .. ":" .. tostring(mapServer.port) .. '\r\n' ..
				"Connection: close\r\n" ..
				"Content-type: application/json\r\n" ..
				"Content-Length:" .. #body .. "\r\n" ..
				"Accept: */*\r\n\r\n" ..
				body 
	local ok, ret = supex.http(mapServer.host, mapServer.port, data, #data)
	
	if not ok then
		only.log('E', 'Failed to get ret !')
		return nil
	end
	if not ret then
		only.log('E', 'Location failed !')
		return nil
	end
	-->获取RESULT部分
	local data = utils.parse_api_result(ret, "getgps")
	
	if not data then
		return nil
	end
	
	return data
end

-->每次取2小时的数据，取出所有数据
local function get_whole_data(tokenCode, imei, startTime, endTime)
	if not endTime then
		endTime = startTime + 49*3600
	end
	
	local st_frame = 0
	local ed_frame = startTime - TEN_MINUTES
	local j, last_final_point = 1
	
	repeat
		st_frame = ed_frame + TEN_MINUTES
		ed_frame = st_frame + TWO_HOURS
		if ed_frame > endTime then 
			ed_frame = endTime
		end  
		local t1 = socket.gettime()
		local data = get_data_from_tsearchapi(imei, st_frame, ed_frame)
		local t2 = socket.gettime()
		--only.log('S', string.format("tsearch time:%s", (t2 - t1)))
		
		if data and #(data) > 0 then 
			local sign = lrange_data_handle(data)
			data = nil 
			if sign then
				local data_sign = true
				last_final_point, data_sign = do_part_line_mileage(j, last_final_point, tokenCode)
				if not data_sign then
					only.log('E',string.format("basic data error time %s  to time %s", st_frame, ed_frame ))
				end 
			end 
		end
		j = j + 1
	until ed_frame == endTime
end


local function get_code_from_redis(key)
        local ok,data = redis_pool_api.cmd('mapGridOnePercent', '','get', key) --FIXME
        if not ok or not  data then
                local info = string.format("fail to get %s from redis %s", key, 'mapGridOnePercent')
                only.log('E', info)
                return
        end
        local ok, grid_info = pcall(json.decode, data)
        if not ok then
                only.log('D', "decode gridInfo failed")
                return
        end
        return grid_info
end

local function gps_data_get_road_info(roadID)
        if roadID == 0 then
                return ""
        end
	local key = roadID .. ":roadInfo"
	local ok, kv_tab = redis_pool_api.cmd('mapRoadInfo', '','hgetall', key)
	if  ok and kv_tab then
		return kv_tab['RN']
	else
		only.log('D', string.format("[hgetall road info error]"))
	end
	return ""
end

local function get_roadid_from_data(store, Longitude, Latitude)
	
	local longitude = tonumber(store[Longitude])/10000000
        local latitude = tonumber(store[Latitude])/10000000
	
	local ok,result = fun_point_match_road.entry(nil, longitude, latitude)
	if result == nil then
		return ""
	end

	return gps_data_get_road_info(result['roadID'])
end

-->获得附近POI
local function get_nearby_poi(store, Longitude, Latitude)
        local tb = {
                longitude = tonumber(store[Longitude])/10000000,
                latitude = tonumber(store[Latitude])/10000000,
	}
        local res = func_search_poi.handle(tb)
        if res then
                return res['name']
        end

        return ""
end


local function get_start_end_city()
	if not TravelInfo['startLongitude'] or not  TravelInfo['startLatitude'] then
		return
	end
        local key = string.format("%d&%d",math.floor(TravelInfo['startLongitude']/100000),math.floor(TravelInfo['startLatitude']/100000))
        local grid_info = get_code_from_redis(key)
        if not grid_info then
                return 
        end
	TravelInfo['startPOIName'] = get_nearby_poi(TravelInfo, 'startLongitude', 'startLatitude')
       	TravelInfo['startRoadName'] = get_roadid_from_data(TravelInfo, 'startLongitude', 'startLatitude') 
	TravelInfo['startProvinceName'], TravelInfo['startProvinceCode'], 
        TravelInfo['startCityName'], TravelInfo['startCityCode'], 
        TravelInfo['startCountyName'], TravelInfo['startCountyCode'] = grid_info['provinceName'], grid_info['provinceCode'],
                                                                        grid_info['cityName'], grid_info['cityCode'],
                                                                        grid_info['countyName'], grid_info['countyCode']
        key = string.format("%d&%d",math.floor(TravelInfo['endLongitude']/100000),math.floor(TravelInfo['endLatitude']/100000))
        grid_info = get_code_from_redis(key)
        if not grid_info then
                return 
        end
	TravelInfo['endPOIName'] = get_nearby_poi(TravelInfo, 'endLongitude', 'endLatitude')
        TravelInfo['endRoadName'] = get_roadid_from_data(TravelInfo, 'endLongitude', 'endLatitude')
	TravelInfo['endProvinceName'], TravelInfo['endProvinceCode'], 
        TravelInfo['endCityName'], TravelInfo['endCityCode'], 
        TravelInfo['endCountyName'], TravelInfo['endCountyCode'] = grid_info['provinceName'], grid_info['provinceCode'],
                                                                        grid_info['cityName'], grid_info['cityCode'],
                                                                        grid_info['countyName'], grid_info['countyCode']                                                               
end
local function go_jave_mysql(res, appKey)
        res =string.format('{"ERRORCODE":"0","RESULT":%s}',res)
        local tb = {jsonTravel = res, appKey = appKey}
        -->> check sign
--	local secret = app_utils.get_secret(appKey)
--	if not secret then
--	        only.log('E',"get %s from public redis error", appKey .. ':secret')
--		gosay.go_false(url_tab, msg["MSG_DO_REDIS_FAILED"])
--	end
--	tb['sign'] = utils.gen_sign(tb, secret)
        local body = ''
	for k,v in pairs(tb) do
		body = string.format("%s%s=%s&", body, k, v)
	end
	body = string.sub(body, 1, -2)
        only.log('D', body)
        local post_data = utils.post_data('DataCore/autoGraph/addTravelInfo.do', addTravelInfo, body)
        only.log('D', post_data)
	local ok, ret = supex.http(addTravelInfo.host, addTravelInfo.port, post_data, #post_data)
	if not ok then
		only.log('D',"post data failed!")
	end        
end

function handle()
	local args          = supex.get_our_body_table()
	only.log('D', "args" .. scan.dump(args))

        -->check args
        if not check_parameter(args) then
		return
	end
        -->do api
       	get_whole_data(args['tokenCode'], args['imei'], args['startTime'], args['endTime'])
        get_start_end_city()
        local str = string.format('{"sumMileage":"%d","actualMileage":"%d","tokenCode":"%s"}', TravelInfo['sumMileage'], TravelInfo['actualMileage'], TravelInfo['tokenCode'])
	--only.log('S', str)
	local ok, res = pcall(json.encode, TravelInfo)
	if not ok or not res then
		only.log('E',"result json encode error")
		return false
	end
	TravelInfo = {}
	only.log('D', scan.dump(res))
        go_jave_mysql(res, args['appKey'])
end

