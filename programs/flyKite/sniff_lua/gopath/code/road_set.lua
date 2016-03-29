--文件名称: road_set.lua
--创建者  : 耿玄玄
--创建时间: 2015-08-28
--文件描述: 记录整理最终途径结果类
--历史记录: 2015-08-28 创建

local only			= require ('only')
local RoadRecordModule 		= require('road_record')
local RoadRecord		= RoadRecordModule.RoadRecord

module('road_set', package.seeall)

RoadSet = {
	IMEI,
	tokenCode,
	accountID,
	raodIdx,
	roadIDSet,
}

function RoadSet:new(IMEI, tokenCode, accountID)
	local self = {
	}

	setmetatable(self, RoadSet)
	RoadSet.__index = RoadSet

	self['IMEI'] = IMEI
	self['tokenCode'] = tokenCode
	self['accountID'] = accountID
	self['roadIdx'] = 0
	self['roadIDSet'] = {}

	return self
end 

--添加roadRecord到RoadSet中
function RoadSet:addRoadRecord(road_record)
	if not road_record then
		return 
	end

	local road_info = road_record:getRoadInfo()
	if not road_info then
		road_info = {}
	end

	self['roadIdx'] = self['roadIdx'] + 1
	road_info['roadIDIndex'] = self['roadIdx']
	road_info['imei'] = self['IMEI']
	road_info['accountID'] = (self['accountID'] ~= self['IMEI']) and self['accountID'] or ''
	road_info['tokenCode'] = self['tokenCode']

	table.insert(self['roadIDSet'], road_info)
--	only.log('D', "RoadSet:addRoadRecord:%s", road_record['roadID'])
end

function RoadSet:remove(road_id)
	local ele_num = #self['roadIDSet']
	if self['roadIdx'] == 0 or ele_num == 0 then 
		return
	end

	if self['roadIDSet'][ele_num]['roadID'] ~= road_id then
		return
	end

--	only.log('D', "RoadSet:remove, road:%s", self['roadIDSet'][ele_num]['roadID'])

	table.remove(self['roadIDSet'], ele_num)
	self['roadIdx'] = self['roadIdx'] - 1
end

--添加空的road_info
function RoadSet:addRoadInfo(road_info)
	if not road_info then
		return
	end 

	road_info['imei'] = self['IMEI']
	road_info['accountID'] = (self['accountID'] ~= self['IMEI']) and self['accountID'] or ''
	road_info['tokenCode'] = self['tokenCode']

	table.insert(self['roadIDSet'], road_info)
end

local function get_roadid_str(road_info_set)
	if not road_info_set then
		return ""
	end

	local full_str = ""

	for i, v in ipairs(road_info_set) do
	repeat
		local road_id = v['roadID']
		if not road_id then
			break	--continue
		end

		if full_str == "" then
			full_str = '\'' .. road_id .. '\''
		else
			full_str = full_str .. ',\'' .. road_id .. '\''
		end
	until true
	end

	return full_str
end

function RoadSet:save(file_path)
	if not file_path then 
		file_path = "~"
	end

	local full_name = string.format("%s/%s_%s.txt", file_path, self['IMEI'], self['tokenCode'])
	local file = io.open(full_name, "w+")

	if not file then
		only.log('E', "open %s error!", full_namename)
	end

	local full_str = get_roadid_str(self['roadIDSet'])
	file:write(full_str)

	file:write("\n\n")
--	file:write(scan.dump(self['roadIDSet']))

	file:close()
end
