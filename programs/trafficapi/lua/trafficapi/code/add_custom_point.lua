local only              = require("only")
local redis_api         = require("redis_pool_api")
local luakv_api		= require("luakv_pool_api")
local supex		= require('supex')
local msg		= require('msg')
local PUBLIC_FUNC_LIST 	= require('PUBLIC_FUNC_LIST')

module('add_custom_point', package.seeall)

--功	能：增加预约点
--参	数：pointList---json格式的点数据
--返 回 值：true---成功	  false---失败
function handle()
	local pointList = supex.get_our_body_table()["pointList"]
	local point_info = PUBLIC_FUNC_LIST.parse_data(pointList) 
	if not point_info then
		local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_ERROR_PARSE_DATA"])
		PUBLIC_FUNC_LIST.send_respond(str)
		return false
	end
	for k,v in ipairs(point_info) do
                local lon = v['longitude']
                local lat = v['latitude']

		local grid_lon,grid_lat = PUBLIC_FUNC_LIST.grid_five_thousandths(lon,lat)

                local grid = string.format('%d&%d',grid_lon,grid_lat)
                local ok,data = luakv_api.cmd('FiveThousandths'," ", 'hset', grid, v['id'], v['type'])
                if not ok then
                        only.log('E',string.format("failed to luakv hmset key:%s",grid))
			local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_DO_LUAKV_FAILED"])
			PUBLIC_FUNC_LIST.send_respond(str)
                        return false
                end
                local ok = redis_api.cmd('clubCustomPoint'," ", 'hset', grid, v['id'], v['type'])
                if not ok then
                        only.log('E',string.format("failed to redis hmset key:%s",grid))
			local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_DO_REDIS_FAILED"])
			PUBLIC_FUNC_LIST.send_respond(str)
                        return false
                end
        end
	
	local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_SUCCESS"])
	PUBLIC_FUNC_LIST.send_respond(str)
	return true	
end



