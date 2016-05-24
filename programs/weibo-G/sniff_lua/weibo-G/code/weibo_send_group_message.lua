local only = require('only')
local redis_api = require('redis_pool_api')
local supex = require('supex')
local Coro = require("coro")
local luakv_api = require('luakv_pool_api')
local scan = require('scan')

module('weibo_send_group_message', package.seeall)



local function self_cycle_idle( coro, idleable )
	if not idleable then 
		only.log('E',"IDLE~~~~")
	else
		if coro:isactive() then
			--print("\x1B[1;35m".."LOOP~~~~".."\x1B[m")
			--coro:fastswitch()lua_default_switch, supex["__TASKER_SCHEME__"],txt)
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
	local ok, info = redis_api.hash_cmd('weibo_hash', usr.uid, 'ZADD', usr.uid .. ":weiboPriority", usr.tasks.level, usr.tasks.label)
	if not ok then only.log('E','weibo redis error') end
        return true
end


local function forward_task_redis(users,tasks)
	if type(users) == "string" then
		only.log('E','xxxxxxxxx users type error is %s',users)
		return
	end
	ok, info = redis_api.cmd('weibo', "", 'SETEX', tasks.label .. ':weibo', 300, tasks.message)
	if not ok then
		only.log('E','weibo redis error')
		return
	end
	if not  users[1] then
			only.log('E',"xxxxxxx users[1] is nil and  = %s",tasks.message)
			only.log('E',"xxxxx users =  %s",scan.dump(users))
	end
	
	-- #users 为1000的整数倍时，执行相应的倍数次，有余数的，取入一位数
	local step = 50
	local i = 0
	while i < #users do
		local coro = Coro:open(true)
		local task_data
		for j = (i + 1), i+step do
			if (j>#users) then
				break
			end
			task_data = {}
			task_data.id = j
			task_data.uid = users[j]
			task_data.tasks = tasks
			if task_data.uid then
				coro:addtask(set_userweibo_redis, coro, task_data)
			else
				
				only.log('E',"uid is nil and  = %s,j=%s",tasks.message,tostring(j))
				only.log('E',"users =  %s",scan.dump(users))
			end

		end
		local ret = coro:startup(self_cycle_idle, coro, true)

		if ret  then
			only.log('D',"Tasks execute success.")
		else
			
			only.log('E',"Tasks execute failure.ret = %s",tostring(ret))
		end
		coro:close()
		collectgarbage("collect")
		i = i + step
	end
	only.log('S',string.format("label %s send %d users",tasks.label,#users))
end

function handle()
	local args = supex.get_our_body_table()
	local GID = args["GID"]
	if GID == '000001312' then
		return
	end
	if GID then
		local i = math.random(1,2)
		local hash_key = {"b0","b1"}
		redis_api.reg( nil,nil )
		local ok, users = redis_api.hash_cmd('statistic_hash',hash_key[i], 'smembers', (GID or '') .. ':channelOnlineUser')
		if not ok then 
			only.log('E','weibo redis error')
			return
		end
		if type(users) == "string" then
			only.log('E','users type error is %s',scan.dump(args))
			return
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
	collectgarbage("collect")
end

