local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local cutils		= require('cutils')

function app_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('access')
end

function app_call(tab)
end
