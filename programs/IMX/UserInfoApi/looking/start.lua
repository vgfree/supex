local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local cutils		= require('cutils')
local redis_api		= require('redis_pool_api')
local looking		= require("looking")

module("start", package.seeall)

function app_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('access')

	--> init redis
	redis_api.init( )
end

function app_call(tab)
	local ret = nil
	if #tab > 2 then
		ret = looking.handle(tab)
	end
	--only.log('E', 'In app_call the ret = %s', scan.dump(ret))
	return ret
end
