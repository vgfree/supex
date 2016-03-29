module('realtime_defs', package.seeall)

STATIC = {
        TIME_INTERVAL_MAX = 15,
        TIME_LIMIT = 4200,
        PATH_AFFIRM_MAX_COUNT = 5,
        FINISH_MILE_ZSET = "finishMileZSet",
        LASTED_MILE_ZSET = "lastedMileZSet",
}

--名称：setTTL
--功能： 给key设置expire时间
--参数：redisName --> redis名称
--参数：setKey    --> 设置的key名
--参数：secs      --> TTL时间
--返回：
--修改：2015-06-27 重构实时里程
function setTTL(redisName, hash_key, setKey, secs)
        local ok, res = redis_pool_api.cmd(redisName, hash_key or '', "expire", setKey, secs) -- ttl 72 hours
        if not ok or not res then
                only.log('W', string.format("reset TTL wanging, key>>%s", setKey))
        end
end

--名称：delKey
--功能： 给key设置expire时间
--参数：redisName --> redis名称
--参数：setKey    --> 设置的key名
--参数：secs      --> TTL时间
--返回：
--修改：2015-06-27 重构实时里程
function delKey(redisName, hash_key, setKey)
        local ok, res = redis_pool_api.cmd(redisName, hash_key or '', "del", setKey) -- ttl 72 hours
        if not ok or not res then
                only.log('W', string.format("delKey wanging, key>>%s", setKey))
        end
end

