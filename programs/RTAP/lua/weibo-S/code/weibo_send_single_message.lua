local only = require('only')
local supex = require('supex')
local Coro = require("coro");
local redis_api = require('redis_pool_api')

module('weibo_send_single_message', package.seeall)



local function self_cycle_idle( coro, idleable )
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


local function work_redis_two( coro, usr )
        assert(usr)
        print(string.format("ID %d start ...", usr.id))

	print("send data")
	local idle = coro.fastswitch
	redis_api.reg( idle, coro )
	local ok, info = redis_api.cmd('weibo', "", 'ZADD', usr.UID .. ":weiboPriority", usr.level, usr.label)
	print(ok, info)
	print("recv data")

        return ok
end

local function work_redis_one( coro, usr )
        assert(usr)
        print(string.format("ID %d start ...", usr.id))

	print("send data")
	local idle = coro.fastswitch
	redis_api.reg( idle, coro )
	local ok, info = redis_api.cmd('weibo', "", 'SETEX', usr.label .. ":weibo", 300, usr.message)
	print(ok, info)
	print("recv data")

        return ok
end





local function forward_task_redis( tasks )
	local coro = Coro:open(true)
	coro:addtask(work_redis_one, coro, tasks[1])
	coro:addtask(work_redis_two, coro, tasks[2])

        if coro:startup(self_cycle_idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end




function handle()
	local args = supex.get_our_body_table()
	local UID = args["UID"]
	local message = args["message"]
	local label = args["label"]
	local level = args["level"]
	if UID and message and label and level then
		local tasks = {
			{ id = 1, label = label, message = message },
			{ id = 2, UID = UID, level = level, label = label },
		}
		forward_task_redis( tasks )
	end
end

