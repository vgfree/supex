local redis_api = require("redis_pool_api")
local only  = require ('only')
local msgsearchapi_zk = require ('msgsearchapi_zk_api')
local msg = require("msg")

module("msgsearchapi_utils", package.seeall)

redisRemainTime = (1*3600 + 1800)   -- 1 hours 30 miniters.
timeRangeLimit = 2*3600
HASH_NUMBER = 4;
maxTime = 48*60*60

-- for tsdb redis sharding
tsdbRedisNamePrefix = "tsdb_redis"

function get_time_interval(time)
        return os.time({
                year = os.date("%Y", time),
                month = os.date("%m", time),
                day = os.date("%d", time),
                hour = os.date("%H", time),
                min = os.date("%M", time) - os.date("%M", time)%10
        })  
end

function parse_time(startTime, endTime)
        startTime = get_time_interval(startTime)
        endTime = get_time_interval(endTime)

        local remainTime = redisRemainTime

        local midTime = get_time_interval(os.time() - remainTime)
        if startTime > midTime then
                return nil, startTime, endTime
        end
        if endTime < midTime then
                return startTime, endTime, nil
        end

        return startTime, midTime, endTime
end

function get_tsdb_name(time, hashkey)
        return msgsearchapi_zk.get_read_tsdb(time, hashkey)
end


function hash_function(key)
        local index = cutils.custom_hash(key, HASH_NUMBER,0)
        local message_redis_name = string.format("message_redis%s",index)
        return message_redis_name
end


function get_column_count(row)
        if not row or string.len(row) == 0 then
                return 0
        end
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


function smembers_data_from_redis(key, hashkey)
        local data
        local data_count = 0
        local message_redis_name
        local colCnt = 0
        message_redis_name = hash_function(hashkey)
        local ok, tab = redis_api.only_cmd(message_redis_name, "smembers", key)
        if not ok then
                only.log("E", "smembers tsdb_redis error.")
                return false
        end
        if tab and #tab > 0 then
                data_count = data_count + #tab
                colCnt = get_column_count(tab[1])
                if data then
                        data = string.format("%s|%s", data, table.concat(tab, "|"))
                else
                        data = string.format("%s", table.concat(tab, "|"))
                end
        end
        if data then
                data = string.format("%d*%d@%s|", data_count, colCnt,data)
        end
        return true, data
end


function get_data_from_redis(key, hashkey)
        local data
        local data_count = 0
        local message_redis_name
        local colCnt = 0
        message_redis_name = hash_function(hashkey)
        local ok, data = redis_api.only_cmd(message_redis_name, "get", key)
        if not ok then
                only.log("E", "smembers tsdb_redis error.")
                return false
        end
        return true, data
end


function get_data_from_tsdb(key, time, hashkey)
        local tsdb_name = get_tsdb_name(time, hashkey)
        only.log("E", "tsdb_name: %s", tsdb_name)
        local data = ""
        local ok, data = redis_pool_api.only_cmd(tsdb_name, "get", key)
        if not ok then
                only.log("E", "smembers tsdb error.")
                return false
        end
        return true, data
end

function smembers_data_from_redis_range(keyvalue, hashkey, startTime, endTime, data_tab)
        local ok, key, value
        local tab = {}
        local colCnt
        local t
        local message_redis_name = hash_function(hashkey)
        only.log("D", message_redis_name)
        for t=startTime, endTime, 600 do
                key = string.format("%s%s%d", keyvalue, os.date("%Y%m%d%H", t), os.date("%M", t)/10)
                ok, tab = redis_api.only_cmd(message_redis_name, "SMEMBERS", key)
                if not ok then
                        only.log("I", string.format("message_redis: SMEMBERS %s error.", key))
                        return false
                end 
        
                if tab and #tab > 0 then
                        table.sort(tab)
                        colCnt = get_column_count(tab[1])
                        value = string.format("%d*%d@%s|", #tab, colCnt, table.concat(tab, "|"))
                        table.insert(data_tab, key)
                        table.insert(data_tab, value)
                end 
        end
        return true
end

function get_data_from_tsdb_range(keyvalue, hashkey,  startTime, endTime, data_tab)
        only.log("D", "startTime: ".. startTime)
        only.log("D", "endTime: ".. endTime)
        local skey = string.format("%s%d", os.date("%Y%m%d%H", startTime), os.date("%M", startTime)/10)
        local ekey = string.format("%s%d", os.date("%Y%m%d%H", endTime), os.date("%M", endTime)/10)
        local t1 = get_tsdb_name(startTime, hashkey)
        local t2 = get_tsdb_name(endTime, hashkey)
        only.log("D", "tsdb_name: %s", t1) 
        local ok, tab = redis_api.only_cmd(t1, "lrange", keyvalue, skey, ekey)
        if not ok then
                only.log("E", "lrange tsdb[%s] error. key:%s skey:%s ekey: %s", t1, keyvalue, skey, ekey)
                return false
        end 

        if tab then
                table.foreach(tab, function(i, v) table.insert(data_tab, v) end)
        end 

        if t1 == t2 then
                return true
        end 
        
        only.log("D", "tsdb_name: %s", t2) 
        ok, tab = redis_api.only_cmd(t2, "lrange", keyvalue, skey, ekey)
        if not ok then
                only.log("E", "lrange tsdb[%s] error. key:%s skey:%s ekey: %s", t2, keyvalue, skey, ekey)
                return false
        end 

        if tab then
                table.foreach(tab, function(i, v) table.insert(data_tab, v) end)
        end
        return true
end

function get_datasize_from_redis_range(keyvalue, hashkey, startTime, endTime, data_tab)
        local ok, key, value
        local totalCnt,cnt = 0,0
        local t
        local message_redis_name = hash_function(hashkey)
        only.log("D", message_redis_name)
        for t=startTime, endTime, 600 do
                key = string.format("%s%s%d", keyvalue, os.date("%Y%m%d%H", t), os.date("%M", t)/10)
                ok, cnt = redis_api.only_cmd(message_redis_name, "SCARD", key)
                if not ok then
                        only.log("I", string.format("message_redis: SMEMBERS %s error.", key))
                        return false
                end 
                totalCnt = totalCnt + cnt
        end
        only.log("E", "total count " .. totalCnt)
        return true, totalCnt
end

function get_datasize_from_tsdb_range(keyvalue, hashkey,  startTime, endTime, data_tab)
        local totalCnt = 0
        local skey,ekey,tsdb_name
        for i=startTime,endTime,5*60*60  do
                if (i+5*60*60 < endTime) then
                        skey = string.format("%s%d", os.date("%Y%m%d%H", i), os.date("%M", startTime)/10)
                        ekey = string.format("%s%d", os.date("%Y%m%d%H", i+5*60*60), os.date("%M", endTime)/10)
                else
                        skey = string.format("%s%d", os.date("%Y%m%d%H", i), os.date("%M", startTime)/10)
                        ekey = string.format("%s%d", os.date("%Y%m%d%H", endTime), os.date("%M", endTime)/10)
                end
                tsdb_name = get_tsdb_name(i, hashkey)
                only.log("D", "tsdb_name: %s", tsdb_name) 
                local ok, tab = redis_api.only_cmd(tsdb_name, "lrange", keyvalue, skey, ekey)
                if not ok then
                        only.log("E", "lrange tsdb[%s] error. key:%s skey:%s ekey: %s", t1, keyvalue, skey, ekey)
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
