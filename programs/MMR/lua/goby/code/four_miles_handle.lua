local cutils        = require('cutils')
local only      = require('only')
local supex      = require('supex')
local conf = require("four_miles_conf")
local luakv_api = require("luakv_pool_api")
local scan = require("scan")
local scene 		= require('scene')
local COUNT = require("count_four_miles")
local redis_api = require('redis_pool_api')
module('four_miles_handle',package.seeall)

-->>开机数据初始化
function init() 
	
	local accountID = supex.get_our_body_table()["accountID"]
	luakv_api.cmd('owner', accountID,"del",accountID .. ":poitype")
	luakv_api.cmd('owner', accountID,"del",accountID .. ":poiID")
	luakv_api.cmd('owner', accountID,"del",accountID .. ":count")
    	local ok1,roadID1 = luakv_api.cmd('owner', accountID, "get", accountID .. ":roadID1" )
    	local ok2,roadID2 = luakv_api.cmd('owner', accountID, "get", accountID .. ":roadID2" )
	luakv_api.cmd('owner', accountID,"del",accountID .. ":" .. roadID1)
	luakv_api.cmd('owner', accountID,"del",accountID .. ":" .. roadID2)
    	luakv_api.cmd('owner', accountID, "del", accountID .. ":roadID1" )
    	luakv_api.cmd('owner', accountID, "del", accountID .. ":roadID2" )
end

local function direction_sub(dir1, dir2)
	local angle = math.abs(dir1 - dir2)
	return (angle <= 180) and angle or (360 - angle)
end

local function sentenced_empty( poitype_table, app_name, roadID1 )
	if next(poitype_table) == nil then
		only.log('D',app_name .. "tabe is null")
		return false
	end
	if roadID1 then
		only.log('D',app_name .. "tabe content is")
		only.log('D',scan.dump(poitype_table))
		return poitype_table,roadID1
	else
		only.log('D',app_name .. "tabe content is")
		only.log('D',scan.dump(poitype_table))
		return poitype_table
	end
end

local function space_filter(poi_set,direction)
	only.log('D',scan.dump(poi_set))
	local poitype_table = {}
	for poiid, v in pairs(poi_set or {}) do
		v['poiID'] = poiid
		local ok = true
		local angle = tonumber( v["direction"] )
		local difference = direction_sub(angle, tonumber(direction))
		if difference <= 60 then
			ok = true
			--v['direction'] = angle
		else
			ok = false
		end
		if ok then
			if not poitype_table[v['type']] then
				poitype_table[v['type']] = {}
				table.insert(poitype_table[v['type']], v)
			else
				table.insert(poitype_table[v['type']], v)
			end
		end
	end
	return sentenced_empty(poitype_table, "space_filter")	
end

local function time_filter( poitype_table )
	local month = os.date('%m')
	month = tonumber(month)
	local hour = os.date('%H')
	hour = tonumber(hour)
	local week = os.date('%w')
	week = tonumber(week)
	for poitype,_ in pairs( poitype_table or {}) do
		repeat
		local time_conf = conf["POI_LIST"][poitype]["time_conf"]
		if time_conf == "1"then
			local hour_conf =conf["POI_LIST"][poitype]["hour"]
			for i=1,#hour_conf do
				hour1=tonumber(hour_conf[i]["hour1"])
				hour2=tonumber(hour_conf[i]["hour2"])
				if hour >= hour1 and  hour <= hour2 then
					poitype_table[poitype] = nil
					break
				end
			end
			local week_conf =conf["POI_LIST"][poitype]["week"]
			if week_conf then
				for i=1,#week_conf do
					week1=tonumber(week_conf[i]["week1"])
					week2=tonumber(week_conf[i]["week2"])
					if week >= week1 and week <=week2 then
						poitype_table[poitype] = nil
						break
					end
				end
			end
			local month_conf =conf["POI_LIST"][poitype]["month"]
			if month_conf then
				for i=1,#month_conf do
					month1=tonumber(month_conf[i]["month1"])
					month2=tonumber(month_conf[i]["month2"])
						if month >=month1 and month <= month2 then
							poitype_table[poitype] = nil
							break
						end
				end
			end
		end
		until true
	end
	return sentenced_empty(poitype_table, "time_filter")	
end

local function rule_filter( poitype_table, lon, lat)
    local poitype_new = {}
    for poitype, v in pairs(poitype_table or {}) do
	repeat
    	if not conf["POI_LIST"][poitype] then
		poitype_table[poitype] = nil
 		break
    	end
        local issued_Type = conf["POI_LIST"][poitype]["issued_Type"]
        if issued_Type == "1" then
            for i=1, #v do
                local max_mileage = -1
                local poi_lon   = v[i]["longitude"]
                local poi_lat   = v[i]["latitude"]
                --计算球面距离
                local mileage = cutils.gps_distance(lon, lat, poi_lon, poi_lat)
		only.log('D',string.format("[poitye:%s]:[mileage:%s]",poitype,mileage))
                mileage = tonumber(mileage) or 0
		
                if mileage  < (tonumber(conf["POI_LIST"][poitype]["dis"])) and (tonumber(conf["POI_LIST"][poitype]["receiver_dis"]))< mileage then
                    --记录符合下法距离并且距离最远的一个
                    if max_mileage == -1 or max_mileage < mileage  then
                        max_mileage = mileage 
			poitype_new[poitype] ={}
            		poitype_new[poitype] = poitype_table[poitype][i]
                    end
                end
            end
        end
        if issued_Type == "2" then 
		for i=1,#v do
                	local poi_lon   = v[i]["longitude"]
                	local poi_lat   = v[i]["latitude"]
                	--计算球面距离
                	local mileage = cutils.gps_distance(lon, lat, poi_lon, poi_lat)
			only.log('D',string.format("[poitye:%s]:[mileage:%s]",poitype,mileage))
                	mileage = tonumber(mileage) or 0
                	if mileage < (tonumber(conf["POI_LIST"][poitype]["dis"])) and (tonumber(conf["POI_LIST"][poitype]["receiver_dis"])) < mileage then
				if not poitype_new[poitype] then
					poitype_new[poitype] = {}
					table.insert(poitype_new[poitype],v[i]  )
				else
					table.insert(poitype_new[poitype],v[i]  )
				end
			end
        	end
	end
	until true
    end	
	return sentenced_empty(poitype_new, "rule_filter")	
end

local function poiID_filter( poitype_table, roadID, accountID )
    local ok1,roadID1 = luakv_api.cmd('owner', accountID, "get", accountID .. ":roadID1" )
    local ok2,roadID2 = luakv_api.cmd('owner', accountID, "get", accountID .. ":roadID2" )
    if not roadID1 and not roadID2 then
	luakv_api.cmd('owner', accountID, 'set', accountID .. ":roadID1",roadID )
	roadID1 = roadID 
	return sentenced_empty(poitype_table, "poiID_filter",roadID1)	
    end

    for poitype,value in pairs( poitype_table or {}) do
	repeat
    	if not conf["POI_LIST"][poitype] then
		poitype_table[poitype] = nil
 		break
    	end
        local issued_Type = conf["POI_LIST"][poitype]["issued_Type"]
        if issued_Type == "1" then
	    local poiID = value["poiID"]	
            local ok1,value1 = luakv_api.cmd('owner', accountID, "sismember", accountID .. ":" .. roadID1, poiID )
            local ok2,value2 = luakv_api.cmd('owner', accountID, "sismember", accountID .. ":" .. ((roadID2 and roadID2) or roadID1), poiID )
            if value1 == 1 or value2 == 1  then
                poitype_table[poitype] = nil
            end
        end
        if issued_Type == "2" then
	    for k=#value,1,-1 do
            	local poiID = value[k]["poiID"]
            	local ok1,value1 = luakv_api.cmd('owner', accountID, "sismember", accountID .. ":" .. roadID1, poiID )
            	local ok2,value2 = luakv_api.cmd('owner', accountID, "sismember", accountID .. ":" .. ((roadID2 and roadID2) or roadID1), poiID )
               	if value1 ==1 or value2 == 1 then
			table.remove(value,k)
               	end
 	    end
		if not next(poitype_table[poitype]) then
			poitype_table[poitype] = nil
		end
        end
	until true
    end
	local ok_old,oldroadID = luakv_api.cmd('owner', accountID, 'get', accountID .. ":oldroadID" )
	if not oldroadID then
		luakv_api.cmd('owner', accountID, 'set', accountID .. ":oldroadID",roadID )
		oldroadID = roadID
	end
	if oldroadID ~= roadID then
		luakv_api.cmd('owner', accountID, 'set', accountID .. ":oldroadID",roadID )
		local ok,count = luakv_api.cmd('owner', accountID, 'get', accountID .. ":count")
		if not count then
			count = 0
		end
		count = count +1
		luakv_api.cmd('owner', accountID, 'set', accountID .. ":count",count )
		if count ==  3 then
			luakv_api.cmd('owner', accountID, 'set', accountID .. ":roadID1",roadID )
			luakv_api.cmd('owner', accountID, 'set', accountID .. ":roadID2",roadID1 )
			luakv_api.cmd('owner', accountID, 'set', accountID .. ":count", 0 )
			if roadID2 then
				luakv_api.cmd('owner', accountID, 'del', accountID .. ":roadID2" )
			end
		end
	end
	return sentenced_empty(poitype_table, "poiID_filter", roadID1)	
end
local function frequency_filter( poitype_table, accountID )
    local issued_poitype_table = {}
    local ok, time = luakv_api.cmd('owner', accountID, "get",accountID .. ":today")
    local today =os.date('%x',os.time())
    for poitype, value in pairs(poitype_table or {} ) do
        local poi_idx = nil
        local i = tonumber(conf["POI_LIST"][poitype]["random_numbers"])
        local issued_Type = conf["POI_LIST"][poitype]["issued_Type"]
	if i > 0 then
		if tostring(poitype) == "1029" then
            		if time and time == today then
                		poitype_table[poitype] = nil
            		else
            			poi_idx = "0" .. ( os.time() % i  + 1)
            			issued_poitype_table [tonumber(poitype .. poi_idx)] =  value
            			poitype_table [poitype] = nil
            			issued_poitype_table [tonumber(poitype .. poi_idx)]["context"] = conf["POI_LIST"][poitype]["context"]
            			luakv_api.cmd('owner', accountID, "set", accountID .. ":today", today)
			end
		else
            		poi_idx = "0" .. ( os.time() % i  + 1)
            		issued_poitype_table [tonumber(poitype .. poi_idx)] =  value
            		poitype_table [tonumber(poitype)] = nil
            		issued_poitype_table [tonumber(poitype .. poi_idx)]["context"] = conf["POI_LIST"][poitype]["context"]
        	end
	end
	if i == 0 then
		if issued_Type == "2"then 	
            		issued_poitype_table[poitype] = value
            		poitype_table [poitype] = nil
            		issued_poitype_table [poitype]["context"] = conf["POI_LIST"][poitype]["context"]
        	end
	end
    end
		only.log('D',"freq_filter")
		only.log('D',scan.dump(issued_poitype_table))
    return issued_poitype_table
end
local function chose(issued_poitype_table,poitype)
	local a = issued_poitype_table[poitype][1]["type"]
end
local function add_attributes(issued_poitype_table, lon, lat, direction)
    local poitype_conf = 0
    for poitype,value in pairs(issued_poitype_table or {} ) do
	local status,err = pcall(chose,issued_poitype_table,poitype)
	if status then
	        poitype_conf = issued_poitype_table[poitype][1]["type"]
	else
        	poitype_conf = issued_poitype_table[poitype]["type"] 
	end
        poitype_conf = tonumber(poitype_conf)
	only.log('D',poitype_conf)
        local issued_Type = conf["POI_LIST"][poitype_conf]["issued_Type"]
	if issued_Type == "1" then
        	local poi_direction = issued_poitype_table[poitype]["direction"]
        	local poi_lon = issued_poitype_table[poitype]["longitude"]
        	local poi_lat = issued_poitype_table[poitype]["latitude"]
        	if  poi_direction then
            		issued_poitype_table [poitype]["receiverDirection"] = string.format('[%s,45]', poi_direction)
        	else
            		only.log('E',"poitype data is error")
            		return false
        	end 
		        issued_poitype_table [poitype]["senderLongitude"] = lon
		        issued_poitype_table [poitype]["senderLatitude"] = lat
        issued_poitype_table [poitype]["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
        issued_poitype_table [poitype]["senderSpeed"] =  string.format('[%s]',speed and math.ceil(speed) or 0)

        issued_poitype_table [poitype]["receiverLongitude"] = poi_lon
        issued_poitype_table [poitype]["receiverLatitude"] = poi_lat
        issued_poitype_table [poitype]["receiverDistance"] = conf["POI_LIST"][poitype_conf]["receiver_dis"]
        issued_poitype_table [poitype]["interval"] = conf["POI_LIST"][poitype_conf]["interval"]
        issued_poitype_table [poitype]["level"] =  conf["POI_LIST"][poitype_conf]["level"] or 35

	end
	if issued_Type == "2" then
		for i = 1,#value do
        		local poi_direction = issued_poitype_table[poitype][i]["direction"]
	        	local poi_lon = issued_poitype_table[poitype][i]["longitude"]
	        	local poi_lat = issued_poitype_table[poitype][i]["latitude"]
	        	if  poi_direction then
        	    		issued_poitype_table [poitype][i]["receiverDirection"] = string.format('[%s,45]', poi_direction)
	        	else
        	    		only.log('E',"poitype data is error")
            			return false
	        	end 
	        issued_poitype_table [poitype][i]["senderLongitude"] = lon
        issued_poitype_table [poitype][i]["senderLatitude"] = lat
        issued_poitype_table [poitype][i]["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
        issued_poitype_table [poitype][i]["senderSpeed"] =  string.format('[%s]',speed and math.ceil(speed) or 0)

        issued_poitype_table [poitype][i]["receiverLongitude"] = poi_lon
        issued_poitype_table [poitype][i]["receiverLatitude"] = poi_lat
        issued_poitype_table [poitype][i]["receiverDistance"] = conf["POI_LIST"][poitype_conf]["receiver_dis"]
        issued_poitype_table [poitype][i]["interval"] = conf["POI_LIST"][poitype_conf]["interval"]
        issued_poitype_table [poitype][i]["level"] =  conf["POI_LIST"][poitype_conf]["level"] or 35

		end
	end
    end
		only.log('D',"add_attributes")
		only.log('D',scan.dump(issued_poitype_table))
    return issued_poitype_table
end


function handle()
	-->> 获得当前GPS数据
	local accountID = supex.get_our_body_table()["accountID"]
	local lon       = supex.get_our_body_table()["longitude"][1]
	local lat       = supex.get_our_body_table()["latitude"][1]
	local speed     = supex.get_our_body_table()["speed"] and supex.get_our_body_table()["speed"][1] or 0
	local direction = supex.get_our_body_table()["direction"][1]
	if not lon or not lat then
		only.log('D',"gps data is error,because lon or lat is nil")
		return false
	end
	-->> 调用点在路上定位
	local ok, result = redis_api.cmd('match_road', '', 'hmget', 'LOCATE', lon, lat, direction)
	if not ok then
		only.log('E',"get road data is fail")
		return false
	end
	local RRID = result[1]
	local SGID = result[2]
	if not RRID or not SGID then
		return false
	end
	only.log('D',RRID)
	only.log('D',SGID)
	local poi_table = {}

	-->> lua调c接口，c接口通过rrid和sgid 返回前方的poi
	poi_table = getpoi(RRID,SGID)
	if not poi_table then
		return false
	end
	local roadID    = RRID
	local roadID1 = 0
	-->> space filter  1、poiid的方向角与当前方向角做比较，大于六十度过滤  2、把前方四公里poiID table转化为poitype table
	local poitype_table = space_filter( poi_table,direction )
	if not poitype_table then
		return false
	end

	-->> rule filter 1同一poitype下发距离最远的,限速摄像 违章摄像除外 2、
	poitype_table = rule_filter(poitype_table, lon, lat)
	if not poitype_table then
		return false
	end

	-->> 把同一roadID上下发过的poitype过滤
	poitype_table,roadID1 = poiID_filter( poitype_table, roadID, accountID )
	if not poitype_table then
		return false
	end

	-->> time filter   1、特定poitype进行时间段的过滤   
	poitype_table = time_filter( poitype_table )
	if not poitype_table then
		return false
	end

	-->> frequency filter 1、对于频率有要求的进行过滤（现在只有加油站为一天一次需要过滤）2、准备下发语音需要poitype 3、产生下发的列表 
	local issued_poitype_table = {}
	issued_poitype_table = frequency_filter( poitype_table,accountID  )
		if not next (issued_poitype_table) then
			only.log('D',"table is nil------------------ 289")
			return false
		end
	-->> 增加poitype下发属性
	issued_poitype_table = add_attributes( issued_poitype_table, lon, lat, direction )

	-->> 将下发的poiid存入缓存中
	for poitype,value in pairs( issued_poitype_table or {}) do
	
		local status,err = pcall(chose,issued_poitype_table,poitype)
		if status then
	      		poitype_conf = issued_poitype_table[poitype][1]["type"]
		else
        		poitype_conf = issued_poitype_table[poitype]["type"] 
		end
		poitype_conf = tonumber(poitype_conf)
		local issued_Type = conf["POI_LIST"][poitype_conf]["issued_Type"]
		if issued_Type == "1" then
			local poiID = value["poiID"]
			luakv_api.cmd('owner', accountID, "sadd", accountID .. ":" .. roadID1, poiID)
		end
		if issued_Type == "2" then
			for k,v in ipairs( value ) do
				local poiID = v["poiID"]
				only.log('E',"poiID:" .. poiID)
				luakv_api.cmd('owner', accountID, "sadd", accountID .. ":" .. roadID1, poiID )
			end
		end
	end
	
	-->> 前方四公里产生私有数据，通过http携带数据

	local issued_list = {}
	for poitype, v in pairs( issued_poitype_table ) do
		-->统计前方四公里work次数
		pcall(COUNT.count_times,poitype)
		local poitype_string = tostring( poitype )
		issued_list[poitype_string] = v
		issued_poitype_table[poitype] = nil
	end
	scene.push( "l_f_fetch_4_miles_ahead_poi", issued_list )
	return true
end
