local only = require('only')
local redis_api = require('redis_pool_api')
local supex = require('supex')
local Coro = require("coro")
local luakv_api = require('luakv_pool_api')

module('weibo_send_group_message', package.seeall)



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



-- 20160304 解决微博发送时，用户数超过1000时后续用户会发送失败bug
local function set_userweibo_redis( coro, usr )
        assert(usr)

	local idle = coro.fastswitch
	redis_api.reg( idle, coro )
	local ok, info = redis_api.cmd('weibo', "", 'ZADD', usr.uid .. ":weiboPriority", usr.tasks.level, usr.tasks.label)
	if not ok then only.log('E','weibo redis error') end
	if (usr.id==1000) then
		only.log('D',string.format("1000 usernum ok"))
	end

        return ok
end


local function forward_task_redis(users,tasks)
	ok, info = redis_api.cmd('weibo', "", 'SETEX', tasks.label .. ':weibo', 300, tasks.message)
	if not ok then only.log('E','weibo redis error') end
	
	-- #users 为1000的整数倍时，执行相应的倍数次，有余数的，取入一位数
	local step = 50
	local j = 0
	local i = 0
	while i < #users do
		local coro = Coro:open(true)
		for j = (i + 1), i+step do
			if (j>#users) then
				only.log('D',string.format("send %d users",j))
				break
			end
			local task_data = {}
			task_data.id = j
			task_data.uid = users[j]
			task_data.tasks = tasks
			coro:addtask(set_userweibo_redis, coro, task_data)

		end

		if coro:startup(self_cycle_idle, coro, true) then
			only.log('D',"Tasks execute success.")
		else
			only.log('E',"Tasks execute failure.")
		end
		coro:close()
		i = i + step
	end
end

function handle()
	local args = supex.get_our_body_table()
	GID = args["GID"]
	if GID then
		local ok, users = luakv_api.cmd("owner", GID, "SMEMBERS", GID)
		if #users == 0 then
			ok, users = redis_api.cmd('statistic',GID, 'smembers', (GID or '') .. ':channelOnlineUser')
			if not ok then only.log('E','weibo redis error') end
		end
		if ok and #users ~= 0 then
			local tasks = {
				level = args['level'],
				label = args['label'],
				message = args['message'],
			}
			
			forward_task_redis(users,tasks)
		end
		
	end
end

