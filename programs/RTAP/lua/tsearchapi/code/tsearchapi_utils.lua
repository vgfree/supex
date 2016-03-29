-- 
-- file: dk_utils.lua
-- auth: chenjianfei@daoke.me
-- date: 2014-06-24
-- desc: common function.
--


local only  = require ('only')
local tsearchapi_zk = require ('tsearchapi_zk_api')

module("tsearchapi_utils", package.seeall)

redisRemainTime = 4*3600   -- 4 hours.
redisURLRemainTime = 2*3600   -- 2 hours.
timeRangeLimit = 2*3600        -- 2 hours.

imeiShardingSize = 10

-- for tsdb redis sharding
tsdbRedisNamePrefix = "tsdb_redis"

function get_imei_hash( imei )
    local tmp = tonumber(imei)
    if not tmp then
        only.log('E',string.format("get_imei_hash:imei format error: %s", imei))
        return nil 
    end 
    return imei % imeiShardingSize
end

function get_time_interval(time)
        return os.time({
                year = os.date("%Y", time),
                month = os.date("%m", time),
                day = os.date("%d", time),
                hour = os.date("%H", time),
                min = os.date("%M", time) - os.date("%M", time)%10
        })  
end

function parse_time(startTime, endTime, isurllog)

        startTime = get_time_interval(startTime)
        endTime = get_time_interval(endTime)

        local remainTime = redisRemainTime
        if isurllog then
                        remainTime = redisURLRemainTime
        end

        local midTime = get_time_interval(os.time() - remainTime)

        if startTime > midTime then
                return nil, startTime, endTime
        end

        if endTime < midTime then
                return startTime, endTime, nil
        end

        return startTime, midTime, endTime
end

-- 2015年 08月 24日 星期一 18:20:00 CST
local TIME_END = 1440411600

local url_idx_0 = 1
local url_idx_1 = 3

function get_tsdb_name_for_url(time, imei)
        local tab = {
                [1] = "tsdb_url_rw_0",
                [2] = "tsdb_url_rw_1",
                [3] = "tsdb_url_rw_2",
                [4] = "tsdb_url_rw_3"
        }

        key = tonumber(imei) % 8192

        if time < TIME_END then
                return tsearchapi_zk.get_read_tsdb(time, imei)
        else
                if key < 4096 then
                        if url_idx_0 > 1 then
                                url_idx_0 = 1
                        else
                                url_idx_0 = 2
                        end
                        return tab[url_idx_0]
                else
                        if url_idx_1 > 3 then
                                url_idx_1 = 3
                        else
                                url_idx_1 = 4
                        end
                        return tab[url_idx_1]
                end
        end
end

function get_tsdb_name(time, imei)
        return tsearchapi_zk.get_read_tsdb(time, imei)
end
