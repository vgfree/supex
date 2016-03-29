package.path = "../?.lua;"

local Coro = require("coro");

--测试一次概率为p的事件
local function RandChance(x)

	local decimal = math.random();
	if x > decimal then
		return true;
	else
		return false;
	end
end

--设置产生随机数的种子
local function SetRandSeed()
	math.randomseed(os.time());
	math.random(); --为来防止第一次调用的时候产生的随机数都是相等的问题
end

local function work( coro, usr )
        assert(usr);
        print(string.format("ID %d start ...", usr.id));
	if usr.id == 2 then --将id号为2的任务设置超时时间
		print(string.format("\x1B[1;31m".."set time out for task %d and won't  break loop after timeout".."\x1B[m",usr.id));
		coro:settimeout(3,false); --false代表任务超时不退出整个协程startup()函数，只放弃该任务
	end
	
        for i=1,usr.cycle do
		--在用户层使用切换函数的时候必须要判断切换函数返回的值并做相应的处理
		local p = math.random();
		if RandChance(p) then --根据这个随机数来决定是有随机睡眠的idleswitch()切换还是无随机睡眠的fastswitch()切换
			print(string.format("IDLE SWITCH ID %d : %d", usr.id, i));
			if not coro:idleswitch() then --切换返回false 说明coro已经在关闭状态，不再允许继续执行下面的代码，所以返回false任务异常结束
				print(string.format("coro already close, exit task:%d", usr.id));
				return false;
			end
		else
			print(string.format("FAST SWITCH ID %d : %d", usr.id, i));
			if not coro:fastswitch() then 
				print(string.format("coro already close, exit task:%d", usr.id));
				return false;
			end
		end
        end

        return usr.rc;
end

local index = 1;
local tasks = {
	{ id = 1,cycle = 2, index = 1,rc = true },
	{ id = 2,cycle = 4, index = 2,rc = true },
	{ id = 3,cycle = 6, index = 3,rc = true },
	{ id = 4,cycle = 8, index = 4,rc = true },
	{ id = 5,cycle = 3, index = 5,rc = true },
	{ id = 6,cycle = 5, index = 6,rc = true },
}
local function idle(coro, usr)
	if usr == true then  --usr为true代表选择空转idle在idle里面添加任务
		if index >  6 then
			print("\x1B[1;35m".."IDLE IDLE ~~~".."\x1B[m");
			if not coro:isactive() then
				coro:stop();
				print("\x1B[1;35m".."STOP LOOP".."\x1B[m");
			end
		else
			coro:addtask(work, coro, tasks[index]);
			index = index + 1;
			print("\x1B[1;35m".."IDLE IDLE ADD~~~".."\x1B[m");
		end
	else
		print("\x1B[1;35m".."IDLE~~~~".."\x1B[m");
	end
end

local function Addtask(coro, tasks)
	for i=1, 6 do
		coro:addtask(work, coro, tasks[i]);
	end
end

do
	SetRandSeed();

	local idleable;
	local coro = nil;

	if nil then
		coro = Coro:open(false); --不空转idle
		idleable = false;
		Addtask(coro, tasks);
	else
		idleable = true;
		coro = Coro:open(true); --空转idle,在idle中添加任务
	end

        if coro:startup(idle, coro, idleable) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m");
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m");
        end
--[[
	--如果当前coro中还存在未执行完毕的任务，可以唤醒coro继续执行
	if coro:isactive() then
		print("\x1B[1;33m".."coro is active and will restart".."\x1B[m");
		coro:startup(idle,coro,idleable);
	end
--]]--
        coro:close();
end
