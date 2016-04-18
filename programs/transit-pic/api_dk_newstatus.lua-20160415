-- ngx_newstatus.lua
-- auth: jiang z.s 
-- 2015-06-03
-- 补充功能
-- 2015-05-10 lru cache  
-- 2015-06-03 adtalk 

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local socket    = require('socket')
local dk_utils  = require('dk_utils')
local mysql_api = require('mysql_pool_api')
local redis_api = require('redis_pool_api')

local app_newstatus_db = "app_ns___newStatus"

local MATH_FLOOR = math.floor
local STR_LEN    = string.len

local IS_NUMBER = utils.is_number
local IS_FLOOR  = utils.is_float
local IS_WORD   = utils.is_word
local IS_INTEGER   = utils.is_integer



local MAX_TIME_UPDATE = 10800  ---最大补传时间为1个小时
local TIME_DIFF = 28800    -- 8*60*60

local G = {
	imei          = '',
	head          = '',
	ip            = '',
	body          = '',

	cur_gmtdate  = 0,	----280614(2014-06-28 GM当前时间,用来纠正异常数据的)
	cur_month     = 0,	----当前月份(201404)
	cur_time      = 0, 	----当前时间(时间戳)
	cur_accountid = '',  ----accountID or args['imei']


	sql_save_gps_info             = "INSERT INTO GPSInfo_%s(accountID, imei, imsi, createTime, collectTime, GMTTime, longitude, latitude, direction, speed, altitude, tokenCode) values %s ",

}




-- local tab_to_map
local tab_GPSTime
local tab_longitude
local tab_latitude
local tab_direction
local tab_speed
local tab_altitude

---- gps补传转发
local other_GPSTime
local other_direction
local other_speed
local other_altitude
local other_longitude
local other_latitude

local EW = {
	E = "+",
	W = "-",
}
local EW_NUMBER = {
	E = 1,
	W = -1,
}

local NS = {
	N = "+",
	S = "-",
}

local NS_NUMBER = {
	N = 1,
	S = -1,
}

module("api_dk_newstatus", package.seeall)


local function check_parameter( args )
	--> check imei
	if (not IS_NUMBER(args["imei"])) or (STR_LEN(args["imei"]) ~= 15) or (string.find(args["imei"], "0") == 1) then
		only.log('E', "imei is error! %s" , args['imei'])
		return false
	end
	--> check imsi
	if (not IS_NUMBER(args["imsi"])) or (STR_LEN(args["imsi"]) ~= 15) then
		only.log('E', "imsi is error! %s", args['imsi'])
		return false
	end
	--> check mod
	if (not IS_WORD(args["mod"])) or (STR_LEN(args["mod"]) > 5) then
		only.log('E', "mod is error! %s ", args['mod'] )
		return false
	end

	--> check tokencode
	if (not IS_WORD(args["tokencode"])) or (STR_LEN(args["tokencode"]) ~= 10) then
		only.log('E', "tokencode is error!  %s ", args['tokencode'])
		return false
	end

	--> check other
	if args["other"] and (STR_LEN(args["other"]) > dk_utils.MAX_OTHER_LEN  ) then
		only.log('E', "other too long! ")
		return false
	end
	return true
end


local function verify_parameter( args )
	local ok, tokencode = redis_api.only_cmd('saveTokencode','get',args["imei"] .. ":tokenCode")
	if not ok then
		return false
	end
	if not tokencode then
		if args['mod'] == 'SG900' or args['mod'] == 'TG900' then
			only.log('W', "%s get tokencode=nil from redis! maybe tokencode over 48h -->--- 666 ", args["imei"] )
			return false
		end

		--非WEME用户开机超过48小时，返回667
		only.log('W', "%s get tokencode=nil from redis! maybe tokencode over 48h -->--- 667 ", args["imei"] )
		return false
	end

	if tokencode ~= args["tokencode"] then
		only.log('E', " IMEI: %s  redis tokencode  [%s] != post tokencode [%s] ",args["imei"], tokencode, args["tokencode"])
		---- tokenCode无效,返回666
		return false
	end

	return true
end

local function parse_data(data)
	if not data or not string.find(tostring(data),",") then return nil end
	if not string.find(tostring(data),";") then
		local field_tab = {}
		field_tab[1] = utils.str_split(data, ',')
		return field_tab
	end
	local field_tab = {}
	local group_tab = utils.str_split(data, ';')
	for k, v in pairs(group_tab) do
		field_tab[ k ] = utils.str_split(v, ',')
	end
	return field_tab
end

local function lg(name, value)
	only.log('E',  ' %s = %s is error data!', name, tostring(value) ) 
end

local function gmttime_to_time(gmttime)
	return os.time({day=tonumber(string.sub(gmttime, 1, 2)), month=tonumber(string.sub(gmttime, 3, 4)), year=G.cur_year + tonumber(string.sub(gmttime, 5, 6)), hour=tonumber(string.sub(gmttime, 7, 8)), min=tonumber(string.sub(gmttime, 9, 10)), sec=tonumber(string.sub(gmttime, 11, 12))})
end


local function parse_gps_data( gps_table )
	if not gps_table or #gps_table ~= 6 then return false end

	---- 可能出现,,,-1,的数据 2014-09-26
	if #gps_table[1] < 1 or #gps_table[2] < 1  or #gps_table[3] < 1  or #gps_table[4] < 1  or #gps_table[5] < 1  or #gps_table[6] < 1 then
		only.log('E', "gps data is Exp: %s",table.concat(gps_table,",")) 
		return false
	end 

	local char_long = string.sub(gps_table[2],-1)
	---- 2014-10-22 需要考虑异常情况
	if not EW_NUMBER[ char_long ] then
		only.log('E', "longitude: %s  char_longis error",gps_table[2]  ) 
		return false
	end

	local save_info = {}

	save_info["longdir"] = EW[ char_long ]
	local char_lat = string.sub(gps_table[3],-1)

	---- 2014-10-22 需要考虑异常情况
	if not NS_NUMBER[ char_lat ] then
		only.log('E', "longitude: %s  char_lat is error",gps_table[3] )
		return false
	end
	save_info["latdir"] = NS[ char_lat ]

	--long lat
	local raw_long = tonumber( string.sub(gps_table[2], 1, -2) )
	local raw_lat = tonumber( string.sub(gps_table[3], 1, -2) )

	----经纬度转换失败的情况 2014-10-13
	if not raw_long or not raw_lat then
		only.log('E',  "longitude: %s  latitude: %s is error",gps_table[2], gps_table[3] ) 
		return false
	end

	local tmp_longitude,tmp_latitude = dk_utils.convert_gps( raw_long, raw_lat )

	save_info["longitude"] = EW_NUMBER[ char_long ] * tmp_longitude
	save_info["latitude"] = NS_NUMBER[ char_lat ] * tmp_latitude
	-- table.insert(tab_longitude, EW_NUMBER[ char_long ] * save_info["longitude"])
	-- table.insert(tab_latitude, NS_NUMBER[ char_lat ] * save_info["latitude"])

	--GMT
	local GMTtime = gps_table[1]
	if IS_NUMBER(GMTtime) and STR_LEN(GMTtime) == 11 then
		----GMTTime 错误,丢失第一位,做容错处理[;20514135752]
		if string.sub(GMTtime,1,5) == string.sub(G.cur_gmtdate,2,6) then
			----发现日期相同,需要修复
			GMTtime = string.format("%s%s",string.sub(G.cur_gmtdate,1,1),GMTtime)
		end
	end
	if (not IS_NUMBER(GMTtime)) or (STR_LEN(GMTtime) ~= 12) then lg("GMT", GMTtime) return false end

	local time_stamp = gmttime_to_time(GMTtime)

	---- 最大补传时间为3个小时, 可能大于当前时间,也可能小于当前时间 2014-09-24
	if  math.abs(tonumber(os.time()) - tonumber(time_stamp + TIME_DIFF ))  < MAX_TIME_UPDATE then
		---- 小于1小时内的数据不修正
		save_info["collectTime"] = time_stamp + TIME_DIFF
	else

		-- save_info["collectTime"] = os.time()
		only.log('E',  " %s GMTtime is ERROR data:%s===", G.imei, GMTtime) 
		return false
	end

	-- table.insert(tab_GPSTime, save_info["collectTime"])

	--direction
	save_info["direction"] = MATH_FLOOR(gps_table[4])
	-- table.insert(tab_direction, save_info["direction"])

	--speed
	save_info["speed"] = MATH_FLOOR(gps_table[5])
	-- table.insert(tab_speed, save_info["speed"])

	--altitude
	save_info["altitude"] = MATH_FLOOR(gps_table[6])
	-- table.insert(tab_altitude, save_info["altitude"])

	-------------debug------------
	if not save_info["longdir"] or not char_long then lg("longdir", string.sub(gps_table[2],-1)) return false end
	if not save_info["latdir"] or not char_lat then lg("latdir", string.sub(gps_table[3],-1)) return false end
	if (not IS_FLOOR(raw_long)) or (STR_LEN(raw_long) > 11) then lg("raw_long", raw_long) return false end
	if (tonumber(save_info["longitude"]) < 0) or (tonumber(save_info["longitude"]) > 180) then lg("longitude", save_info["longitude"]) return false end
	if (not IS_FLOOR(raw_lat)) or (STR_LEN(raw_lat) > 11) then lg("raw_lat", raw_lat)  return false end
	if (tonumber(save_info["latitude"]) < 0) or (tonumber(save_info["latitude"]) > 90) then lg("latitude", save_info["latitude"]) return false end
	if (not IS_FLOOR(save_info["direction"])) or (STR_LEN(save_info["direction"]) > 6) or ((tonumber(save_info["direction"]) < 0) and (tonumber(save_info["direction"]) ~= -1)) or (tonumber(save_info["direction"]) > 360) then lg("direction", save_info["direction"]) return false end
	if (not IS_FLOOR(save_info["speed"])) or (STR_LEN(save_info["speed"]) > 7) or (tonumber(save_info["speed"]) < 0) or (tonumber(save_info["speed"]) > 10000) then lg("speed", save_info["speed"]) return false end
	if (not IS_INTEGER(save_info["altitude"])) or (STR_LEN(save_info["altitude"]) > 6) or (tonumber(save_info["altitude"]) < -9999) or (tonumber(save_info["altitude"]) > 99999) then lg("altitude", save_info["altitude"]) return false end

	return true,save_info
end


---- 拼装mysql,保存gps数据至数据库
---- gps_table 最基本的数据
---- accountid 用户帐号
---- imei
---- imsi 
---- token_code 
---- tab_gpsinfo   当前解析成功返回的数据
---- is_extragps 当前是否为补传gps数据,默认为false 

local function get_gps_sql( gps_table, accountid, imei,  imsi , token_code ,  is_extragps , ret_gpsinfo_tab  )
	if not gps_table then return false end

	local create_time = G.cur_time
	local tb_gps_sql = {}

	ret_gpsinfo_tab['imei']      = imei
	ret_gpsinfo_tab['count']     = 0	
	ret_gpsinfo_tab['accountID'] = accountid
	ret_gpsinfo_tab['list']      = {}
        

	for i=1,#gps_table do
		local ok , save_info = parse_gps_data( gps_table[i] )
		if ok and save_info then
			if not is_extragps then
				table.insert(tab_GPSTime,save_info['collectTime'])
				table.insert(tab_direction,save_info["direction"])
				table.insert(tab_speed,save_info["speed"])
				table.insert(tab_altitude,save_info["altitude"])
				table.insert(tab_longitude,save_info["longitude"])
				table.insert(tab_latitude,save_info["latitude"])
			else
				table.insert(other_GPSTime,save_info['collectTime'])
				table.insert(other_direction,save_info["direction"])
				table.insert(other_speed,save_info["speed"])
				table.insert(other_altitude,save_info["altitude"])
				table.insert(other_longitude,save_info["longitude"])
				table.insert(other_latitude,save_info["latitude"])
			end

			if ret_gpsinfo_tab['count'] == 0 then
				ret_gpsinfo_tab['collectStart'] = save_info['collectTime']
			end

			local t_GMTTime = gps_table[i][1]
			if IS_NUMBER(t_GMTTime) and STR_LEN(t_GMTTime) == 11 then
				----GMTTime 错误,丢失第一位,做容错处理[;20514135752]
				if string.sub(t_GMTTime,1,5) == string.sub(G.cur_gmtdate ,2,6) then
					----发现日期相同,需要修复
					t_GMTTime = string.format("%s%s",string.sub(G.cur_gmtdate,1,1),t_GMTTime)
				end
			end

			if tonumber(dk_utils.DK_WRITE_MYSQL_WITH_GPS) ~= 0 then
				---- gps是否写mysql
				local sqlgps = string.format("('%s', '%s', '%s' , %d , %s, '%s', %s%s, %s%s, %s, %s, %s, '%s')",
				accountid, imei, imsi,
				create_time, save_info["collectTime"], t_GMTTime , save_info["longdir"],
				MATH_FLOOR(save_info["longitude"] * dk_utils.gps_point_unit ), 
				save_info["latdir"],
				MATH_FLOOR(save_info["latitude"] * dk_utils.gps_point_unit ),
				save_info["direction"], 
				save_info["speed"], 
				save_info["altitude"], 
				token_code)
				table.insert(tb_gps_sql, 1, sqlgps)
			end

			-----数据库保存的格式：
			---- key: GPS:${imei}:${time interval}
			---- value: 600*12@ ${collectTime}|${createTime}|${imei}|${accountID}|${imsi}|${GMTTime}|${longitude}|${latitude}|${altitude}|${direction}|${speed}|${tokenCode}|...
			---- 存储至redis
			table.insert(ret_gpsinfo_tab.list,string.format("%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s",
			save_info['collectTime'], create_time, imei, accountid or "", 
			imsi,  t_GMTTime,
			MATH_FLOOR(save_info["longitude"] * dk_utils.gps_point_unit ),
			MATH_FLOOR(save_info["latitude"] * dk_utils.gps_point_unit ), 
			save_info["altitude"], 
			save_info['direction'], 
			save_info['speed'], 
			token_code))

			ret_gpsinfo_tab.count = ret_gpsinfo_tab.count + 1 

		end
	end
	---- gps data maybe ;;;;
	if ret_gpsinfo_tab.count < 1 then return false end

	---- 直接返回
	if tonumber(dk_utils.DK_WRITE_MYSQL_WITH_GPS) ~= 0 then 
		return string.format(G.sql_save_gps_info, G.cur_month , table.concat(tb_gps_sql, ",")) 
	end

end



function split_data_with_gps( accountid, gps , imei, imsi, tokencode  )
	local tab_gpsinfo     = {}		----经纬度tab
	local gps_table = parse_data( gps )
	if type(gps_table) == "table" then
		local sql_gps = get_gps_sql(gps_table, accountid or '', imei , imsi , tokencode , false, tab_gpsinfo )
		-- gps是否写mysql 2014-09-16 
		if tonumber(dk_utils.DK_WRITE_MYSQL_WITH_GPS) ~= 0 then
			if sql_gps then
				local ok,ret = mysql_api.cmd(app_newstatus_db, "insert", sql_gps)
				if not ok then
					only.log('E',  'save gps data failed  %s ', sql_gps ) 
				end
			else
				only.log('D', "gps is error data!  %s ", gps )
			end
		end
		if tab_gpsinfo then
			dk_utils.set_gpsinfo(tab_gpsinfo,G.cur_time)
		end
	else
		only.log('E', "gps split failed! %s ", gps ) 
	end
end



local function split_data_with_extragps( accountid, extragps, imei, imsi, tokencode )
	local gps_table = parse_data( extragps )
	if type(gps_table) == "table" then
		local tab_extragpsinfo = {}
		get_gps_sql(gps_table, accountid or '', imei , imsi , tokencode , true, tab_extragpsinfo )
		if tab_extragpsinfo then
			dk_utils.set_extragpsinfo(tab_extragpsinfo )
		end
	else
		only.log('E', "extragps split failed! %s", extragps ) 
	end
end

local function split_data_with_other( args, accountid )

	if not args["other"] then return false end
	local tmp_tab = utils.str_split(args["other"],"|")
	if not tmp_tab or #tmp_tab < 1  then return false end

	for i,val in pairs(tmp_tab) do
		if val and (string.find(tostring(val),"extragps:")) == 1  then
			----gps补传 
			local tmp_other_str = string.sub(val,10,#val)
			split_data_with_extragps(accountid, tmp_other_str, args["imei"], args['imsi'] , args['tokencode'] )
		else
			----xxxxx
		end
	end
end


function handle( str )
	str = string.gsub(str,"keyvalue\r\n","")
	G["body"] = str
	--local args = cjson.decode(G["body"])
	local args = utils.parse_url(G["body"])
	-- only.log('D',scan.dump(args))
        only.log('D',"body = %s",G.body)
	------
	local cur_date = os.date('%Y_%m_%d')

	G.cur_month = string.gsub(string.sub(cur_date,1,7),"%_","")
	G.cur_time = os.time()


	local cur_today = string.gsub(cur_date,"%_","")
	G.cur_date = cur_today

	-- 280614   2014-06-28
	G.cur_gmtdate = string.format("%s%s%s",string.sub(cur_date,9,10), string.sub(cur_date,6,7), string.sub(cur_date,3,4) )
	G.cur_year = tonumber(string.sub(cur_date,1,2)) * 100
	------


	G.imei = args['imei']
	if  not args['accountID'] or tostring(args['accountID']) == '' then
		G.cur_accountid = G.imei
	else
		G.cur_accountid = args['accountID']
	end

	local ok = check_parameter( args )
	if not ok then
		----- 日志写ts_db
		dk_utils.set_urlLog(G.imei,G.body,G.ip,400,G.cur_time)
		return false
	end

	local ok = verify_parameter( args )
	if not ok then
		----- 日志写ts_db
		dk_utils.set_urlLog(G.imei,G.body,G.ip,400,G.cur_time)
		return false
	end

	only.log('D',"newstatus Notice log imei:%s  imsi:%s  tokencode:%s " , args['imei'], args['imsi'], args['tokencode'])
--[[
	local accountid = dk_utils.lru_cache_get_accountid(G.imei)
	if #tostring(accountid) == 10 then
		G.cur_accountid = accountid
	else
		G.cur_accountid = G.imei
	end
]]--
	---- 可能中途掉线
	---- 避免操作redis过于频繁
	--------------------------------------------------
	-------------debug------------
	if G.cur_time % 2 == 0 then
		redis_api.only_cmd('statistic', 'sadd', 'onlineUser:set', G.cur_accountid )
		redis_api.only_cmd('private', 'set', G.cur_accountid .. ":heartbeatTimestamp", G.cur_time )
	end


	-->| STEP 6 |<--
	-->> reset table
	local tab_jo              = {}
	tab_jo['collect']   = true
	tab_jo["accountID"] = G.cur_accountid
	tab_jo["IMEI"]      = args["imei"]
	tab_jo["tokenCode"] = args["tokencode"]
	tab_jo["model"]     = args["mod"]


	tab_GPSTime         = {}
	tab_longitude       = {}
	tab_latitude        = {}
	tab_direction       = {}
	tab_speed           = {}
	tab_altitude        = {}

	other_GPSTime = {}
	other_direction = {}
	other_speed = {}
	other_altitude = {}
	other_longitude = {}
	other_latitude = {}
	
	-->> 保存GPS至数据库
	if args["gps"] then
		split_data_with_gps( accountid , args["gps"] ,  args["imei"], args['imsi'] , args['tokencode'] )
	end

	if args['other'] then
		split_data_with_other(args, accountid)
	end

	---- 用户当前最新的经纬度
	local cur_longitude = nil
	local cur_latitude = nil

	----  数据转发 
	if tab_GPSTime and #tab_GPSTime > 0 then

		if #tab_longitude > 0 then
			cur_longitude = tab_longitude[1]
		end

		if #tab_latitude > 0 then
			cur_latitude = tab_latitude[1]
		end

		tab_jo["GPSTime"]	= tab_GPSTime
		tab_jo["longitude"]	= tab_longitude
		tab_jo["latitude"]	= tab_latitude
		tab_jo["direction"]	= tab_direction
		tab_jo["speed"]		= tab_speed
		tab_jo["altitude"]	= tab_altitude
		if (other_GPSTime and #other_GPSTime > 0 ) then
			---- gps补传 2015-04-27 
			tab_jo['extragps'] = {}
			tab_jo['extragps']["GPSTime"]   = other_GPSTime
			tab_jo['extragps']["longitude"] = other_longitude
			tab_jo['extragps']["latitude"]  = other_latitude
			tab_jo['extragps']["direction"] = other_direction
			tab_jo['extragps']["speed"]     = other_speed
			tab_jo['extragps']["altitude"]  = other_altitude
		end

		dk_utils.post_data_to_other(tab_jo, args)
	end


	----- 日志写ts_db
	dk_utils.set_urlLog(G.imei,G.body,G.ip,200,G.cur_time)
	return true
end
