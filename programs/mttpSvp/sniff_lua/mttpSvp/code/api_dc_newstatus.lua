local utils     = require('utils')
local only      = require('only')
local redis_api = require('redis_pool_api')
local supex     = require('supex')
local scan      = require('scan')
local cjson     = require('cjson')
local utils     = require('utils')

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
        for k, v in pairs(group_tab) do
                field_tab[ k ] = utils.str_split(v, ',')
        end
        return field_tab
end
--解析GPS数据并且进行拼接
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
local function organize_data(org_table,gps_table,data_table)
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

function handle(msg)
	--变量定义
	--req_field_tab:请求中各字段的value封装的table
	--dot_list     :GPS数据中每个坐标点属性解析并进行字符串拼接后封装的table
	--trans_tab    :请求数据完全解析后待转发的table
	local req_field_tab  = {}
	local dot_list       = {}
	local trans_tab      = {}
	only.log('S','msg = %s', msg)
	local key_log = string.format('newstatuslog:%s',os.date('%Y%m%d',time))
	local value =  string.format('%s  %s',os.date('%Y-%m-%d %H:%M:%S',time),msg)
	redis_api.cmd('newstatusRedis','','LPUSH', key_log, value)
	local isfind = string.find(msg,'TYPE=2')
	local msg_str = string.gsub(msg,"&TYPE.+$","")
	--解析请求中的msg数据并将各字段的value封装至table中
	if isfind then
		--[[local msg_tab = utils.str_split(msg_str, "&")
		only.log('D', 'msg_tab = %s', scan.dump(msg_tab))
		local req_field_tab = extract_val_from_msg(msg_tab)
		only.log('D', 'req_field_tab = %s', scan.dump(req_field_tab))
		
		--提取GPS各属性数据，进行格式转换等数据解析
		local gpsdata = req_field_tab[#req_field_tab]
		]]
		local org_table = utils.parse_url(msg_str)
		only.log('D', 'org_tab = %s',scan.dump(org_table))

		local gps_table = parse_data( org_table['G'] )
		local data_table = {}
		data_table["GPSTime"] = {}
		data_table["longitude"] = {}
		data_table["latitude"] = {}
		data_table["direction"] = {}
		data_table["speed"]  = {}
		data_table["altitude"] = {}
		organize_data(org_table,gps_table, data_table)
		data_table['M'] = org_table['M'] 
		data_table['collect'] = 'true'
		
		--parse_g:psdata(gpsdata, dot_list)
		--only.log('D', 'dot_list = %s', scan.dump(dot_list))

		--组装已经解析的信息待转发	
		--compack_info_tab(trans_tab, req_field_tab, dot_list)
		local ok,MAppKeyInfo = redis_api.cmd('dcRedis','','get', 'MAppKeyInfo:'..org_table['M'])
		local ok,tokenCode = redis_api.cmd('dcRedis','','get', 'dcToken:'..org_table['M'])
		
		if MAppKeyInfo and tokenCode then
			data_table['tokenCode'] = tokenCode
			data_table['firmKey'] = MAppKeyInfo
		end
		local data_str = cjson.encode(data_table)
		only.log('D', 'trans_str =%s',data_str )
	end
end

