
local common_path = '../../../open/lib/?.lua;'
local cpath = '../../../open/lib/?.so;'
package.path = common_path .. package.path
package.cpath = cpath .. package.cpath

local socket = require('socket')
local redis = require('redis')
local sha = require('sha1')
local json = require('cjson')

local cfg = require('config_for_test')


local function http(cfg, data)
	local tcp = socket.tcp()
	if tcp == nil then
		error('load tcp failed!')
		return false
	end
	tcp:settimeout(10000)
	local ret = tcp:connect(cfg["host"], cfg["port"])
	if ret == nil then
		error("connect failed!")
		return false
	end

	tcp:send(data)
	local result = tcp:receive("*a")
	tcp:close()
	return result
end

local function get_data(body)
	return "POST /" .. cfg.path .. " HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", cfg.host, cfg.port) ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

local function gen_sign(T, secret)
    local kv_table = {}
    for k,v in pairs(T) do
        if type(v) ~= "table" then
            if k ~= "sign" then
				print(k)
                table.insert(kv_table, k)
            end
        end
    end
    table.insert(kv_table, "secret")
    table.sort(kv_table)
    local sign_string = kv_table[1] .. T[kv_table[1]]
    for i = 2, #kv_table do
        if kv_table[i] == 'secret' then
            sign_string = sign_string .. kv_table[i] .. secret
        else
            sign_string = sign_string .. kv_table[i] .. T[kv_table[i]]
        end
    end

    print(sign_string)
    local result = sha.sha1(sign_string)
    local sign_result = string.upper(result)

    return sign_result
end

local function get_body()
    status, res = pcall(json.decode, cfg.body)
    if status == nil then
        return
    end
--	print(type(res), status)
    local status, redis_conn
    status, redis_conn = pcall(redis.connect, '127.0.0.1', 6379)

    --get secret--
 -- 
    local secret = redis_conn:hget(res['appKey'] .. ':appKeyInfo', 'secret')
    if not secret then
	print("failed secret",secret)
        return
    end

    res.sign = gen_sign(res, secret)
--]]
	for k, v in pairs(res) do
        if type(v) == 'table' then
            res[k] = json.encode(v)
        end
    end

    local http_body = ''

    for k, v in pairs(res) do
        http_body = http_body .. string.format('%s=%s&', k, v)
    end

--	print('before : ' .. http_body)
	http_body = string.sub(http_body, 1 , -2)
--	print('later : ' .. http_body)
	return http_body
end

local body = get_body()

local data = get_data(body)
print(data)
local info = http(cfg, data)
print(info)
