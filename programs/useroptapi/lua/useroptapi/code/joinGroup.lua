--版权声明：无
--文件名称：
--创建者：
--创建日期：
--文件描述：加入群组
--历史记录：无
local only = require('only')
local cjson = require('cjson')
local supex = require('supex')
local cjson = require('cjson')
local redis_api = require('redis_pool_api')
local msg = require('msg')

module('joinGroup',package.seeall)


function handle()
        local body = supex.get_our_body_table()
        only.log("D","test")

        -- 接收http请求，将结果存入redis中
        local args = supex.get_our_uri_table()

        local uid = args["uid"]
        local gid = args["gid"]

        local xx_redis = "xx_redis"
        local ok = redis_api.cmd(xx_redis, "","sadd", gid, uid)

        if (ok == true) then
                return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], "true")
        else
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
end

