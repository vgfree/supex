--local only = require('only')
--local redis_api = require('redis_pool_api')
--
--
--
--local ok,ret = redis_api.cmd("private", "get", "5858")
--ngx.say(ret)
--only.log('I', "hello world!")
--data = "sadsad"
--only.log('S', data)
--
--ngx.say("世界那么大，我想去走走")



local sock = ngx.socket.tcp()
local ok,err = sock:connect('127.0.0.1', 6379)
if not ok then
	ngx.say('Failed to connect whois server',err)
	return
end
--sock:settimeout(5000)
for i=1,5 do
	local ok, err = sock:send("*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n")
	if not ok then
		ngx.say('Failed to send data to whois server', err)
		return
	end
	local line, err, partial = sock:receive('*l')
	if not line then
		ngx.say('Failed to read a line', err)
		return
	end
	ngx.say(line)


	os.execute("sleep 5")
end
sock:setkeepalive()
