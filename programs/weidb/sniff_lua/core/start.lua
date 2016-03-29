local LIB_CJSON		= require('cjson')
local APP_GOSAY		= require('gosay')
local APP_APPLY		= require('apply')
local APP_REDIS_API	= require('redis_pool_api')
local APP_LHTTP_API	= require('lhttp_pool_api')
local zmq_api		= require('zmq_api')
local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local supex		= require('supex')
local luakv_api		= require('luakv_pool_api')
local BYNAME_LIST	= require('BYNAME_LIST')



function app_line_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('manage')
	lualog.open('access')

	--> init redis
	APP_REDIS_API.init( )
	--> init lhttp
	APP_LHTTP_API.init( )
	--> init zmq
	zmq_api.init( )
	--> init luakv
	luakv_api.init()
	--> init apply
	APP_APPLY.apply_init( )
	--> load apply
	APP_APPLY.apply_load( )

	_G.del_dn_from_pool = APP_REDIS_API.del_from_pool
end

function app_scco_init( sch, num )
	supex["__TASKER_SCHEME__"] = sch
	supex["__TASKER_NUMBER__"] = num

	APP_REDIS_API.reg( lua_default_switch, sch )
	APP_LHTTP_API.reg( lua_default_switch, sch )

	app_line_init()
end

function app_evcoro_init( sch, num )
	supex["__TASKER_SCHEME__"] = sch
	supex["__TASKER_NUMBER__"] = num

	APP_REDIS_API.reg( lua_default_switch, sch )
	APP_LHTTP_API.reg( lua_default_switch, sch )


	app_line_init()
end



function app_exit()
	--> free final
	lualog.close( )
end



function app_rfsh( top, sfd )
	supex["_FINAL_STAGE_"] = top
	only.log("S", 'rfsh logs ... .. .')
end

function app_cntl( top, name, cmds )
	supex["_FINAL_STAGE_"] = top
	only.log("I", string.format("【%s】 ------> |name:%s", cmds, name))
	local ctrl_cmd_list = {
		open = function( name )
			APP_APPLY.push_status( name, "open" )
		end,
		close = function( name )
			APP_APPLY.push_status( name, "close" )
		end,
		insmod = function( name )
			APP_APPLY.apply_insmod( name )
			APP_APPLY.push_status( name, "open" )
		end,
		rmmod = function( name )
			APP_APPLY.apply_rmmod( name )
			APP_APPLY.push_status( name, "null" )
		end,
		delete = function( name )
		end,
	}
	if APP_APPLY.apply_check( name ) then
		ctrl_cmd_list[cmds]( name )
	end
end


function app_pull( top, sfd )
	supex["_FINAL_STAGE_"] = top
	supex["_SOCKET_HANDLE_"] = sfd
	--> come into manage model
	lualog.open( "manage" )
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	local data = app_lua_get_body_data(sfd)
	only.log("I", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		--> parse data
		local pull_cmd_list = {
			get_all_app = function( )
				local data = APP_APPLY.pull_status()
				APP_GOSAY.resp( 200, data )
			end,
			get_app_cfg = function( )
				local data = APP_APPLY.pull_config( jo["appname"] )
				APP_GOSAY.resp( 200, data )
			end,
		}
		if jo["operate"] then 
			local ok,result = pcall( pull_cmd_list[ jo["operate"] ] )
			if not ok then
				only.log("E", result)
			end
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	lualog.open( "access" )
end

function app_push( top, sfd )
	supex["_FINAL_STAGE_"] = top
	supex["_SOCKET_HANDLE_"] = sfd
	--> come into manage model
	lualog.open( "manage" )
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	local data = app_lua_get_body_data(sfd)
	only.log("S", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		--> parse data
		local push_cmd_list = {
			ctl_one_app = function( )
				app_cntl( top, jo["appname"], jo["status"] )
			end,
			fix_app_cfg = function( )
				APP_APPLY.push_config( jo["appname"], jo["config"] )
			end,
		}
		if jo["operate"] then 
			local ok,result = pcall( push_cmd_list[ jo["operate"] ] )
			if not ok then
				only.log("E", result)
			end
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	lualog.open( "access" )
end


function main_call( way )
	--local come_msize = collectgarbage("count")
	--> run call
	local path = supex.get_our_path( )
	only.log("I", 'access : %s', path)
	if way then
		local api = BYNAME_LIST["OWN_LIST"][ path ] or string.sub(path, 2, -1)
		APP_REDIS_API.reg( lua_default_switch, supex["__TASKER_SCHEME__"] )
		APP_LHTTP_API.reg( lua_default_switch, supex["__TASKER_SCHEME__"] )
		APP_APPLY.apply_execute( api )
		APP_REDIS_API.reg( lua_default_switch, supex["__TASKER_SCHEME__"] )
		APP_LHTTP_API.reg( lua_default_switch, supex["__TASKER_SCHEME__"] )
	else
		APP_APPLY.apply_runmods( )
	end
	--[[
	local done_msize = collectgarbage("count")
	collectgarbage("collect")
	local over_msize = collectgarbage("count")
	print( string.format("APPLY CALL COME : memory size \t[%d]KB \t[%d]M", come_msize, come_msize/1024) )
	print( string.format("APPLY CALL DONE : memory size \t[%d]KB \t[%d]M", done_msize, done_msize/1024) )
	print( string.format("APPLY CALL OVER : memory size \t[%d]KB \t[%d]M", over_msize, over_msize/1024) )
	print()
	]]--
end


local function app_call_by_sfd( top, sfd, way )
	lualog.open( "access" )
	only.log("D", '_________________________________START_________________________________________')
	supex["_FINAL_STAGE_"] = top
	supex["_SOCKET_HANDLE_"] = sfd
	
	--> get data
	supex.http_req_init( )
	supex.set_our_info_data( app_lua_get_recv_buf(sfd) )
	supex.set_our_path( app_lua_get_path_data(sfd) )
	supex.set_our_head( app_lua_get_head_data(sfd) )
	supex.set_our_body_data( app_lua_get_body_data(sfd) )
	supex.set_our_body_table( )
	supex.set_our_uri_args( app_lua_get_uri_args(sfd) )
	supex.set_our_uri_table()
	
	lualog.addinfo( supex.get_our_body_table()["accountID"] )
	only.log("I", "BODY DATA is:%s", tostring(supex.get_our_body_data()))
	only.log("I", "URI DATA is:%s", tostring(supex.get_our_uri_args()))

	--> run call
	main_call( way )

	lualog.addinfo( nil )
	only.log("D", '_________________________________OVER_________________________________________\n\n')
end
local function app_call_by_msg( top, msg, way )
	lualog.open( "access" )
	only.log("D", '_________________________________START_________________________________________')
	supex["_FINAL_STAGE_"] = top
	supex["_SOCKET_HANDLE_"] = nil

	------- method , head , body , path , uri_arg
	local our_method, our_head, our_body_data, path, our_uri_args = utils.split_http_data(msg)
	--> get data
	supex.http_req_init( )
	supex.set_our_info_data(msg)
	supex.set_our_path( path )
	supex.set_our_method(our_method)
	supex.set_our_head(our_head)
	supex.set_our_body_data(our_body_data)
	supex.set_our_body_table( )
	supex.set_our_uri_args(our_uri_args)
	supex.set_our_uri_table( )
	

	lualog.addinfo( supex.get_our_body_table()["accountID"] )
	only.log("I", "BODY DATA is:%s", tostring(supex.get_our_body_data()))
	only.log("I", "URI DATA is:%s", tostring(supex.get_our_uri_args()))

	--> run call
	main_call( way )

	lualog.addinfo( nil )
	only.log("D", '_________________________________OVER_________________________________________\n\n')
end

local function app_call_all_by_sfd( top, sfd )
	app_call_by_sfd( top, sfd, false )
end

local function app_call_one_by_sfd( top, sfd )
	app_call_by_sfd( top, sfd, true )
end

local function app_call_all_by_msg( top, msg )
	app_call_by_msg( top, msg, false )
end

local function app_call_one_by_msg( top, msg )
	app_call_by_msg( top, msg, true )
end

app_call_all	= app_call_all_by_msg
app_call_one	= app_call_one_by_msg
