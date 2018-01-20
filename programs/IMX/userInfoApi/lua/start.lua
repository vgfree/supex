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

local ctx = zmq.init(1)

function app_call(tab)
	if tab[1] == 'status' then	
		setting.loginServerInfoSave(tab)
	end
	if tab[1] == 'setting' then
		setting.appServerInfoSave(tab)
		local s = ctx:socket(zmq.PUSH)
		s:connect("tcp://127.0.0.1:10000")
		s:send_table(tab)
		s:close()
	end
	if tab[1] == 'looking' then
		looking.appServerInfoLoad(tab)		
	else
		only.log('E', 'First frame is invalid !')
	end
end
