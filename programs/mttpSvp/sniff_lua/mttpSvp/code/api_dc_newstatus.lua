local utils     = require('utils')
local only      = require('only')
local redis_api = require('redis_pool_api')
local supex     = require('supex')
local scan      = require('scan')
local cjson     = require('cjson')
local cutils    = require('cutils')


module('api_dc_newstatus', package.seeall)

local function parse_data(data)
        if not data or not string.find(tostring(data),",") then return nil end
        if not string.find(tostring(data),";") then
                local field_tab = {}
                field_tab[1] = utils.str_split(data, ',')
                return field_tab
        end
        local field_tab = {}
        local group_tab = utils.str_split(data, ';')
        for k, v in ipairs(group_tab) do
                field_tab[ k ] = utils.str_split(v, ',')
        end
        return field_tab
end

local function parse_gpsdata(gpsdata, dot_list)
	local k = 1 --dot_list的索引初始值
	if not gpsdata then return nil end
	for dot_info in string.gmatch(gpsdata, "[^;]+") do --提取";"分隔的数据
		--only.log('D', 'dot_info = %s', dot_info)
		local dot_str = ""
		local i = 1 --用于经纬度数据转换
		for dot_elem in string.gmatch(dot_info, "[^,]+") do --提取","分隔的数据
			if i == 2 or i == 3 then
				local num_dot_elem = tonumber(dot_elem)
				if num_dot_elem then
					dot_elem =tostring(num_dot_elem / 1000000)
				else
					only.log('D','longitude or latitude convert failed!')
					return nil
				end
			end
			if i == 1 then
				dot_str = dot_elem
			else
				dot_str = dot_str .. string.format("|%s", dot_elem)
			end
			i = i + 1	
		end
		--only.log('D', 'dot_str = %s', dot_str)
		table.insert(dot_list, k, dot_str)
		k = k + 1
	end
--	only.log('D', 'dot_list = %s', scan.dump(dot_list))
end

--从请求传入的msg中提取字段的value封装成table
local function extract_val_from_msg(data_tab)
	value_tab = {}
	for k, elem in pairs(data_tab) do
		local falg = nil --作为只存储“=”号后元素的标志
		for elem_val in string.gmatch(elem, "[^=]+") do
			if falg then 
				value_tab[k] = elem_val
			end
			falg = 1			
		end
	end
	--only.log('D', 'value_tab = %s', scan.dump(value_tab))
	return value_tab
end
	
local function compack_info_tab(trans_tab, data_tab, list)
	trans_tab['G'] = {}
	if not type(data_tab) == 'table' or not type(list) == 'table' then
		return nil
	end
	trans_tab['M'] = data_tab[1]
	trans_tab['A'] = data_tab[2]
	trans_tab['T'] = data_tab[3]
	trans_tab['B'] = data_tab[4]
	trans_tab['G'] = list
end	

local function organize_data(org_table, gps_table, data_table)
	local timestamp = utils.gmttime_to_timestamp(org_table['B'])
	for i = 1,#gps_table do
		table.insert(data_table["GPSTime"] , tonumber(gps_table[i][1]) + timestamp)
		table.insert(data_table["longitude"] , tonumber(gps_table[i][2])/1000000)
		table.insert(data_table["latitude"] , tonumber(gps_table[i][3])/1000000)
		table.insert(data_table["direction"] , tonumber(gps_table[i][4]))
		table.insert(data_table["speed"] , tonumber(gps_table[i][5]))
		table.insert(data_table["altitude"] , tonumber(gps_table[i][6]))
	end
end

function post_data_to_other(tab_jo)
        only.log("D",scan.dump(tab_jo))
	local ok, result = pcall(cjson.encode, tab_jo)
	if ok then
		local host_info = { host = "192.168.1.12" ,port = "9002" }

		local request = utils.compose_http_json_request2( host_info ,'publicentry', nil, result)
       		only.log("D", 'composed http json request = %s', request)

		local ok, ret = redis_api.only_cmd("dcRedis","lpushx", "0|1", request)
		if ok then
       			only.log("D", 'saved redis')
		end
	end
end

function handle(msg)
	only.log('S','msg = %s', msg)

	local key_log = string.format('newstatuslog:%s',os.date('%Y%m%d',time))
	local value =  string.format('%s  %s',os.date('%Y-%m-%d %H:%M:%S',time),msg)
	redis_api.cmd('newstatusRedis','','LPUSH', key_log, value)

	local isfind = string.find(msg,'TYPE=2')
	local msg_str = string.gsub(msg,"&TYPE.+$","")
	if isfind then
		local org_table = utils.parse_url(msg_str)
		only.log('D', 'org_tab = %s', scan.dump(org_table))

		local gps_table = parse_data( org_table['G'] )
		local data_table = {}
		data_table["GPSTime"] = {}
		data_table["longitude"] = {}
		data_table["latitude"] = {}
		data_table["direction"] = {}
		data_table["speed"]  = {}
		data_table["altitude"] = {}
		organize_data(org_table, gps_table, data_table)
		data_table['mirrtalkID'] = org_table['M'] 
		data_table['accountID'] = org_table['A'] 
		data_table['collect'] = 'true'
		
--[[
		local ok,MAppKeyInfo = redis_api.cmd('dcRedis','','get', 'MAppKeyInfo:'..org_table['M'])
		local ok,tokenCode = redis_api.cmd('dcRedis','','get', 'dcToken:'..org_table['M'])
		
		if MAppKeyInfo and tokenCode then
			data_table['tokenCode'] = tokenCode
			data_table['firmKey'] = MAppKeyInfo
		end
]]--
		local data_str = cjson.encode(data_table)
		only.log('D', 'trans_str =%s',data_str )
 		post_data_to_other(data_table)
	end
end

