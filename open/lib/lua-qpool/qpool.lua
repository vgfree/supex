package.path = "../../?.lua"

local modename = "qpool"
local Pool = {}
_G[modename] = Pool
package.loaded[modename] = Pool

local Queue = require("queue")

--创建一个容器用于存放池子
function Pool:pool_create( capacity )
	local pool = {
			_pool = {},		--用于存放池子的容器
			_capacity = capacity,	--存放池子容器的容量
			_counter = 0		--存放池子容器的使用量
		};
	setmetatable(pool, self);
	self.__index = self;
	return pool;
end

--初始化一个池子,失败返回false， 成功返回true
function Pool:pool_init(name, max, createcb, destroycb, checkercb)
	assert( name );
	local ret = false;
	if self._counter < self._capacity then
		if self._pool[name] == nil then
			self._pool[name] = {
				_name = name;			--池子的名称
				_capacity = max,		--池子的容量
				_queue = Queue:new( max ),	--池子中用于存放数据的队列
				_counter = 0,			--池子使用量的计数
				_createcb = createcb,		--用于创建数据的回调函数
				_destroycb = destroycb,		--用于销毁数据的回调函数
				_checkercb = checkercb		--用于检测数据的回调函数
			};
			self._counter = self._counter + 1;
			ret = true;
		else
			print(string.format("name of \'%s\' pool already existed", name));
		end
	else
		print(string.format("no more space for new pool in container"));
	end

	return ret;
end

--根据池子的名称来获取池子,失败返回nil，成功返回对应的池子
function Pool:pool_gain( name )
	assert( name );
	if self._pool[name] ~= nil then
		return self._pool[name];
	else
		print(string.format("no name of \'%s\' pool", name));
		return nil;
	end
end

--从池子里面取出数据，如果没有数据，则创建一个,失败返回nil，成功返回数据
function Pool:pool_element_pull( pool,  usr )
	assert( pool );
	local element  = nil ;
	while 1 do
		element = pool._queue:pull();
		if element then
			if pool._checkercb then		--如果有检测数据的回调函数则进行检测
				if pool._checkercb(element, usr) then
					return element;
				else			--检测数据不合格，则将此数据释放，然后继续取数据
					self:pool_element_free(pool, element, usr);
				end
			else
				return element;
			end
		else
			break;
		end
	end

	--从队列里面获取数据失败，则根据用户提供的参数usr创建一个数据
	if pool._counter < pool._capacity then
		element = pool._createcb( usr );
		if not element then
			print(string.format("pull data from \'%s\' pool failed", pool._name));
		else
			pool._counter = pool._counter + 1;
		end
	else
		print(string.format("pool \'%s\' is full", pool._name));
	end

	return element;
end

--往池子里面添加一个数据,失败返回false,成功返回true
function Pool:pool_element_push(pool, element)
	assert( pool and element );
	local ok = false;
	ok  = pool._queue:push(element);
	if not ok  then
		print(string.format("push a element into \'%s\' pool failed", pool._name));
	end
	return ok;
end

--释放池子中的某个数据
function Pool:pool_element_free(pool, element, usr)
	pool._counter = pool._counter - 1;
	if pool._destroycb then
		pool._destroycb(element, usr);
	end
end

