local only = require('only')
local supex = require('supex')

module('gainTS', package.seeall)


function handle()
	local args = supex.get_our_uri_table()
	local file  = args["file"]
	-- get TS data
	local fd = io.open("./data/" .. file, "r")
	local data = fd:read("*a")
	fd:close()

	local afp = supex.rgs(200)
	if data then
		supex.say(afp, data)
	end
	return supex.over(afp, "video/MP2T")
end

