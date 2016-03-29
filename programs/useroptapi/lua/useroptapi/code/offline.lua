--版权声明：无
--文件名称：
--创建者：
--创建日期：
--文件描述：下线
--历史记录：无
local only = require('only')
local cjson = require('cjson')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local msg = require('msg')

module('offline',package.seeall)


function handle()

        local args = supex.get_our_uri_table()

        local cid = args["cid"]

        local xx_redis = "xx_redis"
        local ok, uid = redis_api.cmd(xx_redis, "","get", cid)
        
        local ok1 = redis_api.cmd(xx_redis, "","srem", "online_cid", cid)
        local ok2 = redis_api.cmd(xx_redis, "","del", uid)
        local ok3 = redis_api.cmd(xx_redis, "","del", cid)
        
        if (ok1 == true and ok2 == true and ok3 == true) then
                return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], "true")
        else
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
end
