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
local ok,addcount,aa = true,0
local datamileage = {}
local TWO_HOURS = 3600*2  
local TEN_MINUTES  = 600

module('domile', package.seeall)


local RT = {

		[0] 	=  	120,
		[1] 	= 	80,
		[2] 	= 	70,
		[3] 	= 	60,
		[4]		= 	50,
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
    if not args['accountID']  then
        only.log('E', "requrie json have nil of \"accountID\"")
        gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"accountID")
    elseif string.len(args['accountID']) == 10 then
        local ok, imei = redis_pool_api.cmd('private', 'get', string.format('%s:IMEI', args['accountID']))
        if not ok or not imei then
            only.log('E', "get IMEI error")
            gosay.go_false(url_tab, msg['MSG_DO_REDIS_FAILED'])
        end
        args['imei'] = imei
    elseif string.len(args['accountID']) == 15 then
        local ok, accountID = redis_pool_api.cmd('private', 'get', string.format('%s:accountID', args['accountID']))
        if not ok or not accountID then
            only.log('E', "get accountID error")
            accountID = ''
        end
        args['imei'] = args['accountID']
        args['accountID'] = accountID›  
    end
    if not  args['tokenCode'] then
            only.log('E', "requrie json have nil of \"tokenCode\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"tokenCode")
    end
        args['startTime'] = tonumber(args['startTime'])
        args['endTime'] = tonumber(args['endTime'])
    if not args['startTime'] then
            only.log('E', "requrie json have nil of \"time\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"startTime")
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
  
       --  local ok, ret = supex.http(serv['host'], serv['port'], body_data, #body_data)
    local  ret = http_api.http(serv, body_data,true)
        -- if not ok or not ret then return nil end
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
 --only.log('D', "frame_data" .. scan.dump(frame_data))
	  return frame_data
end


--功能作用 ：调用tsearchapi之前要进行分割时间，以2小时为段长
--参数     ：开始时间，结束时间，imei
--返回值   ：从tsearchapi中取到的数据进行分割处理，返回处理后的数据

function mileage_time_handle(imei,startTime,endTime)
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
		    datamileage  = get_from_mileage_data(imei,st_frame,ed_frame,startTime,endTime)
			if not datamileage then
			only.log('E',string.format("get_from_mileage_data is error :%s",datamileage))
			end

	    until ed_frame == endTime
--	only.log('D',"datamileage"..scan.dump(datamileage))
	return ok
end



--功能作用：从固定位置进行数据计算
--参数    ：data_mileage,startTime,endTime
--返回值  ：取到数据放到table中进行返回
local function data_handle_mileage(data_mileage,startTime,endTime)

	for k,v in pairs(data_mileage) do	
		local tmp = {}
		local mile_data = {}
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
--	only.log('D', string.format("tmp :%s", scan.dump(tmp)))	
			table.insert(mile_data,tmp)
		end
	end
	return mile_data
end



--功能作用  ：从tsearchapi接口获取数据，然后在data_handle_mileage函数中处理
--参数	  ：imei，st_frame,ed_frame,startTime,endTime
--返回值   ：返回从data_handle_mileage函数处理后的数据

local function get_from_mileage_data(imei,st_frame,ed_frame,startTime,endTime)

	aa = imei
	local tsearch_api = "getgps"
	local data_mileage = get_data_from_tsearchapi(tsearch_api,imei,st_frame,ed_frame)
--	only.log('D', "data_mileage" .. scan.dump(data_mileage))
	if not data_mileage then
		only.log('E',"tsearch_api:%s imei:%s st_frame:%s ed_frame:%s",tsearch_api,imei,st_frame,ed_frame)
	end
	local handledata = data_handle_mileage(data_mileage,startTime,endTime)
	if not handledata then
		only.log("handledata is fail")
	end
--	only.log('D', "handledata" .. scan.dump(handledata))
	return handledata
end



---功能作用   ：限速处理
---参数	    ：speed，rt
--返回值     ：返回统计的数据

local function handle_speed(speed,rt)	

	for k,v in pairs(RT)do	
        if k == rt  and speed >= v then
			j = (speed-v)/v * 100
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
 end



---功能作用 ：得到整个时间段的数据，每段路数据
---参数	    ：real_mileage_data
---返回值   ：返回统计后的数据

 function get_data()
	local count,m = 1
	local pointcount,sum_speed,maxspeed,starttime,endtime,etime,avgspeed,imei,rrid,sgid,countycode = 0,0,0,0,0,0
--	only.log('D', string.format("real_mileage_data:%s", scan.dump(real_mileage_data)))
	local filename 	= string.format("/data/ye/%s",aa)
	local fd 	= io.open(filename,"rw+")
	fd:write("imei|tokencode|rrid|sgid|maxspeed|avgspeed|pointcount|onelevecount|twolevecount|threelevecount|starttime|endtime|accountID|countycode")
	fd:write("\n")
	for k,v in pairs(datamileage)do
		local len = table.maxn(datamileage)
		only.log('D', string.format("len =:%s", scan.dump(len)))
		local ok,data = redis_api.cmd('road','','hmget','MLOCATE',v["imei"],v["longitude"],v["latitude"],v["direction"],v["altitude"],v["speed"],v["GPSTime"])
--		only.log('D', string.format("data:%s", scan.dump(data)))
		if not ok and not data then
		only.log('I', string.format("get data failed by : [** imei:%s, lon:%s, lat:%s, dir:%s, alt:%s, spe:%s , gpst:%s**]", imei,longitude,latitude,direction,altitude,speed,gpstime))
		end		 
		local t = string.format("%s%s",data[1],data[2])
		only.log('D', string.format("t =:%s", scan.dump(t)))

	-------------------  每段路的数据统计
			if  data [1] == nil or data [2] == nil  then
				goto continue
			end				
				::once::
	        if  m == t or count == 1 then    
				if v["speed"] > maxspeed then
				    maxspeed = v["speed"]
				end
				pointcount = pointcount + 1
				if pointcount == 1 then
				    starttime = v["GPSTime"]	
				end	
					handle_speed(v["speed"],tonumber(data[5]))
					rrid = data[1]       
	            	sgid = data[2]
	            	countycode = data[4]
	            	imei = v["imei"]
	            	accountID = v["accountID"]
	            	tokencode = v["token_code"]
					endtime = v["GPSTime"]
	            	sum_speed = sum_speed + v["speed"]
	            	count = count  + 1	        	
	            	m = t
	           	only.log('D', string.format(" m =:%s", scan.dump(m)))
	        end

	        if  m ~= t or k == len  then
				avgspeed = sum_speed / pointcount
				addcount = addcount + pointcount
				local result = {imei,tokencode,rrid,sgid,maxspeed,avgspeed,pointcount,onelevecount,twolevecount,threelevecount,starttime,endtime,accountID,countycode}
				avgspeed,maxspeed,sum_speed,pointcount,endtime,starttime,onelevecount,twolevecount,threelevecount = 0,0,0,0,0,0,0,0,0
				for k,v in pairs(result)do
					fd:write(v,"|")
			    end   
					fd:flush()
					m = t
				if k == len then
					break
				end 
				goto once
			end 
			::continue::					
	end
		fd:close()
		return addcount
end




function handle()

	local args = supex.get_our_body_table()
	only.log('D', "args" .. scan.dump(args))
	check_parameter(args)
	local ok = mileage_time_handle(args['imei'],args['startTime'],args['endTime'])
	--only.log('D', string.format("real_mileage_data :%s", scan.dump(real_mileage_data)))
	if  not ok  then
	only.log('E',"real_mileage_data error")
	end
	local ok = get_data()
	if ok then
		return ok
	end

end


