local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local cutils		= require('cutils')
local redis_api		= require('redis_pool_api')
local api_pic_newstatus	= require("api_pic_newstatus")

function app_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('access')

	--> init redis
	redis_api.init( )
end

function app_call( tab )
	--for k,v in ipairs(tab) do
		--print(k, v)
	--end
	print(tab[1])
	print(tab[2])
	api_pic_newstatus.handle1(tab[3])
        api_pic_newstatus.handle2(tab[4])
end
