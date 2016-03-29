package.path = "../?.lua"

local Pool = require("qpool")

local function create( usr )
	local element = {_id = usr._id, _capacity = usr._capacity, _counter = usr._counter}
	element._check = element._id + element._capacity + element._counter;
	print(string.format("create element.id:%d element.counter:%d", element._id, element._counter));
	return element;
end

local function destroy(element, usr)
	element = nil;
	print(string.format("destroy element in destroy function"));
	return ;
end

local function checker(element, usr)
	local check = element._id + element._capacity + element._counter;
	if check == element._check then
		print("examine element successed");
		return true;
	else
		print("examine element failed");
		return false;
	end
end

do 
	local name = "11"
	local pool_name = {};
	local container = nil; 
	local pool_capacity = 10;
	local container_capacity = 10; 
	
	container = Pool:pool_create( container_capacity ); --创建一个容器用来存放池子

	local i = 1;
	while i <= container_capacity do
		pool_name[i] = name;
		name = name + "11";
		local ok =container:pool_init(pool_name[i], pool_capacity, create, destroy, checker); --初始化一个新池子
		if not ok then
			break;
		end
		--print(string.format("init pool name:%d",pool_name[i]));
		i = i + 1;
	end

	local pool = {};
	for i=1, container_capacity do
		pool[i] = container:pool_gain(pool_name[i]); --根据池子名称获取池子
		if not pool[i] then
			break;
		end
		--print(string.format("gain pool :%s", pool[i]._name));
	end

	for i in pairs(pool) do
		local element = {};
		for k=1, pool[i]._capacity do
			local usr = { _counter = pool[i]._counter, _capacity = pool[i]._capacity, _id = k};
			 element[k] = container:pool_element_pull(pool[i], usr); --从池子里面获取数据，没有数据会调用callback函数创造新数据
			if not element[k] then
				print(string.format("pull element from \'%s\' pool failed", pool[i]._name));
				break ;
			end
			print(string.format("pull element from \'%s\'pool:elemet.id:%d", pool[i]._name, element[k]._id));

		end
		
		for k=1, pool[i]._capacity do
			local ok = container:pool_element_push(pool[i], element[k]); --数据用完之后放回到池子中
			if not ok then
				print(string.format("push element into \'%s\' pool failed", pool[i]._name));
				break;
			end
			print(string.format("push element into \'%s\'pool:elemet.id:%d", pool[i]._name, element[k]._id));
		end
		print("\n\n");
	end
end
