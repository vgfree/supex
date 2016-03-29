local scan              = require('scan')
local supex_http        = _G.supex_http
local supex_say         = _G.app_lua_add_send_data

module("supex", package.seeall)

__SERV_NAME__      = app_lua_get_serv_name()

_FINAL_STAGE_    = false
_SOCKET_HANDLE_  = 0
__TASKER_SCHEME__  = 0

--[[
function luakviter(redname, hashkey, ...)
        return pcall(_G.luakv_iter, ...)
end
]]--
--[[
function luakv(redname, hashkey, ...)
        local ok, result = pcall(_G.luakv_cmd, ...)
        if not ok then
                only.log("E", string.format("call supex.cmd(...) fail by %s: %s",
                        scan.dump(...), result))
        end
        return ok, result
end
]]--


--[[
@args 接受如下形式的参数
1.run('cmd keyvalue1, keyvalue2, ...')
2.run('cmd', 'keyvalue1', 'keyvalue2', ...)
3.run({{ 'cmd1', 'keyvaluse1', 'keyvaluse2', ... }, { 'cmd2', 'keyvaluse1', 'keyvaluse2', ... }, ...})
每个命令的操作子可以是字符串，也可以是数字。
@return true / false, error or table(表的形式如: { {result, ...}, { result, ... }, ... })
]]
function luakv( redname, hashkey, ... )
        -- collectgarbage()

        local transresult = {
                _tonumber_ = function ( result )
                        return tonumber(result[1])
                end,

                _boolean_ = function ( result )
                        return result and true or false
                end,

                _ontrans_ = function ( result )
                        return result
                end,

                _first_ = function ( result )
                        return result[1]
                end,
        }

        local transresult = setmetatable({

                sadd            = transresult._tonumber_,
                del             = transresult._tonumber_,
                dbsize          = transresult._tonumber_,
                incr            = transresult._tonumber_,
                incrby          = transresult._tonumber_,
                decr            = transresult._tonumber_,
                decrby          = transresult._tonumber_,
                lpush           = transresult._tonumber_,
                expire          = transresult._tonumber_,
                expireat        = transresult._tonumber_,
                pexpire         = transresult._tonumber_,
                pexpireat       = transresult._tonumber_,

                smembers        = transresult._ontrans_,
                lrange          = transresult._ontrans_,
                set             = transresult._boolean_,
                flushdb         = transresult._boolean_,
                exists          = transresult._boolean_,

                get             = transresult._first_,

        }, { __index = transresult })

        --临时 模拟 sismember
        local sismember = function ( cmd )

                local _, _, cmd, key, value = string.find(cmd, '^%s*(%S+)%s+(%S+)%s+(%S+)%s*$')

                for result in _G.luakv_ask('smembers ' .. key) do
                        if result == tostring(value) then
                                return true
                        end
                end

                return false
        end

        --[[
        runcmd('set key value');
        or
        runcmd('set', 'key', 'value');
        ]]
        local runcmd = function ( ... )
                local ok, command = pcall(table.concat, {...}, ' ')

                if not ok then
                        return ok, command
                end

                -- command = string.lower(command)
                local _, _, oper = string.find(command, '^%s*(%S+)%s*')
                oper = oper or '@'
                oper = string.lower(oper)

                local ok, value = nil, nil
                local result = setmetatable({}, { __mode = 'kv' })
                if oper == 'sismember' then
                        result = sismember(command)

                else
                        for value in _G.luakv_ask(command) do
                                result[#result + 1] = value
                        end
                        --[[根据命令进行数据转换]]
                        result = (transresult[oper] or transresult._ontrans_)(result);

                end

                return result
        end

        local ok, result
        if type(...) == 'table' then
                local results = setmetatable({}, { __mode = 'kv' })
                for i, subtab in ipairs(...) do
                        if type(subtab) ~= 'table' then
                                only.log("E", "error args to call supex.cmd(...)")
                                break;
                        end
                        ok, result = pcall(runcmd, unpack(subtab))
                        if not ok then
                                only.log("E", string.format("call supex.cmd(...) fail by %s: %s",
                                        scan.dump(...), result))
                                return ok, result
                        end

                        results[i] = result
                end
                return ok, results
        else
                ok, result = pcall(runcmd, ...)
                if not ok then
                        only.log("E", string.format("call supex.cmd(...) fail by %s: %s",
                                scan.dump(...), result))
                end
                return ok, result
        end
end

--
-- http function
--
function http(host, port, data, size)
        return supex_http(__TASKER_SCHEME__, host, port, data, size)
end

function spill(data)
        return supex_say(_SOCKET_HANDLE_, data)
end

function rgs( status )
        local afp = { }
        setmetatable(afp, { __index = _M })
        afp.status = status

        afp.fsize = 0
        afp.fdata = { }

        return afp
end

function say(afp, data)
        table.insert(afp.fdata, data)
        afp.fsize = afp.fsize + data:len()
end

function over(afp)
        --> update body
        local body = table.concat(afp.fdata)
        --> update head
        local head = string.format('HTTP/1.1 %s OK\r\nServer: supex/1.0.0\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n', afp.status, afp.fsize)
        --> flush data
        local data = head .. body
        return supex_say(_SOCKET_HANDLE_, data)
end
