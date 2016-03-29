local LIB_CJSON		= require('cjson')
local APP_PLAN		= require('plan')
local APP_UTILS		= require('utils')
local APP_REDIS_API	= require('redis_pool_api')
local lualog		= require('lualog')
local only		= require('only')

local app_lua_gain_plan = app_lua_gain_plan
local SERV_NAME		= app_lua_get_serv_name()
local APP_APPLY		= require( SERV_NAME )
local crzpt		= require('crzpt')
local luakv_api		= require('luakv_pool_api')
local CFG_LIST		= require('cfg')

function app_line_init( )
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('manage')
	lualog.open('access')
	--> init redis
	APP_REDIS_API.init( )
	--> init luakv
	luakv_api.init()
	--> init apply
	APP_APPLY.origin( )
end

function app_scco_init( sch )
	crzpt["__TASKER_SCHEME__"] = sch

	app_line_init( )
end

function app_exit( )
	--> free final
	lualog.close( )
end

function app_load( )
	--APP_PLAN.init( )
end


function app_rfsh( )
	only.log("S", 'rfsh logs ... .. .')
end



function app_pull( top, msg )
	crzpt["_FINAL_STAGE_"] = top
	--> come into manage model
	lualog.open( "manage" )
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	local data = msg
	only.log("I", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		local ok,result = pcall(APP_APPLY.lookup, jo)
		if not ok then
			only.log("E", result)
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	lualog.open( "access" )
end

function app_push( top, msg )
	crzpt["_FINAL_STAGE_"] = top
	--> come into manage model
	lualog.open( "manage" )
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	local data = msg
	only.log("S", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		local ok,result = pcall(APP_APPLY.accept, jo)
		if not ok then
			only.log("E", result)
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	lualog.open( "access" )
end

function app_call( top, idx )
	crzpt["_FINAL_STAGE_"] = top
	only.log("D", '_________________________________START_________________________________________')
	--> set app log
	lualog.open( SERV_NAME )
	only.log("I", string.format("call plan ID : %d", idx))
	local data = app_lua_gain_plan(idx)
	only.log("D", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		--> run call
		local ok,result = pcall(APP_APPLY.handle, jo)
		if not ok then
			only.log("E", result)
		end
	end
	::DO_NOTHING::
	--> reset app log
	lualog.open( "access" )
	--> reset data
	only.log("D", '_________________________________OVER_________________________________________\n\n')
end
