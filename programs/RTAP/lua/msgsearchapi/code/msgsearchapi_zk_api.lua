local redis_api = require ('redis_pool_api')
local link = require ('link')
local cutils = require("cutils")
local only = require("only")

module('msgsearchapi_zk_api', package.seeall)

local last_access_key = nil
local last_access_redname = nil

function get_read_tsdb(time, key)
        local time_field = tonumber(os.date("%Y%m%d%H%M%S", time))
        local index = cutils.custom_hash(key, 8192 ,0)
        local ok, ip, port = _G.zk_get_read_dn(index, time_field)
        if not ok then
                return "nil"
        end

        local redname = ip .. port

        redis_api.add_to_pool(redname, ip, port)

        if last_access_key ~= key then
                last_access_key = key
                last_access_redname = redname
        else
                return last_access_redname
        end
        
        return redname

end
