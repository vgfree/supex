local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("msgsearchapi_utils")
local supex = require("supex")

module('getsendinfo', package.seeall)

local function check_parameter(args)
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
        local key = string.format("sendInfo:%s", args["bizid"])
        local sendinfo
        local hashkey
        hashkey = args["bizid"]

        --> return result.
        ok, sendinfo = dk_utils.get_data_from_redis(key, hashkey)
        if not ok then
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
        if not sendinfo then
                ok, sendinfo = dk_utils.get_data_from_tsdb(key, curTime, hashkey)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end
        if not sendinfo then
                sendinfo = ""
        end
        only.log("D", "replyVoice: " .. sendinfo)
        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], sendinfo)
end 
