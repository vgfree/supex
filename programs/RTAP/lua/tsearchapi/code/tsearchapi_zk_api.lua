local redis_api = require ('redis_pool_api')
local link = require ('link')
local only = require ('only')

module('tsearchapi_zk_api', package.seeall)

local last_access_imei = "000000000000000"
local last_access_redname = "nil"

local second_expansion_time_start = 1422962400
local second_expansion_time_end   = 1433938800

function get_read_tsdb(time, imei)
        local time_field = tonumber(os.date("%Y%m%d%H%M%S", time))
        local key = 0

        if not imei then
                key = math.random(8192) - 1
        elseif time >= second_expansion_time_start and time < second_expansion_time_end then
                if (tonumber(imei) % 2) == 0 then
                        key = 2048
                else
                        key = 6144
                end
        else
                key = tonumber(imei) % 8192
        end
        
        local ok, ip, port = _G.zk_get_read_dn(key, time_field)
        if not ok then
                return "nil"
        end

        local redname = ip .. port

        redis_api.add_to_pool(redname, ip, port)

        if last_access_imei ~= imei then
                last_access_imei = imei
                last_access_redname = redname
        else
                return redname
        end

        return redname
end
