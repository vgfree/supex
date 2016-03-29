local only = require('only')
local plan = require('plan')
local crzpt		= require('crzpt')

module('taskloop', package.seeall)

local function test( name )
	print(name)
end

function origin()
	only.log("D", "init start ... .. .")-- how to mount task once?
end

function lookup( jo )
	local pull_cmd_list = {
		erro = function( )
			only.log("E", "unknown cmd")
		end,
	}
	local ok,result = pcall( pull_cmd_list[ jo["EXEC"] or "erro" ] or pull_cmd_list[ "erro" ] )
	if not ok then
		only.log("E", result)
	end
end

function accept( jo )
	local push_cmd_list = {
		make = function( )
			local pidx = plan.make( jo["ARGS"], test, jo["TIME"], jo["LIVE"] )

			if pidx and crzpt["_FINAL_STAGE_"] then
				local ok = plan.mount( pidx )
				only.log("I", string.format("mount pidx %s %s", pidx, ok))
			end
		end,
		erro = function( )
			only.log("E", "unknown cmd")
		end,
	}
	local ok,result = pcall( push_cmd_list[ jo["EXEC"] or "erro" ] or push_cmd_list[ "erro" ] )
	if not ok then
		only.log("E", result)
	end
end

function handle()
	only.log("E", "??????")
	plan.make( "hello world!", test, 74220, false )
end

