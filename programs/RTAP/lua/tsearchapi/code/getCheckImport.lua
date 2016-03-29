local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("tsearchapi_utils")

module('getCheckImport', package.seeall)

local timeRoleranceLimit = 2 * 600

local function check_data_from_tsdb(key, time)
        local tsdb_name = dk_utils.get_tsdb_name(time, nil)
        only.log("E", "tsdb_name: %s", tsdb_name)
        local keys = nil
        local ok, exists = redis_api.only_cmd(tsdb_name, "exists", key)
        if not ok then
                only.log("E", "keys tsdb error.")
                return false
        end

        return true, exists
end

function handle()
        local import_time = os.time() - dk_utils.timeRangeLimit - timeRoleranceLimit

        -->> get data.
        local key = string.format("ACTIVEUSER:%s%d", os.date("%Y%m%d%H", import_time), os.date("%M", import_time)/10)
        only.log("D", "check active user keys:" .. key)
        
        local ok, exists = check_data_from_tsdb(key, import_time)
        if not ok then
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
                
        --> return result.
        if not exists then
                exists = 0
        end

        local keys = nil
        if exists == 0 then
                keys = ""
        else
                keys = key
        end

        only.log("D", "active user keys: " .. keys)
        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], keys)
end 
