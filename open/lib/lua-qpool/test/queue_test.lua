package.path = "../../?.lua"

local Queue = require("queue")

local function destroy( element )
	print(string.format("element.cycle:%d element.rc:%d", element.cycle, element.rc));
end

do
	local index = 1;
	local queue = Queue:new( 10000 );

	while index < 10005 do
		local task = {cycle = 0, rc = 1 };
		task.cycle = index;
		task.rc = task.cycle +1;
		local ok = queue:push(task); 
		if not ok then
			break;
		end
		index = index +1;
----[[
		if i == 10 or i == 5 then
			local value = nil;
			value = queue:pull();
			if value == nil then 
				break;
			end
			print(string.format("pull value in push:%d, %d", value.cycle, value.rc));
		end
--]]--
	end

	index = 1;
	while index < 10005 do
		local value = nil
		value = queue:pull();
		if value == nil then 
			print("break");
			break;
		end
		print(string.format("pull value:%d, %d", value.cycle, value.rc));
		local ok  = queue:push(value)
		if not ok then
			print("push error in pull");
			break;
		end
		index = index +1;
	end

	queue:free( destroy );
end
