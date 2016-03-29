local only = require('only')
local supex = require('supex')
local Coro = require("coro");
local redis_api = require('redis_pool_api')
local lhttp_api = require('lhttp_pool_api')

module('x_test', package.seeall)

local data = {
	'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.renren.com\r\n' ..
	'Accept: */*\r\n\r\n',

	'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.youku.cn\r\n' ..
	'Accept: */*\r\n\r\n',

	'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.zhihu.com\r\n' ..
	'Accept: */*\r\n\r\n',
}


local function work_redis( coro, usr )
        assert(usr)
        print(string.format("ID %d start ...", usr.id))

	print("send data")
	local idle = coro.fastswitch
	redis_api.reg( idle, coro )
	local ok, info = redis_api.cmd(usr.redis, "", 'set', "key", "value")
	print(ok, info)
	print("recv data")

        return ok
end

local function work_lhttp( coro, usr )
        assert(usr)
        print(string.format("ID %d start ...", usr.id))

	print("send data")
	local idle = coro.fastswitch
	lhttp_api.reg( idle, coro )
	local ok, info = lhttp_api.cmd(usr.lhttp, "", "origin", data[usr.id])
	print(ok, info["data"])
	print("recv data")

        return ok
end


local function idle( coro, idleable )
	if not idleable then 
		print("\x1B[1;35m".."IDLE~~~~".."\x1B[m")
	else
		if coro:isactive() then
			--print("\x1B[1;35m".."LOOP~~~~".."\x1B[m")
			--coro:fastswitch()lua_default_switch, supex["__TASKER_SCHEME__"],txt)
			lua_default_switch(supex["__TASKER_SCHEME__"])
		else
			print("\x1B[1;33m".."coro:stop()".."\x1B[m")
			coro:stop()
			return
		end
	end
end





local tasks = {
	{ id = 1, redis = "redis1", lhttp = "lhttp1"  },
	{ id = 2, redis = "redis2", lhttp = "lhttp2"  },
	{ id = 3, redis = "redis3", lhttp = "lhttp3"  },
	{ id = 4, redis = "redis4", lhttp = "lhttp4"  },
	{ id = 5, redis = "redis5", lhttp = "lhttp5"  },
	{ id = 6, redis = "redis6", lhttp = "lhttp6"  },
}


local function forward_task_redis()
	local coro = Coro:open(true)
	coro:addtask(work_redis, coro, tasks[1])
	--coro:addtask(work_redis, coro, tasks[2])
	--coro:addtask(work_redis, coro, tasks[3])

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end


local function forward_task_lhttp()
	local coro = Coro:open(true)
	coro:addtask(work_lhttp, coro, tasks[1])
	coro:addtask(work_lhttp, coro, tasks[2])
	coro:addtask(work_lhttp, coro, tasks[3])

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end




function handle()
	--forward_task_redis()
	redis_api.reg( lua_default_switch, supex["__TASKER_SCHEME__"] )

	forward_task_lhttp()
	lhttp_api.reg( lua_default_switch, supex["__TASKER_SCHEME__"] )
end

