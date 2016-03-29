local utils     = require('utils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local gosay     = require('gosay')
local spx_utils	= require('spx_utils')

module('api_gain_file', package.seeall)
	
function handle()
	local args = supex.get_our_uri_table()
	if not args then
		only.log('E', 'args is nil')
		local afp = supex.rgs(400)
		return supex.over(afp)
	end
	
	local group = args["group"]
	local file  = args["file"]
	
	if not group or not file then
		only.log('E', 'args is nil')
		local afp = supex.rgs(400)
		return supex.over(afp)
	end

	--> fiter
	local st, ed, fileType = string.find(file, "%.(%a%a+)$")
	if not fileType or string.find(fileType,"%.") or  #fileType < 1 or #fileType > 4 then
		only.log("E","file is not know the type")
		local afp = supex.rgs(403)
		return supex.over(afp)
	end

	--> get file
	local ok, binary = spx_utils.get_from_dfsdb(file, group)
	if not ok then
		ok, binary = spx_utils.get_from_dfsdb_spare(file, group)
		if not ok then
			only.log('E', 'get from dfsdb is failed')
			local afp = supex.rgs(500)
			return supex.over(afp)
		end
	end
	
	if not binary then
		only.log('E', 'get from dfsdb is ok but binary is failed')
		local afp = supex.rgs(404)
		return supex.over(afp)
	end
	
	local afp = supex.rgs(200)
	supex.say(afp, binary)
	return supex.over(afp, "application/octet-stream")
end
