local only = require('only')
local supex = require('supex')
local Coro = require("coro");
local redis_api = require('redis_pool_api')

module('weibo_send_single_message', package.seeall)



local function self_cycle_idle( coro, idleable )
	if not idleable then 
		only.log("D", "IDLE~~~~")
	else
		if coro:isactive() then
			lua_default_switch(supex["__TASKER_SCHEME__"])
		else
			only.log("D", "coro:stop()")
			coro:stop()
			return
		end
	end
end


local function work_redis_two( coro, usr )
        assert(usr)

	only.log("D", "send data")
	local idle = coro.fastswitch
	redis_api.reg( idle, coro )
	local ok, info = redis_api.cmd('weibo', "", 'ZADD', usr.UID .. ":weiboPriority", usr.level, usr.label)
	only.log("D", "recv data")

        return ok
end

local function work_redis_one( coro, usr )
        assert(usr)

	only.log("D", "send data")
	local idle = coro.fastswitch
	redis_api.reg( idle, coro )
	local ok, info = redis_api.cmd('weibo', "", 'SETEX', usr.label .. ":weibo", 300, usr.message)
	only.log("D", "recv data")

        return ok
end





local function forward_task_redis( tasks )
	local coro = Coro:open(true)
	coro:addtask(work_redis_one, coro, tasks[1])
	coro:addtask(work_redis_two, coro, tasks[2])

        if coro:startup(self_cycle_idle, coro, true) then
                only.log("D", "Tasks execute success.")
        else
                only.log("D", "Tasks execute failure.")
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

