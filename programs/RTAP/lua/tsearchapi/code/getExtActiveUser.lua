local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("tsearchapi_utils")
local supex = require("supex")

module('getExtActiveUser', package.seeall)

local function check_parameter(args)
        args["time"] = tonumber( args["time"] )
        only.log('D',args["time"])
        if not args["time"] or string.len(args["time"]) ~= 10 then
                only.log("E", "time is error!")
                return false, "time" 
        end
        return true
end
local function get_data_from_redis(key)
        local users = nil
        local user_count = 0
        local tsdb_redis_name
        for i=1, dk_utils.imeiShardingSize do
                tsdb_redis_name = string.format("%s%s", dk_utils.tsdbRedisNamePrefix, i-1)
                local ok, tab = redis_api.only_cmd(tsdb_redis_name, "smembers", key)
                if not ok then
                        only.log("E", "smembers tsdb_redis error.")
                        return false
                end
                if tab and #tab > 0 then
                        user_count = user_count + #tab
                        if users then
                                users = string.format("%s|%s", users, table.concat(tab, "|"))
                        else
                                users = string.format("%s", table.concat(tab, "|"))
                        end
                end
        end

        if not users then
                users = ""
        else
                users = string.format("%d*1@%s|", user_count, users)
        end
        
        return true, users
end

local function get_data_from_tsdb(key, time)
        local users = ""
        local tsdb_name = dk_utils.get_tsdb_name(time, nil)
        only.log("D", "tsdb_name: %s", tsdb_name)
        local ok, users = redis_pool_api.only_cmd(tsdb_name, "get", key)
        if not ok then
                only.log("E", "smembers tsdb error.")
                return false
        end

        return true, users
end

function handle()
        local args = nil 

        if supex.get_our_body_data() then
                args = supex.get_our_body_table()
        else
                args = supex.get_our_uri_table()
        end

        -->> check parameters.
        local ok, errlog = check_parameter(args)
        if not ok then
                return gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], errlog)
        end

        -->> get data.
        local key = string.format("RACTIVEUSER:%s%d", os.date("%Y%m%d%H", args["time"]), os.date("%M", args["time"])/10)
        local users
        local curTime = os.time()
        only.log("D", "get active users:" .. key)
        if args["time"] > curTime then
                users = ""
        elseif curTime - args["time"] <= dk_utils.redisRemainTime then
                ok, users = get_data_from_redis(key)
        else
                ok, users = get_data_from_tsdb(key, args["time"])
        end

        if not ok then
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
                
        --> return result.
        if not users then
                users = ""
        end
        only.log("D", "active user: " .. users)
        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], users)
end 
