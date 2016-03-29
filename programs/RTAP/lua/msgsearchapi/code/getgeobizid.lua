local utils = require("utils")
local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("msgsearchapi_utils")
local supex = require("supex")

module('getgeobizid', package.seeall)

local function check_parameter(args)
        local lon = args["senderLongitude"]
        local lat = args["senderLatitude"]
        if lon then
                lon = tonumber(lon)
                if not lon or lon<-180 or lon>180 then
                        return false, "Longitude"
                end
                if not lat then
                        return false, "Latitude"
                end
        end

        if lat then
                lat = tonumber(lat)
                if not lat or lat<-90 or lat>90 then
                        return false, "Latitude"
                end
                if not lon then
                        return false, "Longitude"
                end
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
        local senderLongitude 
        local senderLatitude
        senderLongitude = string.format("%6s", args['senderLongitude'])
        senderLatitude = string.format("%6s", args['senderLatitude'])

        --local grid_no = string.format('%d&%d', math.floor(args['senderLongitude']*100), math.floor(args['senderLatitude'] * 100))
        local grid_no = string.format('%s&%s', (senderLongitude * 100), (senderLatitude * 100))
        only.log('D', grid_no)
        local key = string.format("GEOtoMessage:%s:",grid_no)
        local hashkey = (args['senderLongitude']+ args['senderLatitude'])*100
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

        only.log("D", "range geobizid count: " .. #tab)

        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], str)

end 
