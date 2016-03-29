local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("msgsearchapi_utils")
local supex = require("supex")

module('getreadmessageinfo', package.seeall)

local function check_parameter(args)
        args["time"] = tonumber( args["time"] )
        if not args["time"] or string.len(args["time"]) ~= 10 then
                only.log("E", string.format("time is error. time:[%s]", args["time"]))
                return false, "time" 
        end
        if not args["bizid"] or string.len(args["bizid"]) ~= 34 then
                only.log("E", string.format("bizid is error. bizid:[%s]", args["bizid"]))
                return false, "bizid" 
        end
        return true
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

        local curTime = os.time()
        local key = string.format("readMessageInfo:%s:%s%d", args["bizid"], os.date("%Y%m%d%H", args["time"]), os.date("%M", args["time"])/10)
        local readmessageinfo
        local hashkey
        hashkey = args["bizid"]
        if args["time"] > curTime then
                readmessageinfo = ""
        elseif curTime - args["time"] <= dk_utils.redisRemainTime then
                ok, readmessageinfo = dk_utils.smembers_data_from_redis(key, hashkey)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        else
                ok, readmessageinfo = dk_utils.get_data_from_tsdb(key, args["time"], hashkey)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end
        --> return result.
        if not readmessageinfo then
                readmessageinfo = ""
        end
        only.log("D", "readMessageInfo: " .. readmessageinfo)
        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], readmessageinfo)
end 
