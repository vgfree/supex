--Author	: dujun
--Date		: 2015-11-19
--Function	: get text info

local only			= require('only')
local scan			= require('scan')
local func_get_traffic_msg	= require('func_get_traffic_msg')

module("func_get_trafficText", package.seeall)

local roadType = {
	[1] = "畅通",
	[2] = "缓行",
	[3] = "拥堵",
	[4] = "无路况",
	[6] = "拥堵缓行"
}

function get_time_and_len(time, len)

	local pass_time, part_time = math.modf(time/60)
	if part_time >= 0.5 then pass_time = pass_time + 1 end
	local length, dec = math.modf(len/50)
	length = length*50
	if dec > 0 then length = length + 50 end
	return pass_time, length
end

function operate_text(i,text,trafficText)

	local pass_time, length = get_time_and_len(text[i]['PT'],text[i]['RL'])
	front_road = {
		[1] = string.format("从%s到%s路口，", text[i]['startName'], text[i]['endName']),
		[2] = string.format("有%s路段，", roadType[text[i]['TT']]),
		[3] = string.format("%s长度为%d米，",roadType[text[i]['TT']], length),
		[4] = string.format("预计%d分钟通过。", pass_time)
	}
	if pass_time < 1 then front_road[4] = '' end
	if length < 100 then front_road[3] = '' end
	local str = table.concat(front_road)
	if text[i]['TT'] == 1 then
		str = string.format("从%s到%s路口畅通!",text[i]['startName'], text[i]['endName'])
	end
	local tab = {
		text = str,
		startSG = text[i]['startSG'],
		endSG = text[i]['endSG'],
		isCurrent = 0,
		TT = text[i]['TT']
	}
	table.insert(trafficText, tab)
end

function get_text_message(text, textType)

	local trafficText = {}
	if textType == 1 then
		local pass_time, length = get_time_and_len(text[1]['PT'],text[1]['RL'])
		pass_road = {
			[1] = string.format("正在经过%s路段，", roadType[text[1]['TT']]),
			[2] = string.format("%s长度为%d米，", roadType[text[1]['TT']], length),
			[3] = string.format("预计通过%d分钟驶出，", pass_time),
			[4] = string.format("驶出位置位于%s路口。", text[1]['endName'])
		}
		if pass_time < 1 then front_road[3] = '' end
		if length < 100 then front_road[2] = '' end
		local str = table.concat(pass_road)
		local tab = {
			text = str,
			startSG = text[1]['startSG'],
			endSG = text[1]['endSG'],
			isCurrent = 1,
			TT = text[1]['TT']
		}
		table.insert(trafficText, tab)
		for i = 2, #text do
			operate_text(i,text,trafficText)
		end
	else
		for i = 1, #text do
			operate_text(i,text,trafficText)
		end
	end
	return trafficText
end

--将合并元素插入结果集中
local function insert_traffic(ret_traffic, last_traffic)
	last_traffic['inserted'] = 1
	table.insert(ret_traffic ,last_traffic)
end

--合并路况信息
local function merge_traffic(traffic1, traffic2, index) 
--	only.log('I', "traffic1" .. scan.dump(traffic1))
--	only.log('I', "traffic2" .. scan.dump(traffic2))
--	only.log('I', "index" .. (index or "nil"))
--	only.log('I', "last_index" .. (last_index or "nil"))
	local ret_traffic = {}
	ret_traffic['startName'] = traffic1['startName']
	ret_traffic['endName'] = traffic2['endName']
	ret_traffic['PT'] = traffic1['PT'] + traffic2['PT']
	ret_traffic['RL'] = traffic1['RL'] + traffic2['RL']
	ret_traffic['startSG'] = traffic1['startSG']
	ret_traffic['endSG'] = index

	ret_traffic['TT'] = func_get_traffic_msg.compare_TT(traffic1['TT'],traffic2['TT'])
	return ret_traffic
end

function get_last_by_traffic(seg_traffic, index, last_name)
	local ret = {}

	ret['TT'] = seg_traffic['TT']
	ret['PT'] = seg_traffic['PT']
	ret['startSG'] = index
	ret['endSG'] = index
	ret['RL'] = seg_traffic['RL']
	ret['startName'] = last_name or ''
	ret['endName'] = seg_traffic['IN']

	return ret
end

function get_result(speedInfo, resultType)

	local textType
	local seg_tab = speedInfo['traffic']['roadSegment']
	if #seg_tab == 0 then
		return speedInfo
	end
	if seg_tab[1]['TT'] ~= 4 then
		textType = 1
	else
		textType = 2
	end

	local traffic_infos = seg_tab
	local ret_traffic = {}
	local last_traffic = nil
	local last_name = speedInfo['current']['startIN']

	for k,v in ipairs(traffic_infos) do
	repeat
		if not last_traffic then	--第一个路口
			if v['TT'] ~= 4 then
				last_traffic = get_last_by_traffic(v, k, last_name)
			end
			last_name = v['IN']
			break  --continue
		end
		if last_name == '' then
			last_name = v['IN']
			last_traffic = merge_traffic(last_traffic,get_last_by_traffic(v,k,last_name),k)
			break
		end
		
		local cur_TT = v['TT']
		if cur_TT == 4 then    --无路况的结束当前合并
			insert_traffic(ret_traffic ,last_traffic) --插入合并结果中
			last_traffic = nil
		elseif cur_TT == 1 then
			if last_traffic['TT'] ~= 1 then --结束当前路况合并
				insert_traffic(ret_traffic ,last_traffic) --插入合并结果中
				last_traffic = get_last_by_traffic(v, k, last_name)
			else
				last_traffic = merge_traffic(last_traffic, get_last_by_traffic(v, k, last_name), k)
			end
		elseif cur_TT == 2 or cur_TT == 3 or cur_TT == 6 then
			if (last_traffic['TT'] == 1 or last_traffic['TT'] == 4) then  --结束当前路况合并
				insert_traffic(ret_traffic ,last_traffic) --插入合并结果中
				last_traffic = get_last_by_traffic(v,k, last_name)
			else
				last_traffic = merge_traffic(last_traffic, get_last_by_traffic(v, k, last_name), k)
			end
		end
		last_name = v['IN']
	until true
	end

	--最后一个元素也要插入
	if last_traffic and last_traffic['TT'] ~= 4 and last_traffic['inserted'] ~= 1 then
		insert_traffic(ret_traffic ,last_traffic)
	end

--	only.log('I',"merge traffic result" .. scan.dump(ret_traffic))

--	return ret_traffic
	
	trafficText = get_text_message(ret_traffic, textType)
	speedInfo['traffic']['trafficText'] = trafficText

	if resultType == 2 then
		speedInfo['traffic']['trafficText'] = {}
	elseif resultType == 3 then
		speedInfo['traffic']['roadSegment'] = {}
	end
	return speedInfo
end

