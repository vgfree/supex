package.path = "../?.lua;"

local Coro = require("coro");


local function work( coro, usr )
        assert(usr)
        print(string.format("ID %d start ...", usr.id))

	print("send data")
	print(coro.fastswitch)
	coro:fastswitch()
	print("recv data")

        return usr.rc
end



local function idle( coro, idleable )
	if not idleable then 
		print("\x1B[1;35m".."IDLE~~~~".."\x1B[m")
	else
		if coro:isactive() then
			print("\x1B[1;35m".."LOOP~~~~".."\x1B[m")
			coro:fastswitch()
		else
			print("\x1B[1;33m".."coro:stop()".."\x1B[m")
			coro:stop()
			return
		end
	end
end





local tasks = {
	{ id = 1,cycle = 2, index = 1,rc = true },
	{ id = 2,cycle = 4, index = 2,rc = true },
	{ id = 3,cycle = 6, index = 3,rc = true },
	{ id = 4,cycle = 8, index = 4,rc = true },
	{ id = 5,cycle = 3, index = 5,rc = true },
	{ id = 6,cycle = 5, index = 6,rc = true },
}


----------------------------------------------------------------------
do
	local coro = Coro:open(true)
	coro:addtask(work, coro, tasks[1])
	coro:addtask(work, coro, tasks[2])
	coro:addtask(work, coro, tasks[3])

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end
