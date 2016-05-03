local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local cutils		= require('cutils')
local redis_api		= require('redis_pool_api')
local api_pic_newstatus	= require("api_pic_newstatus")

require("zmq")

local ctx = zmq.init(1)
local s = ctx:socket(zmq.PUSH)

function app_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('access')

	--> init redis
	redis_api.init( )
	s:connect("tcp://localhost:5557")
end

function app_call( tab )
	--for k,v in ipairs(tab) do
		--print(k, v)
	--end
	s:send_table(tab)

	--print(tab[1])
	local imageURL = api_pic_newstatus.handle_image(tab[3])
	api_pic_newstatus.handle_gson(tab[2], imageURL, 'jpg')
	--api_pic_newstatus.handle_image(tab[3])
        --api_pic_newstatus.handle_audio(tab[4])
end
