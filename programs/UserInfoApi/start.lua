local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local cutils		= require('cutils')
local redis_api		= require('redis_pool_api')
local api_redis_save	= require("api_redis_save")

function app_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('access')

	--> init redis
	redis_api.init( )
end

function app_call(tab)
	if #tab > 2 then
		api_redis_save.handle(tab)
	end
end
