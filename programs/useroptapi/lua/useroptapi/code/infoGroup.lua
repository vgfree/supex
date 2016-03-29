--版权声明：无
--文件名称：
--创建者：
--创建日期：
--文件描述：获取群组信息
--历史记录：无
local only = require('only')
local utils = require("utils")
local supex = require("supex")
local redis_api = require('redis_pool_api')
local msg = require('msg')

module('infoGroup',package.seeall)

function handle()

        local args = supex.get_our_uri_table()

        local gid = args["gid"]
        local info = args["info"]

        local ok
        local xx_redis = "xx_redis"
        local info_table = {}
        if (info == "1") then
                ok, info_table = redis_api.cmd(xx_redis, "","smembers", gid)
        else
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end

        local info_data = {}
        ok, info_data = utils.json_encode(info_table)
        if (ok == true) then
                return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], info_data)
        else
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
end
