--版权声明：无
--文件名称：
--创建者：
--创建日期：
--文件描述：绑定uid 和 cid
--历史记录：无
local only = require('only')
local cjson = require('cjson')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local msg = require('msg')

module('bindUid',package.seeall)

function handle()

        local args = supex.get_our_uri_table()

        local cid = args["cid"]
        local uid = args["uid"]

        only.log("D", "uid = %s", uid)
        only.log("D", "cid = %s", cid)

        local xx_redis = "xx_redis"
        local ok1 = redis_api.cmd(xx_redis, "", "set", cid, uid)
        local ok2 = redis_api.cmd(xx_redis, "", "set", uid, cid)

        if (ok1 == true and ok2 == true) then
                return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], "ture")
        else
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
end 
