local modename = "coro"
local Coro = {}
_G[modename] = Coro
package.loaded[modename] = Coro

local ffi = require("ffi");
ffi.cdef[[
	struct timeval{
		unsigned long int tv_sec;
		unsigned long int tv_usec;
	};
        int usleep(unsigned int p);
        unsigned int sleep(unsigned int m);
	int gettimeofday(struct timeval *tv,void *tz);
]];

local Ring = require("ring")

local TASKSTAT = {
        INITIAL = 1,
        RUNING = 2,
        EXITED = 3,
        EXCEPT = 4,
	TIMEOUT = 5,
}

local COROCONFIGURE = {
	BLURRED = 1,	--是否模糊休眠[关闭时设置为nil] 
}


--获取一个小数的整数部分
local function getInteger(x)

	local temp = math.ceil(x);

	if x <= 0 or temp == x then
		return temp;
	elseif temp > x then
		return temp - 1;
	end
end

--测试一次事件为p的概率
local function RandChance(x)

	local decimal = math.random(); --无参数的时候获取的随机数范围为[0-1]
	if x > decimal then
		return true;
	else
		return false;
	end
end

--设置产生随机数的种子
local function SetRandSeed()
	math.randomseed(os.time());
	math.random(); --为来防止第一次调用的时候产生的随机数都相等的问题
end

--获取休眠的时间[微秒级别]
local function getmicrosecond( p, idlecnt )

	local bool = RandChance(p);
	local x = 0;
	if bool then
		x = math.random(idlecnt);
		x = x*math.pow(2,10);
		return bool, x;
	else
		return bool, x;
	end
end

--获取系统当前的时间[微秒级别]
local function gettimeofday()

	local tm = ffi.new("struct timeval");
	ffi.C.gettimeofday(tm,nil);
	local sec =  tonumber(tm.tv_sec);
	local usec =  tonumber(tm.tv_usec);
	return sec, usec;
end

--检测是否超时
local function detecttimeout( task )

	local sec, usec = gettimeofday();
	if  sec > task._tv.sec or sec == task._tv.sec and usec > task._tv.usec then
		print(string.format("\x1B[1;31m".."[ detecttimeout ] task %d timeout for %d seconds and %d microseconds" 
					.."\x1B[m",task._user.id,sec - task._tv.sec,usec - task._tv.usec));
		task._stat = TASKSTAT.TIMEOUT;
		coroutine.yield();
	end 
end


--创建一个新的协程任务
local function newtask( callback, corptr, user )
        -- body
        assert(callback);
        local task = { 
                _callback = callback,
                _user = user,
                _coro = nil,
		_stoploop = nil,		--超时之后是否停止整个协程
		_to = 0,			--是否设置超时
		_tv = { sec = 0, usec = 0 },
                _stat = TASKSTAT.INITIAL,	--是否完成
		_self = corptr,			--所属的对象
                };
        task._coro = coroutine.create(function ( task )
		-- body
		local close = nil;
		task._stat = TASKSTAT.RUNING;
		close =  coroutine.yield(); --close为true则代表此任务由close()函数唤醒，协程为close状态
		if close then
			task._stat = TASKSTAT.EXCEPT; --协程为close状态，则不执行用户层的callback函数直接退出
		else
			local ok, result = pcall(task._callback, task._self, task._user);
			if not ok then
				print(result);
				task._stat = TASKSTAT.EXCEPT;
			elseif not result then
				task._stat = TASKSTAT.EXCEPT;
			else
				task._stat = TASKSTAT.EXITED;
			end
		end
	end);
        assert(task._coro);
        return task;
end

--打开一个协程
function Coro:open( idleable )
        -- body
        local coro = { _result = nil, _stop = false,  _idleable = idleable, _idlecnt = 0, _tasks = Ring:new() };
	local usr = {id = 0}
	local task = {_callback = nil, _user = usr, _coro = nil};
	coro._tasks:append( task ); --添加一个哑元表作为head，以此判断协程是否循环了一轮
        setmetatable(coro, self);
        self.__index = self;
        return coro;
end

--添加协程任务
function Coro:addtask( callback, corptr, user )
        -- body
	local task = nil;
	if self._stop then
		print(string.format("coro already stoped, add task forbidden"));
	else
		task = newtask(callback, corptr, user);
		coroutine.resume(task._coro, task, false);
		self._tasks:append(task);
	end
end

--启动协程，开始执行所添加的任务或者用户提供的idle函数
function Coro:startup( idle, corptr, user )
        -- body
        local start = nil;
	local taskcnt = self._tasks:gettaskcnt() - 1;
        self._result = true;
	self._stop = false;
	
	if COROCONFIGURE.BLURRED then
		SetRandSeed();
	end

        while taskcnt > 0  or self._idleable and idle do
                local del = nil;
                local task = self._tasks:getdata();
		taskcnt = self._tasks:gettaskcnt() - 1; --任务链表中一个哑元任务排除在外

		if self._stop then
			break;
		end

		if taskcnt < 1 and self._idleable and idle then 
                        pcall(idle, corptr, user);
			if COROCONFIGURE.BLURRED and self._idlecnt > 0 then
				local p = self._idlecnt/taskcnt;
				local bool, ms = getmicrosecond(p,self._idlecnt);
				if bool then
					print(string.format("\x1B[1;35m".."now start to sleep for %d microseconds".."\x1B[m",ms));
					ffi.C.usleep(ms);
				end
				self._idlecnt = 0;
			end
		else
			if task == self._tasks:gethead() then
				pcall(idle, corptr, user);
				if COROCONFIGURE.BLURRED and self._idlecnt > 0 then
					local p = self._idlecnt/taskcnt;
					local bool, ms = getmicrosecond(p,self._idlecnt);
					if bool then
						print(string.format("\x1B[1;35m".."now start to sleep for %d microseconds".."\x1B[m",ms));
						ffi.C.usleep(ms);
					end
					self._idlecnt = 0;
				end
			else
				assert(task._stat == TASKSTAT.RUNING);
				coroutine.resume(task._coro, false); --false代表在startup里面唤醒协程，协程不为close状态
				if task._stat == TASKSTAT.EXCEPT then
					self._result = false;
					del = self._tasks:deleteprevious();
					print(string.format("\x1B[1;31m".."task except and break loop".."\x1B[m"));
					break;
				elseif task._stat == TASKSTAT.TIMEOUT then
					self._result = false;
					del = self._tasks:deleteprevious();
					if task._stoploop then
						print(string.format("\x1B[1;31m".."task timeout and break loop".."\x1B[m"));
						break;
					end
				elseif task._stat == TASKSTAT.EXITED then
					del = self._tasks:deleteprevious();
				end
			end
			self._tasks:forwardstep();
		end

        end

        return self._result;
end

--无睡眠的切换，协程为close状态的时候，返回false用户层停止运行,否则返回true，用户层正常运行
function Coro:fastswitch( ... )
	--body
	local task = self._tasks:getdata();
	if task._to > 0 then
		detecttimeout(task);
	end
	local close = coroutine.yield();
	if close then  --close的状态为true，则返回false给用户通知用户停止运行
		return false;
	else
		return true;
	end
end

--随机睡眠的切换，协程为为close状态的时候，返回false用户层停止运行,否则返回true，用户层正常运行
function Coro:idleswitch( ... )
	--body
	local task = self._tasks:getdata();
	self._idlecnt = self._idlecnt + 1;

	if task._to > 0 then
		detecttimeout(task);
	end
	local close = coroutine.yield();
	if close then --close的状态为true，则返回false给用户通知用户停止运行
		return false;
	else
		return true; 
	end
end

--设置任务超时，timeout单位为ms, stoploop为true时超时则startup()函数退出循环，放弃所有的任务,否则只放弃该任务
function Coro:settimeout( timeout, stoploop )
	--body
	local ms = 0;
	local sec, usec = gettimeofday();
	local task = self._tasks:getdata();

	ms = timeout * 1000;
	sec = sec + ms/1000000;
	usec = usec + ms%1000000;
	task._to = timeout;
	task._tv.sec = getInteger(sec);
	task._tv.usec = usec;
	task._stoploop = stoploop;
end

--检测协程中是否还存在任务，返回true代表任务链表中还存在未执行完的任务,false则代表无待执行任务
function Coro:isactive( ... )
        -- body
	local taskcnt = self._tasks:gettaskcnt() - 1; --任务链表里面有一个哑元任务排除在外
        return taskcnt > 0 and true or false;
end

--手动停止协程，停止之后不可以添加任务，可以使用startup()函数重启协程
function Coro:stop( ... )
	--body
	self._stop = true;
end

--close协程之前将任务链表里面的任务栈销毁
function Coro:close( ... )
        -- body
	self._stop = true;
	local taskcnt = self._tasks:gettaskcnt() - 1; --任务链表里面有一个哑元任务排除在外
	while taskcnt > 0 do
		local task = self._tasks:getdata();
		if task == self._tasks:gethead() then
			self._tasks:forwardstep();
		else
			coroutine.resume(task._coro, true); --true代表在close里面唤醒协程, 协程为close状态
			if task._stat == TASKSTAT.EXITED or task._stat == TASKSTAT.TIMEOUT or task._stat == TASKSTAT.EXCEPT then
				del = self._tasks:deleteprevious();
				self._tasks:forwardstep();
			end
		end
		taskcnt = self._tasks:gettaskcnt() - 1;
	end
        self._tasks:free();
end
