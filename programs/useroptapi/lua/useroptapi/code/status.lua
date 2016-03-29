--版权声明：无
--文件名称：
--创建者：
--创建日期：
--文件描述：查看uid状态(是否在线)
--历史记录：无
local only = require('only')
local cjson = require('cjson')
local supex = require('supex')
local redis_api = require('redis_pool_api')
local msg = require('msg')

module('status',package.seeall)


function handle()
        local args = supex.get_our_uri_table()

        local uid = args["uid"]
        local cid = redis_api.cmd(xx_redis, "", "get", uid)
        local xx_redis = "xx_redis"
        local ok, status = redis_api.cmd(xx_redis, "","sismember", "online_cid", cid)
        if ok then
                if status then
                        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], "true")
                else
                        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], "false")
                end
        else
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
end
