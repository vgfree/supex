local redis_api = require('lredis_pool_api')
local Coro = require("coro");
local only = require('only')
local utils = require('utils')
local link  = require ("link")
local msg = require('msg')
local map = require('map')
local json = require('cjson')
local supex = require('supex')
local http_api = require('http_short_api')
local utils = require('utils')
local socket = require('socket')
local cfg = require('cfg')
local scan = require('scan')

local redis_api = require('lredis_pool_api')
local Coro = require("coro");


local function self_cycle_idle( coro, idleable )
        if not idleable then
                only.log('E',"IDLE~~~~")
        else
                if coro:isactive() then
                        lua_default_switch(supex["__TASKER_SCHEME__"])
                else
                        only.log('D',"coro:stop()")
                        coro:stop()
                        return
                end
        end
end




---------异步执行

local function work_redis6(coro, task)
		local idle = coro.fastswitch
        redis_api.reg( idle, coro )
        local key = "hello"
        local val = "word"
        local ok, info = redis_api.cmd('private',"",'set',key,val)
        local ok, info = redis_api.cmd('private',"",'get',key)
        only.log('D',"info"..scan.dump(info))
end

local function asynchronism_6()

		local task = 'road'
        local coro = Coro:open(true)

        for i = 1, 50 do
                coro:addtask(work_redis6, coro, task)
        end

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()

end

local function work_redis5(coro, task)

		local idle = coro.fastswitch
        redis_api.reg( idle, coro )
		local key1 = "site"
        local key2 = "redis"
        local val1 = "redis.com"
        local ok, info = redis_api.cmd('private',"",'hset',key1,key2,val)
        print(ok, info)
        print("recv data")
        local ok, info = redis_api.cmd('private',"",'hget',key1,key2)
        print(info)
        only.log('D',"info"..scan.dump(info))

end

local function asynchronism_5()

		local task = 'road'
        local coro = Coro:open(true)
        for i = 1, 50 do
                coro:addtask(work_redis5, coro, task)
        end
        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()

end

local function work_redis4(coro, task)

		local idle = coro.fastswitch
        redis_api.reg( idle, coro )
        local key3 = "bbs"
        local val2 ,val3,val4 = "daoke","weime","nipaikanwo"
        local ok, info = redis_api.cmd('private',"",'sadd',key3,val2,val3,val4)
        print("recv data")
        local ok, info = redis_api.cmd('private',"",'SMEMBERS',key3)
        print(info)
        only.log('D',"info"..scan.dump(info))

end

local function  asynchronism_4()
		local task = 'road'
        local coro = Coro:open(true)

        for i = 1, 50 do
                coro:addtask(work_redis4, coro, task)
        end

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end

local function work_redis3(coro, task)

      	local ok,data = redis_api.cmd('road_test','','hmget','MLOCATE',409601328282142,121.364435,31.224012,-1,32,0,1458711428)
    	only.log('D',"data"..scan.dump(data))

end

local function asynchronism_3()

		local task = 'road'
        local coro = Coro:open(true)

        for i = 1, 50 do
                coro:addtask(work_redis3, coro, task)
        end

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end

local function work_redis2(coro, task)

        local idle = coro.fastswitch
        redis_api.reg( idle, coro )

        local file = io.open('about.txt', 'w+')
        for i = 1, 1025*1024 do
                file:write('1234567890abcdef')
        end
        file:close()
 
        file = io.open('about.txt')
        local data = file:read('*all')

        local key = "hello"
        local val =  data
        local ok, info = redis_api.cmd('private',"",'set',key,val)
        local ok, info = redis_api.cmd('private',"",'get',key)
        only.log('D',"info"..scan.dump(info))

end

local function asynchronism_2()

		local task = 'road'
        local coro = Coro:open(true)

        for i = 1, 2 do
                coro:addtask(work_redis2, coro, task)
        end

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end


----------同步执行
function synchroscope_redis()

		local key = "hello"
        local val = "word"
        local ok, info = redis_api.cmd('private',"",'set',key,val)
        local ok, info = redis_api.cmd('private',"",'get',key)
        only.log('D',"info"..scan.dump(info))
        --------
        local key1 = "site"
        local key2 = "redis"
        local val1 = "redis.com"
        local ok, info = redis_api.cmd('private',"",'hset',key1,key2,val)
        print(ok, info)
        print("recv data")
        local ok, info = redis_api.cmd('private',"",'hget',key1,key2)
        print(info)
        only.log('D',"info"..scan.dump(info))
        -------
        local key3 = "bbs"
        local val2 ,val3,val4 = "daoke","weime","nipaikanwo"
        local ok, info = redis_api.cmd('private',"",'sadd',key3,val2,val3,val4)

        print("recv data")
        local ok, info = redis_api.cmd('private',"",'SMEMBERS',key3)
        print(info)
        only.log('D',"info"..scan.dump(info))

        -----------
        local ok,data = redis_api.cmd('road_test','','hmget','MLOCATE',409601328282142,121.364435,31.224012,-1,32,0,1458711428)
        only.log('D',"data"..scan.dump(data))
        -------
end

function asynchronous_redis()

		asynchronism_6()
		asynchronism_5()
		asynchronism_4()
		asynchronism_3()
		asynchronism_2()
        return ok

end
function handle()

	print("start")
	synchroscope_redis()
	asynchronous_redis()
end