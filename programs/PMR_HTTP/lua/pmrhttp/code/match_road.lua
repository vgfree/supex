--Date		: 2016-01-14
--Author	: dujun
--Function	: location by GPSPoint

local only	= require('only')
local scan	= require('scan')
local supex	= require('supex')
local utils	= require('utils')

module('match_road', package.seeall)

local function match_road(tab)

	tab['mapRoad'] = {}
	for i=1, #tab['longitude'] do
		lon = tab['longitude'][i]
		lat = tab['latitude'][i]
		dir = tab['direction'][i]

		local ret = entry_cmd_locate(lon, lat, dir)
		if not ret then
			ret = {}
		end
		only.log('D', 'ret = ' .. scan.dump(ret))
		table.insert(tab['mapRoad'], ret)
	end

	return tab
end

local function resp_msg(data)
	local afp = supex.rgs(200)
	supex.say(afp, data)
	return supex.over(afp)
end

function handle()

	local args = supex.get_our_body_table()
	if not args or not next(args) then
		resp_msg('[]')
		return false
	end
	only.log('D', 'args = ' .. scan.dump(args))

	local result = match_road(args)
	local ok, info = utils.json_encode(result)
	if not ok then
		only.log('E', "json_encode error : " .. scan.dump(result))
		resp_msg('[]')
		return false
	end
	resp_msg(info)
end
