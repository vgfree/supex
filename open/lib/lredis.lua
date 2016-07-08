local lredis = {
	_VERSION     = 'lredis-lua 0.1.0-dev',
	_DESCRIPTION = 'A Lua client library for redis.',
	_COPYRIGHT   = 'Copyright (C) 2016 daoke.me',
}

local byte = string.byte
local sub = string.sub

local commands = {
    "append",            "auth",              "bgrewriteaof",
    "bgsave",            "bitcount",          "bitop",
    "blpop",             "brpop",
    "brpoplpush",        "client",            "config",
    "dbsize",
    "debug",             "decr",              "decrby",
    "del",               "discard",           "dump",
    "echo",
    "eval",              "exec",              "exists",
    "expire",            "expireat",          "flushall",
    "flushdb",           "get",               "getbit",
    "getrange",          "getset",            "hdel",
    "hexists",           "hget",              "hgetall",
    "hincrby",           "hincrbyfloat",      "hkeys",
    "hlen",
    "hmget",             --[[ "hmset", ]]     "hscan",
    "hset",
    "hsetnx",            "hvals",             "incr",
    "incrby",            "incrbyfloat",       "info",
    "keys",
    "lastsave",          "lindex",            "linsert",
    "llen",              "lpop",              "lpush",
    "lpushx",            "lrange",            "lrem",
    "lset",              "ltrim",             "mget",
    "migrate",
    "monitor",           "move",              "mset",
    "msetnx",            "multi",             "object",
    "persist",           "pexpire",           "pexpireat",
    "ping",              "psetex",       --[[ "psubscribe", ]]
    "pttl",
    "publish",      --[[ "punsubscribe", ]]   "pubsub",
    "quit",
    "randomkey",         "rename",            "renamenx",
    "restore",
    "rpop",              "rpoplpush",         "rpush",
    "rpushx",            "sadd",              "save",
    "scan",              "scard",             "script",
    "sdiff",             "sdiffstore",
    "select",            "set",               "setbit",
    "setex",             "setnx",             "setrange",
    "shutdown",          "sinter",            "sinterstore",
    "sismember",         "slaveof",           "slowlog",
    "smembers",          "smove",             "sort",
    "spop",              "srandmember",       "srem",
    "sscan",
    "strlen",       --[[ "subscribe", ]]      "sunion",
    "sunionstore",       "sync",              "time",
    "ttl",
    "type",         --[[ "unsubscribe", ]]    "unwatch",
    "watch",             "zadd",              "zcard",
    "zcount",            "zincrby",           "zinterstore",
    "zrange",            "zrangebyscore",     "zrank",
    "zrem",              "zremrangebyrank",   "zremrangebyscore",
    "zrevrange",         "zrevrangebyscore",  "zrevrank",
    "zscan",
    "zscore",            "zunionstore",       "evalsha",
    "geoadd",            "geopos",            "geodist",
    "georadius",         "georadiusbymember", "geohash"
}

--module('lredis', package.seeall)

function shallowcopy(orig)
	local orig_type = type(orig)
	local copy
	if orig_type == 'table' then
		copy = {}
		for orig_key, orig_value in pairs(orig) do
			copy[orig_key] = orig_value
		end
	else -- number, string, boolean, etc
		copy = orig
	end
	return copy
end

local function deepcopy(orig)
	local orig_type = type(orig)
	local copy
	if orig_type == 'table' then
		copy = {}
		for orig_key, orig_value in next, orig, nil do
			copy[deepcopy(orig_key)] = deepcopy(orig_value)
		end
		setmetatable(copy, deepcopy(getmetatable(orig)))
	else -- number, string, boolean, etc
		copy = orig
	end
	return copy
end

-- ############################################################################

local network = {}

function network.write(client, buffer)
	local byte, err = client.network.socket:send(buffer)
	if err then error('lredis network send error:' .. err) end
	return byte
end

function network.read(client, len)
  local res, err, part
	if len == nil then
    len = '*l'
    res, err = client.network.socket:receive(len)
  else
    res, err, part = client.network.socket:receive(len)
    if part == '' then part = nil end
    res = res or part
  end

	local ok = res and true or false
	if ok then
		return ok, res
	else
		return ok, err
	end
end

-- ############################################################################

local defaults = {
	host        = '127.0.0.1',
	port        = 80,
	tcp_nodelay = true,
}

local idle_fcb, idle_arg = nil, nil

-- triggered in nonblock status
function lredis.reg_idle_cb(fcb, arg)
	assert(type(fcb) == "nil" or type(fcb) == "function", "'fcb' should be function type!")
	local old_idle_fcb, old_idle_arg = idle_fcb, idle_arg

	idle_fcb = fcb
	idle_arg = arg
	return old_idle_fcb, old_idle_arg
end

local function gen_req(args)
  local nargs = #args
  local req = '*' .. nargs .. '\r\n'

  for i = 1, nargs do
    local arg = args[i]
    if type(arg) ~= 'string' then
      arg = tostring(arg)
    end
    req = req .. '$' .. #arg .. '\r\n' .. arg .. '\r\n'
  end

  return req
end

local function readline(client)
	while true do
		local ok, line = client.network.read(client)
		if not ok then
			if line == "timeout" then
				--EWOULDBLOCK
				if idle_fcb then
					idle_fcb(idle_arg)
				end
      else
        error('lredis network read error:' .. line);
			end
		else
      return line
    end
  end
end

local function read(client, size)
  local data = ''
  local data_size = 0
  local recv_size = size

	while true do
		local ok, part = client.network.read(client, recv_size)
		if not ok then
			if part == "timeout" then
				--EWOULDBLOCK
				if idle_fcb then
					idle_fcb(idle_arg)
				end
      else
        error('lredis network read error: ' .. part);
			end
		elseif part then
      data = data .. part

      local part_size = #part
      data_size = data_size + part_size

      local rest_size = size - data_size
      if rest_size == 0 then
        return data
      end

      recv_size = rest_size
    end
  end
end

local function read_reply(client)
  local line = readline(client)
  local prefix = byte(line)

  if prefix == 36 then    -- char '$'
    --print("bulk reply")
 
    local size = tonumber(sub(line, 2))
    --print(size)
    if size < 0 then
      error('lredis illegal bulk string length $' .. size);
    end
 
    local data = read(client, size)

    -- \r\n
    read(client, 2)
  
    return data
 
  elseif prefix == 43 then    -- char '+'
    --print("status reply")
 
    return sub(line, 2)
 
  elseif prefix == 42 then -- char '*'
    local n = tonumber(sub(line, 2))
 
    --print("multi-bulk reply: ", n)
    if n < 0 then
      error('lredis illegal multi-bulk string line size $' .. n);
    end
 
    local vals = {}
    local nvals = 0
    for i = 1, n do
      local res = read_reply(client)
      nvals = nvals + 1
      vals[nvals] = res
    end
 
    return vals
 
  elseif prefix == 58 then    -- char ':'
    -- print("integer reply")
    return tonumber(sub(line, 2))
 
  elseif prefix == 45 then    -- char '-'
    -- print("error reply: ", n)
    error('lredis reply error: ' .. sub(line, 2))
     
  else
    -- when `line` is an empty string, `prefix` will be equal to nil.
    error('unkown prefix: ' .. tostring(prefix))
  end
end

local function custom_request(client, ...)
  local args = {...}
  if #args > 0 then
    local req = gen_req(args)
		repeat
			local out = client.network.write(client, req)
      --print('send: ' .. out)
			req = string.sub(req, out + 1, -1)
		until (#req == 0)
	else
		client.error('no command and arguments')
	end

	local idle_fcb = idle_fcb
	local idle_arg = idle_arg
	if idle_fcb then
		client.network.socket:settimeout(0) --noblock
	else
		client.network.socket:settimeout(nil) -- restore to block indefinitely
	end

  local ret = read_reply(client)
  return ret
end


local function merge_defaults(parameters)
	if parameters == nil then
		parameters = {}
	end
	for k, v in pairs(defaults) do
		if parameters[k] == nil then
			parameters[k] = defaults[k]
		end
	end
	return parameters
end

local function parse_boolean(v)
	if v == '1' or v == 'true' or v == 'TRUE' then
		return true
	elseif v == '0' or v == 'false' or v == 'FALSE' then
		return false
	else
		return nil
	end
end




local function load_methods(proto, commands)
	local client = setmetatable ({}, getmetatable(proto))

	for cmd, fn in pairs(commands) do
		if type(fn) ~= 'function' then
			lhttp.error('invalid type for command ' .. cmd .. '(must be a function)')
		end
		client[cmd] = fn
	end

	for i, v in pairs(proto) do
		client[i] = v
	end

	return client
end

local function create_client(proto, client_socket, commands)
	local client = load_methods(proto, commands)
	client.error = lredis.error
	client.network = {
		socket = client_socket,
		read   = network.read,
		write  = network.write,
	}
	return client
end

local client_prototype = {}

local function connect_tcp(socket, parameters)
	local host, port = parameters.host, tonumber(parameters.port)
	if parameters.timeout then
		socket:settimeout(parameters.timeout, 't')
	end

	local ok, err = socket:connect(host, port)
	if not ok then
		error('could not connect to '..host..':'..port..' ['..err..']')
	end
	socket:setoption('tcp-nodelay', parameters.tcp_nodelay)
	return socket
end

local function connect_unix(socket, parameters)
	local ok, err = socket:connect(parameters.path)
	if not ok then
		lhttp.error('could not connect to '..parameters.path..' ['..err..']')
	end
	return socket
end

local function create_connection(skt_class, parameters)
	if parameters.socket then
		return parameters.socket
	end

	local perform_connection, socket

	if parameters.scheme == 'unix' then
		perform_connection, socket = connect_unix, skt_class.unix
		--perform_connection, socket = connect_unix, require('socket.unix')
		assert(socket, 'your build of LuaSocket does not support UNIX domain sockets')
	else
		if parameters.scheme then
			local scheme = parameters.scheme
			assert(scheme == 'lredis' or scheme == 'tcp', 'invalid scheme: '..scheme)
		end
		perform_connection, socket = connect_tcp, skt_class.tcp
	end

	return perform_connection(socket(), parameters)
end

-- ############################################################################

function lredis.error(message, level)
	error(message, (level or 1) + 1)
	print("===================")
end

function lredis.connect(skt_class, ...)
	local args, parameters = {...}, nil

	local skt_class = shallowcopy(skt_class)
	if #args == 1 then
		if type(args[1]) == 'table' then
			parameters = args[1]
		else
			local uri = skt_class.url
			--local uri = require('socket.url')
			parameters = uri.parse(select(1, ...))
			if parameters.scheme then
				if parameters.query then
					for k, v in parameters.query:gmatch('([-_%w]+)=([-_%w]+)') do
						if k == 'tcp_nodelay' or k == 'tcp-nodelay' then
							parameters.tcp_nodelay = parse_boolean(v)
						elseif k == 'timeout' then
							parameters.timeout = tonumber(v)
						end
					end
				end
			else
				parameters.host = parameters.path
			end
		end
	elseif #args > 1 then
		local host, port, timeout = unpack(args)
		parameters = { host = host, port = port, timeout = tonumber(timeout) }
	end

	local commands = lredis.commands or {}
	if type(commands) ~= 'table' then
		lredis.error('invalid type for the commands table')
	end

	local socket = create_connection(skt_class, merge_defaults(parameters))
	local client = create_client(client_prototype, socket, commands)

	return client
end

local cmds = {}

function cmds.hmset(client, hashname, ...)
  local args = {...}
  if #args == 1 then
    local t = args[1]

    local n = 0
    for k, v in pairs(t) do
        n = n + 2
    end

    local array = {}

    local i = 0
    for k, v in pairs(t) do
      array[i + 1] = k
      array[i + 2] = v
      i = i + 2
    end
    -- print("key", hashname)
    return custom_request(self, "hmset", hashname, unpack(array))
  end

  -- backwards compatibility
  return custom_request(self, "hmset", hashname, ...)
end

for i = 1, #commands do
  local cmd = commands[i]

  cmds[cmd] =
    function (client, ...)
      return custom_request(client, cmd, ...)
    end
end

lredis.commands = cmds

return lredis
