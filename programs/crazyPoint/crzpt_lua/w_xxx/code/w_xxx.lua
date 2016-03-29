local only = require('only')
local plan = require('plan')
local scan = require('scan')
local crzpt		= require('crzpt')

module('w_xxx', package.seeall)

--[[
local function main_entry( ... )
print("hello world!")
end
]]--


function origin()
	only.log("D", "init start ... .. .")-- how to mount task once?
end

function origin()
	only.log("D", "init start ... .. .")-- how to mount task once?
end


function lookup( jo )
	print("xxxx")
end

function accept( jo )
	--[[
	--> make lua
	local delay = 240
	local time = (os.time() + 8*60*60 + delay) % (24*60*60)
	local pidx = plan.make( nil, main_entry, time, true )
	--> mount C
	if pidx and crzpt["_FINAL_STAGE_"] then
	local ok = plan.mount( pidx )
	only.log("I", string.format("mount pidx %s %s", pidx, ok))
	end
	]]--

end


function handle( jo )
	print( scan.dump(jo) )
end

