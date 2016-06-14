local utils     = require('utils')
local only      = require('only')
local redis_api = require('redis_pool_api')
local supex     = require('supex')
local scan      = require('scan')

module('api_dc_newstatus', package.seeall)

--变量定义
--req_field_tab:请求中各字段的value封装的table
--dot_list     :GPS数据中每个坐标点属性解析并进行字符串拼接后封装的table
--trans_tab    :请求数据完全解析后待转发的table
local req_field_tab  = {}
local dot_list       = {}
local trans_tab      = {}

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
--由于请求按照顺序描述，返回的value_tab的字段按索引顺序排列为：
--value_tab[1]:M value_tab[2]:A value_tab[3]:T value_tab[4]:B value_tab[5]:G
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

function handle(msg)
	--local data = supex.get_our_info_data()
	--msg = string.sub(data,8)
	only.log('S','msg = %s', msg)
	local key_log = string.format('newstatuslog:%s',os.date('%Y%m%d',time))
	local value =  string.format('%s  %s',os.date('%Y-%m-%d %H:%M:%S',time),msg)
	redis_api.cmd('newstatusRedis','','LPUSH', key_log, value)
	
	--解析请求中的msg数据并将各字段的value封装至table中
	local msg_tab = utils.str_split(msg, "&")
	only.log('D', 'msg_tab = %s', scan.dump(msg_tab))
	local req_field_tab = extract_val_from_msg(msg_tab)
	only.log('D', 'req_field_tab = %s', scan.dump(req_field_tab))
		
	--提取GPS各属性数据，进行格式转换等数据解析
	local gpsdata = req_field_tab[#req_field_tab]
	only.log('D', 'gpsdata = %s', gpsdata)
	parse_gpsdata(gpsdata, dot_list)
	only.log('D', 'dot_list = %s', scan.dump(dot_list))

	--组装已经解析的信息待转发	
	compack_info_tab(trans_tab, req_field_tab, dot_list)
	only.log('D', 'trans_tab = %s', scan.dump(trans_tab))
end

