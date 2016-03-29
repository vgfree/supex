local crzpt_http = _G.crzpt_http

module("crzpt", package.seeall)


_FINAL_STAGE_	= false
_SOCKET_HANDLE_	= 0
__TASKER_SCHEME__	= 0

local our_info_data	= nil
local our_body_data	= nil
local our_body_table	= {}
local our_uri_args	= nil
local our_uri_table	= {}

function http(host, port, data, size)
	return crzpt_http(__TASKER_SCHEME__, host, port, data, size)
end

function get_our_body_data( ... )
	return our_body_data
end

function get_our_body_table( ... )
	return our_body_table
end

function get_our_uri_args( ... )
	return our_uri_args
end

function get_our_uri_table( ... )
	return our_uri_table
end
