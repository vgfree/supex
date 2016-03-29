local utils = require("utils")
local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("tsearchapi_utils")
local supex = require("supex")

module('geturllog', package.seeall)

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
        if args["endTime"] - args["startTime"] > dk_utils.timeRangeLimit then
                only.log("E", "startTime: ".. args["startTime"])
                only.log("E", "endTime: ".. args["endTime"])
                only.log("E", "time interval out or range!")
                local errlog = string.format("time interval must less than %d hours. startTime or endTime", dk_utils.timeRangeLimit/3600)
                return false, errlog
        end
        return true
end

local function get_column_count(row)
        -->> check input.
        if not row or string.len(row) == 0 then
                return 0
        end

        -->> get count.
        local index = 0 
        local count = 1
        while true do
                index = string.find(row, "|", index + 1)
                if not index then
                        break
                end
                count = count + 1
        end
        return count 
end

local function get_data_from_redis(imei, startTime, endTime, gps_tab)
        only.log("D", "get data from redis.")
        local ok
        local key, value
        local tab = {}
        local colCnt
        local tsdb_redis_name = string.format("%s%s", dk_utils.tsdbRedisNamePrefix, dk_utils.get_imei_hash(imei))
        only.log("D", "redis_name: %s", tsdb_redis_name)
        for t=startTime, endTime, 600 do
                -->> key format: URL:${imei}:${time_interval}
                key = string.format("URL:%s:%s%d", imei, os.date("%Y%m%d%H", t), os.date("%M", t)/10)
                ok, tab = redis_api.only_cmd(tsdb_redis_name, "SMEMBERS", key)
                if not ok then
                        only.log("I", string.format("tsdb_redis: SMEMBERS %s error.", key))
                        return false
                end

                -->> encode the urllog data.
                if tab and #tab > 0 then
                        table.sort(tab)
                        colCnt = get_column_count(tab[1])
                        value = string.format("%d*%d@%s|", #tab, colCnt, table.concat(tab, "|"))

                        table.insert(gps_tab, key)
                        table.insert(gps_tab, value)
                end
        end

        -->> return
        return true, result
end

local function get_data_from_tsdb(imei, startTime, endTime, gps_tab)
        --> get urlinfo from tsdb.
        only.log("D", "get data from tsdb.")
        only.log("D", "startTime: %s; endTime: %s", startTime, endTime)

        local prekey = "URL:" .. imei .. ":"
        local skey = string.format("%s%d", os.date("%Y%m%d%H", startTime), os.date("%M", startTime)/10)
        local ekey = string.format("%s%d", os.date("%Y%m%d%H", endTime), os.date("%M", endTime)/10)

        only.log("D", "prefix key: %s; startKey: %s; endKey: %s", prekey, skey, ekey)

        local t1 = dk_utils.get_tsdb_name_for_url(startTime, imei)
        local t2 = dk_utils.get_tsdb_name_for_url(endTime, imei)

        only.log("D", "tsdb_name:%s", t1)
        local ok, tab = redis_api.only_cmd(t1, "lrange", prekey, skey, ekey)
        if not ok then
                only.log("E", "lrange tsdb[%s] error. prekey:%s skey:%s ekey: %s", t1, prekey, skey, ekey)
                return false
        end

        if tab then
                table.foreach(tab, function(i, v) table.insert(gps_tab, v) end)
        end

        if t1 == t2 then
                return true
        end

        only.log("D", "tsdb_name:%s", t2)
        ok, tab = redis_api.only_cmd(t2, "lrange", prekey, skey, ekey)
        if not ok then
                only.log("E", "lrange tsdb[%s] error. prekey:%s skey:%s ekey: %s", t2, prekey, skey, ekey)
                return false
        end

        if tab then
                table.foreach(tab, function(i, v) table.insert(gps_tab, v) end)
        end

        only.log("I", "tab size: %d", #tab)

        return true
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
        local t1, t2, t3 = dk_utils.parse_time(args["startTime"], args["endTime"], true)

        -->> get data from tsdb if necessary.
        local tab = {}
        if t1 then
                ok = get_data_from_tsdb(args["imei"], t1, t2, tab)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end

        -->> get data from redis if nesessary.
        if t3 then
                ok = get_data_from_redis(args["imei"], t2, t3, tab)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end

        -->> decode. 
        if not tab then
                tab = {}
        end
        local str 
        ok, str = utils.json_encode(tab) 

        --> return result.
        only.log("D", "urllog count: ".. #tab)
        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], str)
end 
