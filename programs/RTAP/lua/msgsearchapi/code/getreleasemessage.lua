local utils = require("utils")
local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("msgsearchapi_utils")
local supex = require("supex")

module('getreleasemessage', package.seeall)

local function check_parameter(args)
        if (not utils.is_number( args["appKey"] )) or #args["appKey"] > 10 then
                only.log("E", string.format("appKey is error. appKey:[%s]", args["appKey"]))
                return false, "appKey" 
        end
        if args["accountID"] then
                if string.len(args["accountID"]) ~= 10 then
                        only.log("E", string.format("accountID is error. accountID:[%s]", args["accountID"]))
                        return false, "accountID" 
                end
        end
        if args["imei"] then
                if (not utils.is_number(args["imei"])) or (string.len(args["imei"]) ~= 15) or (string.find(args["imei"], "0") == 1)  then
                        only.log("E", "imei is error.")
                        return false, "imei"
                end
        end
        if not args["accountID"] and not args["imei"] then
                only.log("E", "One of the imei or accountID")
                return false, "One of the imei or accountID"        
        end

        args["startTime"] = tonumber( args["startTime"] )
        if not args["startTime"] or string.len(args["startTime"]) ~= 10 then
                only.log("E", "startTime is error!")
                return false, "startTime" 
        end 

        args["endTime"] = tonumber( args["endTime"] )
        if not args["endTime"] or string.len(args["endTime"]) ~= 10 then
                only.log("E", "endTime is error.")
                return false, "endTime"
        end 

        if args["startTime"] > args["endTime"] then
                args["startTime"] ,args["endTime"] = args["endTime"], args["startTime"]
        end 

        if args["endTime"] - args["startTime"] > dk_utils.timeRangeLimit then
                only.log("E", "startTime: ".. args["startTime"])
                only.log("E", "endTime: ".. args["endTime"])
                only.log("E", "time interval out of range!")
                local errlog = string.format("time interval must less than %d hours. startTime or endTime", dk_utils.timeRangeLimit/3600)
                return false, errlog
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

        -->> get data.
        local bizid
        local hashkey
        local key
        if args["accountID"]  then
                key = string.format("releaseMessage:%s:%s:", args["accountID"], args["appKey"])
                hashkey = args["accountID"]
        else
                key = string.format("releaseMessage:%s:%s:", args["imei"], args["appKey"])
                hashkey = args["imei"]
        end

        local t1, t2, t3 = dk_utils.parse_time(args["startTime"], args["endTime"])
        local tab = {}
        if t1 then
                ok = dk_utils.get_data_from_tsdb_range(key, hashkey, t1, t2, tab)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end
        if t3 then
                ok = dk_utils.smembers_data_from_redis_range(key, hashkey, t2, t3, tab)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end

        local str
        if not tab then
                tab = {}
        end
        ok, str = utils.json_encode(tab)

        only.log("D", "range bizid count: " .. #tab)

        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], str)
end 
