local only = require('only')
local utils = require('utils')
local link  = require ("link")
local msg = require('msg')
local map = require('map')
local json = require('cjson')
local supex = require('supex')
local lua_decoder = require('libluadecoder')
local http_api = require('http_short_api')
local utils = require('utils')
local socket = require('socket')
local cfg = require('cfg')
local scan = require('scan')
local gosay = require ('gosay')
local func_search_poi = require('func_search_poi')
local redis_api = require('redis_pool_api')

local onelevecount,twolevecount,threelevecount = 0,0,0
local TWO_HOURS = 3600*2  
local TEN_MINUTES  = 600

module('domile', package.seeall)

local RT = {

	[0] 	=  	120,
	[1] 	= 	80,
	[2] 	= 	70,
	[3] 	= 	60,
	[4]     = 	50,
	[5] 	= 	30,
	[10] 	=	80,
	[11]	=	60,
	[12]	=	40
}	


local url_tab = {
        type_name = 'map',
        app_key = '',
        client_host = '',
        client_body = '',
}

--功能作用：参数检查
--参数    ：accountID,imei,startTime,endTime
--返回值  ：返回 ok，false
local function check_parameter(args)
    only.log('D', '###$$$>>>>check args ')
        -->> safe check
       --safe.sign_check(args, url_tab)
    if not  args['tokenCode'] then
            only.log('E', "requrie json have nil of \"tokenCode\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"tokenCode")
    end
        args['startTime'] = tonumber(args['startTime'])
        args['endTime'] = tonumber(args['endTime'])
    if not args['endTime'] then
        only.log('E', "requrie json have nil of \"time\"")
        gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"args['endTime']")
    end
    return args
end


--功能作用：从tsearhapi接口获取http请求后的数据
--参数    ：tsearch_api,imei,st_frame,ed_frame
--返回值  ：解码后数据返回
local function get_data_from_tsearchapi(tsearch_api,imei,st_frame,ed_frame )

    local body_info = {imei = imei, startTime = st_frame, endTime = ed_frame}
    only.log('D', "body_info" .. scan.dump(body_info))
    local serv = link["OWN_DIED"]["http"]["getgps"]
    local body = utils.gen_url(body_info)
    local api_name = "tsearchapi/v2/" .. tsearch_api
    local body_data = utils.compose_http_json_request(serv, api_name, nil, body)
    if not body_data then
	ngx.say("body_data if faile")
    end
    local ok, ret = supex.http(serv['host'], serv['port'], body_data, #body_data)
--    local  ret = http_api.http(serv, body_data,true)
    if not ok or not ret then return nil end
          -->获取RESULT后的数据
 --   only.log('D', "%s", ret)

    local data = utils.parse_api_result(ret, tsearch_api)
    if not data then
        return {}
    end
    if #data == 0 then return {} end
          --data中，偶数下标的元素才是gps数据,去掉奇数元素
        for k,_ in ipairs (data) do
        	table.remove(data, k)
		    k = k + 2
        end
          --使用tsearch解码格式解析数据
    local frame_data = lua_decoder.decode(#data, data)
        data = nil
    if not frame_data then
        return {}
    end
-- 	only.log('D', "frame_data" .. scan.dump(frame_data))
    return true,frame_data
end


--功能作用 ：调用tsearchapi之前要进行分割时间，以2小时为段长
--参数     ：开始时间，结束时间，imei
--返回值   ：从tsearchapi中取到的数据进行分割处理，返回处理后的数据

local function gps_time_handle(imei,startTime,endTime)
    if not endTime then
	    endTime = startTime + 49*3600
    end
	local st_frame = 0
	local ed_frame = startTime - TEN_MINUTES
	repeat
	        st_frame = ed_frame + TEN_MINUTES
	        ed_frame = st_frame + TWO_HOURS
	        if ed_frame > endTime then
	                ed_frame = endTime
                end
		local ok = from_gpsdata_get_part_mileageinfo(imei,st_frame,ed_frame,startTime,endTime)
		if not ok then
		only.log('E',"from_gpsdata_get_part_mileageinfo is error")
		end
	until ed_frame == endTime
--	only.log('D',"datamileage"..scan.dump(datamileage))
	return true
end



--功能作用：从固定位置进行数据计算
--参数    ：data_mileage,startTime,endTime
--返回值  ：取到数据放到table中进行返回
local function handle_gps_data(gps_data,startTime,endTime)
	part_mileageinfo = {}
	for k,v in pairs(gps_data) do	
		local tmp = {}
		tmp['GPSTime'] = tonumber(v[1])
		tmp['imei'] = v[3]
		tmp['accountID'] = v[4]
		tmp['longitude'] = tonumber(v[7])/10000000
		tmp['latitude'] = tonumber(v[8])/10000000
		tmp['altitude'] = tonumber(v[9])
		tmp['direction'] = tonumber(v[10])
		tmp['speed'] = tonumber(v[11])
		tmp['token_code'] = v[12]
		if (tmp['GPSTime'] >=  startTime) or (tmp['GPSTime'] <=  endTime) then
			table.insert(part_mileageinfo,tmp)
		end
	end
	return true
end



--功能作用 ：从tsearchapi接口获取数据，然后在data_handle_mileage函数中处理
--参数	   ：imei，st_frame,ed_frame,startTime,endTime
--返回值   ：返回从data_handle_mileage函数处理后的数据

local function from_gpsdata_get_part_mileageinfo(imei,st_frame,ed_frame,startTime,endTime)

	local tsearch_api = "getgps"
	local ok,gps_data = get_data_from_tsearchapi(tsearch_api,imei,st_frame,ed_frame)
	if gps_data == nil then
		return
	end
--	only.log('D', "gps_data" .. scan.dump(gps_data))
	if not ok then
		only.log('I',string.format("tsearch_api:%s imei:%s st_frame:%s ed_frame:%s",tsearch_api,imei,st_frame,ed_frame))
	end
	local ok = handle_gps_data(gps_data,startTime,endTime)
--	only.log('D', string.format("part_mileageinfo :%s", scan.dump(part_mileageinfo)))	
	if not ok then
		only.log('I',string.format("get data fail by:startTime:%s endTime :%s",startTime,endTime))
	end
	return true
end



---功能作用  ：限速处理
---参数	     ：speed，rt
--返回值     ：返回统计的数据

local function handle_mileage_speed(speed,rt)
	if speed > RT[rt] then
		local v = RT[rt]
		local j = (speed-v)/v * 100
                if j >= 0.1 and j < 0.2 then
		onelevecount = onelevecount + 1
		end
		if j >= 0.2 and j < 0.5 then
		twolevecount = twolevecount + 1
		end
		if j >= 0.5 then
		threelevecount = threelevecount + 1
		end
        end
 end



---功能作用 ：得到整个时间段的数据，每段路数据
---参数	    ：real_mileage_data
---返回值   ：返回统计后的数据

local function get_mileage_data(tokencode)

	local firstgpspoint,ret_count,tmpRRID_SRID = 1,0
	local pointcount,sum_speed,maxspeed,starttime,endtime,avgspeed,imei,rrid,sgid,countycode = 0,0,0,0,0,0
	local filename = string.format("/data/ye/ye/%s",tokencode)
	local fd = io.open(filename,"wr+")
--	fd:write("\n")
--	fd:write("imei|tokencode|rrid|sgid|maxspeed|avgspeed|pointcount|onelevecount|twolevecount|threelevecount|starttime|endtime|accountID|countycode")	
	for k,v in pairs(part_mileageinfo)do
		local len = table.maxn(part_mileageinfo)
		local ok,data = redis_api.cmd('road','','hmget','MLOCATE',v["imei"],v["longitude"],v["latitude"],v["direction"],v["altitude"],v["speed"],v["GPSTime"])
		if not ok then
		only.log('I', string.format("get data failed by : [** imei:%s, lon:%s, lat:%s, dir:%s, alt:%s, spe:%s , gpst:%s**]", imei,longitude,latitude,direction,altitude,speed,gpstime))
		end
--		only.log('D',"data"..scan.dump(data))	
		local RRID_SRID = string.format("%s%s",data[1],data[2])
		only.log('D', string.format("RRID_SRID =:%s", scan.dump(RRID_SRID)))
	-------------------  每段路的数据统计
			if  data [1] == nil or data [2] == nil  then
			        goto continue
			end
				
	----- GPS前后两个点的GGRD与RRID相同时，进行该段路上的里程处理
				::once::
	        if  tmpRRID_SRID == RRID_SRID or firstgpspoint == 1 then
			if v["speed"] > maxspeed then
			maxspeed = v["speed"]
			end
			pointcount = pointcount + 1
			if pointcount == 1 then
			starttime = v["GPSTime"]
                        end
                        handle_mileage_speed(v["speed"],tonumber(data[5]))
                        rrid = data[1]
		        sgid = data[2]
		        countycode = data[4]
		        imei = v["imei"]
		        accountID = v["accountID"]
		        tokencode = v["token_code"]
                        endtime = v["GPSTime"]
		        sum_speed = sum_speed + v["speed"]
		        firstgpspoint = firstgpspoint + 1
		        tmpRRID_SRID = RRID_SRID
                end

	----- GPS前后两个点的GGRD与RRID不相同时，进行上段路上的里程信息写到文件中
	------同时将第一次出现不同点的GPS数据，在次循环相同时GPS数据统计，直到出现取到GPS数据不同
	        if  tmpRRID_SRID ~= RRID_SRID or k == len  then
                        avgspeed = sum_speed / pointcount
                        if avgspeed == 0 then
                                return
                        end
                        local result = {imei,tokencode,rrid,sgid,maxspeed,avgspeed,pointcount,onelevecount,twolevecount,threelevecount,starttime,endtime,accountID,countycode}
                        avgspeed,maxspeed,sum_speed,pointcount,endtime,starttime,onelevecount,twolevecount,threelevecount = 0,0,0,0,0,0,0,0,0
                        fd:write("\n")
                        fd:write("|")
                        for k1,v1 in pairs(result)do
                        fd:write(v1,"|")
                        end
                        fd:write("\n")
                        fd:flush()
                        tmpRRID_SRID = RRID_SRID
                        if k == len  then
                                break
                        end
                        goto once
                end
                        ::continue::
	end

	fd:close()
	return true
end


function handle()

	local args = supex.get_our_body_table()
	only.log('D', "args" .. scan.dump(args))
	local args = check_parameter(args)
	local ok = gps_time_handle(args['imei'],args['startTime'],args['endTime'])
	if  not ok then
	only.log('D',"mile_data is error")
	end
	local ok = get_mileage_data(args['tokenCode'])
	if ok  then
        only.log('D',"get_data is success")
	end
end


