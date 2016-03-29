local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("tsearchapi_utils")
local utils = require("utils")
local supex = require("supex")

module('getgpssize', package.seeall)

local maxTime = 48*60*60

local function check_parameter(args)
        --> check imei
        if (not utils.is_number(args["imei"])) or (string.len(args["imei"]) ~= 15) 
                or (string.find(args["imei"], "0") == 1) then
                only.log("E", "imei is error!")
                return false, "imei"
        end

        --> check startTime.
        args["startTime"] = tonumber( args["startTime"] )
        if not args["startTime"] or string.len(args["startTime"]) ~= 10 then
                only.log("E", "startTime is error!")
                return false, "startTime"
        end

        --> check endTime.
        args["endTime"] = tonumber( args["endTime"] )
        if not args["endTime"] or string.len(args["endTime"]) ~= 10 then
                only.log("E", "endTime is error!")
                return false, "endTime"
        end

        --> FIXME: if startTime > endTime, then swap them.
        if args["startTime"] > args["endTime"] then
                args["startTime"] ,args["endTime"] = args["endTime"], args["startTime"]
        end

        --> FIXME: if the timeRange more than one hour, then return fasle. 
        if args["endTime"] - args["startTime"] > maxTime then
                only.log("E", "startTime: ".. args["startTime"])
                only.log("E", "endTime: ".. args["endTime"])
                only.log("E", "time interval out of range!")
                local errlog = string.format("time interval must less than %d hours. startTime or endTime", maxTime/3600)
                return false, errlog
        end
        return true
end

local function get_data_from_redis(imei, startTime, endTime)
        local ok, key
        local cnt, totalCnt = 0, 0
        local tsdb_redis_name = string.format("%s%s", dk_utils.tsdbRedisNamePrefix, dk_utils.get_imei_hash(imei))
        for t=startTime,endTime,600 do
                -->> key format: GPS:${imei}:${time_interval}
                key = string.format("GPS:%s:%s%d", imei, os.date("%Y%m%d%H", t), os.date("%M", t)/10)
                -- ok, cnt = redis_api.only_cmd("tsdb_redis", "SCARD", key)
                ok, cnt = redis_api.only_cmd(tsdb_redis_name, "SCARD", key)
                if not ok then
                        only.log("I", string.format("tsdb_redis: SMEMBERS %s error.", key))
                        return false
                end

                totalCnt = totalCnt + cnt
        end

        only.log("E", "total count " .. totalCnt)
        return true, totalCnt
end

local function get_data_from_tsdb(imei, startTime, endTime)
        --> get gspinfo from tsdb.
        local totalCnt = 0
        local totalTime = endTime-startTime

        --        if (totalTime > 5*60*60) then
        for i=startTime,endTime,5*60*60  do
                local prekey, skey, ekey        
                if (i+5*60*60 < endTime) then

                        prekey = "GPS:" .. imei .. ":"
                        skey = string.format("%s%d", os.date("%Y%m%d%H", i), os.date("%M", i)/10)
                        ekey = string.format("%s%d", os.date("%Y%m%d%H", i+5*60*60), os.date("%M", i+5*60*60)/10)
                else

                        prekey = "GPS:" .. imei .. ":"
                        skey = string.format("%s%d", os.date("%Y%m%d%H", i), os.date("%M", i)/10)
                        ekey = string.format("%s%d", os.date("%Y%m%d%H", endTime), os.date("%M", endTime)/10)

                end

                only.log("D", "tsdb-cmd: lrange ".. prekey .. " " .. skey .. " " .. ekey)
                local tsdb_name = dk_utils.get_tsdb_name(i, imei)
                only.log("D", "tsdb_name: %s", tsdb_name)
                local ok, tab = redis_api.only_cmd(tsdb_name, "lrange", prekey, skey, ekey)
                only.log("D", "tsdb-cmd: lrange end.")
                if not ok then
                        only.log("E", "get tsdb error. key: ".. key)
                        return false
                end

                if tab then
                        for k,v in ipairs(tab) do
                                if k % 2 == 0 then
                                        totalCnt = totalCnt + (tonumber(string.match(v,"(%d+)%*"))or 0 )
                                end
                        end
                end
        end

        return true, totalCnt
end

function handle()
        local args = nil 

        if supex.get_our_body_data() then
                args = supex.get_our_body_table()
        else
                args = supex.get_our_uri_table()
        end

        --> check parameters.
        local ok, errlog = check_parameter(args)
        if not ok then
                return gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], errlog)
        end

        -->> parse time.
        local t1, t2, t3 = dk_utils.parse_time(args["startTime"], args["endTime"])

        -->> get data from tsdb if necessary.
        local gps_cnt = 0
        local total_cnt = 0
        if t1 then
                ok, total_cnt = get_data_from_tsdb(args["imei"], t1, t2)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
                gps_cnt = gps_cnt + total_cnt
        end

        -->> get data from redis if nesessary.
        if t3 then
                ok, total_cnt = get_data_from_redis(args["imei"], t2, t3)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
                gps_cnt = gps_cnt + total_cnt
        end

        --> return result.
        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], gps_cnt)
end 
