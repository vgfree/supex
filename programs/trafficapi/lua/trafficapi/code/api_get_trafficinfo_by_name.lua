-- Author       : tengxinghui
-- Date         : 2015-11-18
-- Function     : 

local redis = require('redis_pool_api')
local utils = require('utils')
local only = require('only')
local msg = require('api_msg')
local gosay = require('gosay')
local supex = require('supex')
local safe = require('safe')
local json = require('cjson')
local CityRef = require('CityRef')
local get_traffic_msg = require('func_get_traffic_msg')
local scan = require('scan')
local switch = require('Half_and_Full')

module("api_get_trafficinfo_by_name", package.seeall)

local Strike={
	'由东向西',
	'由西向东',
	'由南向北',
	'由北向南',
	}

local tt = {
	'畅通',
	'缓行',
	'拥堵',
	'无路况',
	'严重拥堵',
	'拥堵缓行',	
	}

local function check_parameter(args)

        if not args['uid'] or tostring(args['uid']) == '' then
                gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'uid')
		return false
        end

	if not args['longitude'] or not tonumber(args['longitude']) then
                gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'longitude')
		return false
        end 

        if not args['latitude'] or not tonumber(args['latitude']) then
                gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'latitude')
		return false
        end 

        local longitude = tonumber(args['longitude'])
        local latitude = tonumber(args['latitude'])

        if longitude < -180 or longitude > 180
                or latitude < -90 or latitude > 90
        then
                gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'longitude or latitude')
		return false
        end 

        if not args['resultType'] or not tonumber(args['resultType']) or 
		not (tonumber(args['resultType']) == 1 or tonumber(args['resultType']) == 2 or tonumber(args['resultType']) == 3)
	then
                gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'resultType')
		return false
        end

        if not args['roadName'] or tostring(args['roadName']) == '' then
                gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'roadName')
		return false
        end
	return true
end

--将合并元素插入结果集中
local function insert_traffic1(ret_traffic, last_traffic)
	last_traffic['inserted'] = 1
	table.insert(ret_traffic ,last_traffic)

	return true
end

--将合并元素插入结果集中
local function insert_traffic2(ret_traffic, last_traffic)
	if last_traffic['traffic_len'] < 500 then
		return false
	end

	last_traffic['inserted'] = 1
	table.insert(ret_traffic ,last_traffic)

	return true
end

--合并路况信息
local function merge_traffic(traffic1, traffic2, index) 
	local ret_traffic = {}
	ret_traffic['startName'] = traffic1['startName']
	ret_traffic['endName'] = traffic2['endName']
	ret_traffic['PT'] = traffic1['PT'] + traffic2['PT']
	ret_traffic['RL'] = traffic1['RL'] + traffic2['RL']
	ret_traffic['startSG'] = traffic1['startSG']
	ret_traffic['endSG'] = index
	if ret_traffic['PT'] == 0 then
		ret_traffic['AS'] = -1
	else
		ret_traffic['AS'] = math.floor((ret_traffic['RL'] / ret_traffic['PT']) * 3.6)
	end

	ret_traffic['TT'] = get_traffic_msg.compare_TT(traffic1['TT'], traffic2['TT'])

	local traffic_len = 0;
	if (traffic1['TT'] == 2 or traffic1['TT'] == 3 or traffic1['TT'] == 6) then
		traffic_len = traffic_len + traffic1['RL']
	end
	if (traffic2['TT'] == 2 or traffic2['TT'] == 3 or traffic2['TT'] == 6) then
		traffic_len = traffic_len + traffic2['RL']
	end

	ret_traffic['traffic_len'] = traffic_len
	
	--only.log('D',"traffic_len=============="..scan.dump(traffic_len))
	return ret_traffic
end

function get_last_by_traffic(seg_traffic, index, last_name)
	local ret = {}

	ret['TT'] = seg_traffic['TT']
	ret['PT'] = seg_traffic['PT']
	ret['AS'] = seg_traffic['AS']
	ret['IR'] = seg_traffic['IR']
	ret['traffic_len'] = seg_traffic['RL']
	ret['startSG'] = index
	ret['endSG'] = index
	ret['RL'] = seg_traffic['RL']
	ret['startName'] = last_name or ''
	ret['endName'] = seg_traffic['IN'] or seg_traffic['endName']

	return ret
end

function merge_traffic2(traffic1, traffic2)
	local ret = {}
	
	ret['TT'] = get_traffic_msg.compare_TT(traffic1['TT'], traffic2['TT'])
	ret['traffic_len'] = traffic1['traffic_len'] + traffic2['traffic_len']
	ret['PT'] = traffic1['PT'] + traffic2['PT']
	ret['RL'] = traffic1['RL'] + traffic2['RL']
	ret['startSG'] = traffic1['startSG']
	ret['endSG'] = traffic2['endSG']
	ret['startName'] = traffic1['startName']
	ret['endName'] = traffic2['endName']

	ret['AS'] = math.floor((ret['RL'] / ret['PT']) * 3.6)  

	--only.log('D',"retttt------->>>>>>>>>>>>>"..scan.dump(ret))

	return ret
end
--
function get_result(traffic_part1)
	local traffic_infos = traffic_part1
	--only.log('D',"------->>>>>>>>>>>>>"..scan.dump(traffic_infos))
	local ret_traffic = {}
	local last_traffic = nil
	local last_name = traffic_infos[1]['startName']


	for k,v in ipairs(traffic_infos) do
	repeat
		v['inserted'] = nil
		if not last_traffic then        --第一个路口
                        if v['TT'] ~= 4 then
                                last_traffic = v
                        end
                        last_name = v['endName']
                        break  --continue
                end
                
                local cur_TT = v['TT']
                if cur_TT == 4 then    --无路况的结束当前合并
                        insert_traffic2(ret_traffic ,last_traffic) --插入合并结果中
                        last_traffic = nil
                elseif cur_TT == 1 then
                        if last_traffic['TT'] ~= 1 then --结束当前路况合并
                                insert_traffic2(ret_traffic ,last_traffic) --插入合并结果中
                                last_traffic = v
                        else
                                last_traffic = merge_traffic2(last_traffic, v, k)
                        end
                elseif cur_TT == 2 or cur_TT == 3 or cur_TT == 6 then
                        if (last_traffic['TT'] == 1 or last_traffic['TT'] == 4) then  --结束当前路况合并
                                insert_traffic2(ret_traffic ,last_traffic) --插入合并结果中
                                last_traffic = v
                        else
                                last_traffic = merge_traffic2(last_traffic, v, k)
                        end
                end
                last_name = v['endName']

	until true
	end

	--最后一个元素也要插入
	if last_traffic and last_traffic['inserted'] ~= 1 then
		insert_traffic2(ret_traffic ,last_traffic)
	end

	return ret_traffic
end

local function merge_data(roadName,tab,roadRootID,segmentID)

	local key = string.format("%d,%d:SGInfo",roadRootID,segmentID)
        local ok, ret = redis.cmd('mapSGInfo','', 'hmget', key, 'SGSC','SGFER')
        if not ok then
                gosay.resp_msg(msg['SYSTEM_ERROR'])
		return false, false
        end 

	local traffic_infos = tab
	local ret_traffic = {}
	local last_traffic = nil
	local last_name = ret[1]
--	only.log('I',"traffic_infos" .. scan.dump(traffic_infos))

	for k,v in ipairs(traffic_infos) do
	repeat
		if not last_traffic then	--第一个路口
			last_traffic = get_last_by_traffic(v, k, last_name)
			last_name = v['IN']
			--only.log('D',"last_name"..last_name)
			if last_traffic['IR'] == 1 then
				insert_traffic1(ret_traffic ,last_traffic) --插入合并结果中
				last_traffic = nil
				break
			end
		end

		local cur_IR = v['IR']
		if cur_IR == 2 then
			last_traffic = merge_traffic(last_traffic, get_last_by_traffic(v, k, last_name), k)
		else
			last_traffic = merge_traffic(last_traffic, get_last_by_traffic(v, k, last_name), k)
			insert_traffic1(ret_traffic ,last_traffic) --插入合并结果中
			last_traffic = nil
		end
		--only.log('D',"ret_traffic+++++++++"..scan.dump(ret_traffic))
		last_name = v['IN']
	until true
	end

	--最后一个元素也要插入
	if last_traffic and last_traffic['inserted'] ~= 1 then
		insert_traffic1(ret_traffic ,last_traffic)
	end
--	only.log('D',"merge1 traffic result" .. scan.dump(ret_traffic))
	if #ret_traffic == 0 then
		return true, false
	end
	local result = get_result(ret_traffic)
--	only.log('D',"merge2 traffic result" .. scan.dump(result))
	return true, result	
end

local function get_text(tab,roadName,strike)
	
	local textInfo = {}
	for k,v in ipairs(tab) do
		local TT = tab[k]['TT']
               	local str = string.format("从%s到%s%s时速%d,", tab[k]['startName'], tab[k]['endName'],tt[TT],tab[k]['AS'])
		local str_tab = {
			['startSG'] = tab[k]['startSG'],
			['endSG'] = tab[k]['endSG'],
			['TT'] = tab[k]['TT'],
			['text'] = str,
			}
                table.insert(textInfo, str_tab)
	end
	local trafficText ={
			['trend'] = Strike[strike],
			['textInfo'] = textInfo,
			}
	return trafficText
end

local function get_cityCode(longitude,latitude)

	local gridID = string.format('%d&%d', math.floor(longitude * 100), math.floor(latitude * 100))

	local ok, ret = redis.cmd('mapGridOnePercent_v2','', 'hget', gridID,'cityCode')
	if not ok or not ret then
		only.log('E', 'Failed to get cityCode by key : %s', gridID)
		gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'longitude or latitude ')
		return false
	end
	return ret	
end

local function get_seg_msg(cityCode,cityName,roadName)

	local key = string.format("%s:%s",cityCode,roadName)

        local ok, ret = redis.cmd('roadname_search','', 'hgetall', key)
	if not ok or not ret then
		only.log('E',"fail to get data from roadname_search")
		gosay.resp_msg(msg["SYSTEM_ERROR"])
		return false
	end

	local trendInfo,trafficText = {},{}
	local ret_tab = {}
	
	local roadLength = 0
        for roadRootID,v in pairs(ret) do
		roadLength = 0
		local ok, rrid_info = pcall(json.decode,v)

		local road_info = {}
		for segmentID=1,rrid_info['sg_count'] do
                	local tab = get_traffic_msg.get_speed_info(roadRootID,segmentID,600)
			if not tab then
				return false
			end
			roadLength = roadLength + tab['RL']
			table.insert(road_info,tab)
		end
		--only.log('D',"road_info---"..scan.dump(road_info))
		local ok, ret_tab = merge_data(roadName,road_info,roadRootID,1)
		if not ok then 
			return false
		end
		if ret_tab then
			local ret_txt = get_text(ret_tab,roadName,rrid_info['strike'])
			table.insert(trafficText,ret_txt)
		end
		table.insert(trendInfo,road_info)
        end

	local value_rt
	for roadRootID, v in pairs(ret) do
		local road_tab = get_SGInfo(roadRootID, 1)
		if not road_tab and not road_tab[6] then
			only.log('E', 'Failed to get roadLevel !')
		end
		value_rt = road_tab[6]
		if tonumber(value_rt) == 0 then
			value_rt = 1
		else
			value_rt = 2
		end
		break
	end
	local road_attribute ={
		['roadName'] = roadName,
		['cityName'] = cityName,
		['roadLength'] = roadLength,
		['roadLevel'] = tonumber(value_rt)
	} 
 	
	local traffic ={
		['trendInfo'] = trendInfo,
		['trafficText'] = trafficText, 
	}

	local last_tab ={
		['traffic'] = traffic,
		['roadInfo'] = road_attribute, 
	}
	
	return last_tab
end

function handle()
	local body = supex.get_our_data()
	if not body or type(body) ~= 'string' then
		return gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'])
	end

	local args = {}
	local ret = get_traffic_msg.str_split(body, '&')
	for k, v in ipairs(ret) do
		local k_v = get_traffic_msg.str_split(v, '=')
		args[k_v[1]] = k_v[2]
	end

        if not check_parameter(args) then
		return false
	end
	--only.log('D',scan.dump(args))
	local cityCode
	if not args['cityName'] or tostring(args['cityName']) == '' then
		cityCode = get_cityCode(args['longitude'],args['latitude'])
		if not cityCode then
			return false
		end
	else 
		cityCode = CityRef.CityRefName[args['cityName']]
		if not cityCode then
			gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'cityName')
			return false
		end
	end
	
	local cityName = CityRef.CityRefCode[tostring(cityCode)]
		only.log('D',"cityName>>>>>>>>>>"..cityName)

	args['roadName'] = switch.half_to_full(args['roadName'])
	local key = string.format('%d:hotRoadName', cityCode)
	redis.cmd('roadname_search', '', 'zincrby', key, 1, args['roadName'])

	local trafic_msg = get_seg_msg(cityCode,cityName,args['roadName'])
--	only.log('D', 'traffic_msg = ' .. scan.dump(trafic_msg))
	if not trafic_msg then
		only.log('E',"trafic_msg error")
		return false
	end
	
	if args['resultType'] == '2' then
		trafic_msg['traffic']['trafficText'] = {}
	elseif args['resultType'] == '3' then
		
		trafic_msg['traffic']['trendInfo'] = {}
	end		
	local ok, fmt_data = pcall(json.encode,trafic_msg)	
	if not ok then
		only.log('E',"json encode error:"..scan.dump(trafic_msg))
		gosay.resp_msg(msg['SYSTEM_ERROR'])
		return false
	end
        gosay.resp_msg(msg['MSG_SUCCESS_WITH_RESULT'], fmt_data)

	return true
end


