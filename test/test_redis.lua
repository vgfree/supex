redis = require("redis")




temp_arg = {"hello", "world"}

function task_cb(arg)
		for i,k in pairs(arg) do
				print(i,k)
		end
end

--block
function block()
		local client = redis.connect()
		local reply = client["dbsize"](client)
		print("block reply->" .. reply)
end

function nonblock()
		redis.reg_idle_cb(task_cb, temp_arg)

		local client = redis.connect()
		local reply = client["dbsize"](client)
		print("nonblock reply->" .. reply)
end

function nonblock_restore()
		redis.reg_idle_cb(task_cb, temp_arg)

		local client = redis.connect()
		local reply = client["dbsize"](client)
		print("nonblock reply->" .. reply)

		redis.reg_idle_cb(nil, nil)
		reply = client["dbsize"](client)
		print("nonblock restore reply->" .. reply)
end

-- case block
print("==================<Block test>========================")
block()
print("==================<Block test end>====================\n\n")

-- case nonblock
print("==================<Nonblock test end>=================")
nonblock()
print("==================<Nonblock test end>=================\n\n")

-- case nonblock then resotre
print("==================<Nonblock to restore test end>======")
nonblock_restore()
print("==================<Nonblock to restore test end>======\n\n")

