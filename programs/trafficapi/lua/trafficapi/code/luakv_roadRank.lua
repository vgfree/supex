
local luakv		= require('luakv_pool_api')
local only		= require('only')
local gosay		= require('gosay')
local supex		= require('supex')
local scan		= require('scan')
local msg		= require('api_msg')
local func_get_traffic	= require('func_get_traffic_msg')

module("luakv_roadRank", package.seeall)

function luakv_memory(args)

	if tonumber(args['maxSpeed']) == 0 or tonumber(args['avgSpeed']) == 0 or tonumber(args['passTime']) == 0 then
		return false
	end

	if tonumber(args['passTime']) < 1 then
		args['passTime'] = 1
	end

	local key = string.format("%d%03d:trafficInfo", args['RRID'], args['SGID'])
	only.log('D', 'key = ' .. key)
	local value = string.format("%d:%d:%d:%d", args['maxSpeed'], args['avgSpeed'], args['collectTime'], args['passTime'])
	only.log('D', 'value = ' .. value)

	if key and value then
		luakv.cmd('luakv', '', 'set', key, value)
		return true
	end
--[[
	local ok, result = luakv.cmd('luakv', '', 'get', key)
	only.log('D', 'result = ' .. result)

	gosay.resp_msg(msg['MSG_SUCCESS_WITH_RESULT'], result)
--]]

end

function handle()
	
	local args = supex.get_our_body_table()
	if not args or not next(args) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],'args')
	end

	only.log('D', 'args = ' .. scan.dump(args))
	luakv_memory(args)
	return true
end

