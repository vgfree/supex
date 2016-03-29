local utils     = require('utils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local gosay     = require('gosay')
local spx_utils	= require('spx_utils')
local redis_api = require("redis_short_api")

module('api_gain_video', package.seeall)


function handle()
	local args = supex.get_our_uri_table()
	if not args then
		only.log('E', 'args is nil')
		local afp = supex.rgs(400)
		return supex.over(afp)
	end
	
	local group = args["group"]
	local file  = args["file"]
	local isStorage = args['isStorage']
	
	if not file or (not group and not isStorage)then
		only.log('E', 'args is nil')
		local afp = supex.rgs(400)
		return supex.over(afp)
	end
	--judge isStorage exist
	if not isStorage then
		group = string.gsub(group, 'group_', 'dfsdb_')
		isStorage = 'true'
	end
	--> fiter
	local st, ed, type_val = string.find(file, "%.(%a%a.)$")
	if (not type_val) then
		only.log("E","file is not know the type")
		local afp = supex.rgs(403)
		return supex.over(afp)
	end

	--> get mp4
	local ok,binary = nil ,nil
	if isStorage == 'true' then
		ok, binary = spx_utils.get_from_dfsdb(file, group)
		if not ok then
			ok,binary = spx_utils.get_from_dfsdb_spare(file, group)
			if not ok then
				only.log('E', 'get from dfsdb is failed')
				local afp = supex.rgs(500)
				return supex.over(afp)
			end
		end
	else
		ok, binary = spx_utils.get_from_redis(file, group)
		if not ok then
			ok,binary = spx_utils.get_from_redis_spare(file, group)
			if not ok then
				only.log('E', 'get from redis is failed')
				local afp = supex.rgs(500)
				return supex.over(afp)
			end
		end
	end

	if not binary then
		only.log('E', 'get from dfsdb is ok but binary is failed')
		local afp = supex.rgs(404)
		return supex.over(afp)
	end
	
	local afp = supex.rgs(200)
	supex.say(afp, binary)
	return supex.over(afp, "video/mp4")
end



