--版权声明：无
--文件名称：
--创建者：
--创建日期：
--文件描述：解除绑定
--历史记录：无
local only = require('only')
local cjson = require('cjson')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local msg = require('msg')

module('unbindUid',package.seeall)


function handle()
        local body = supex.get_our_body_table()
        only.log("D","test")

        local args = supex.get_our_uri_table()

        local cid = args["cid"]
        local uid = args["uid"]

        local xx_redis = "xx_redis"
        local ok1 = redis_api.cmd(xx_redis, "","del", cid)
        local ok2 = redis_api.cmd(xx_redis, "","del", uid)

        if (ok1 == true and ok2 == true) then
                return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], "true")
        else
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end

end
