require("socket")

local function post_data(s)
	local fd_name = s
	local fd = io.open(fd_name, 'r')
	local a = 1
	while true do
		for line in fd:lines() do
			local tcp = socket.tcp()
	
			local ret = tcp:connect('127.0.0.1', 4100)
			local res
			local data = 'POST / HTTP/1.0\r\n' ..

	                'Host:127.0.0.1:4100\r\n' ..
	                'Content-Length:' .. tostring(#line) .. '\r\n' ..
	                'Content-Type:application/x-www-form-urlencoded\r\n\r\n' ..
	                line
		--	print(data)
			res = tcp:send(data)
			print(a)
			a = a + 1
			tcp:shutdown("both")
		
		end
	end
end

local s = arg[1]
post_data(s)
