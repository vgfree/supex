local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local cutils		= require('cutils')
local redis_api		= require('redis_pool_api')
local setting		= require("setting")
local looking		= require("looking")

function app_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('access')

	--> init redis
	redis_api.init( )
end

function app_call(tab)
	only.log('I', "%s", tab[1])
	if tab[1] == 'status' then	
		setting.loginServerInfoSave(tab)
		return
	end
	if tab[1] == 'setting' then
		setting.appServerInfoSave(tab)
		app_lua_send_message(tab)
		return
	end
	if tab[1] == 'looking' then
		looking.appServerInfoLoad(tab)		
	else
		only.log('E', 'First frame is invalid !')
	end
end
