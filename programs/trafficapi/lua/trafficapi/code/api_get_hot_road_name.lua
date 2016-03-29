--Author	: dujun
--Date		: 2015-12-17
--Function	: get hot road name

local utils		= require('utils')
local redis		= require('redis_pool_api')
local only		= require('only')
local msg		= require('api_msg')
local gosay		= require('gosay')
local supex		= require('supex')
local safe		= require('safe')
local json		= require('cjson')
local scan		= require('scan')

local url_info = {
        type_name = 'system',
        app_key = nil,
        client_host = nil,
        client_body = nil
}

module("GetHotRoadName", package.seeall)

local function check_parameter(args)

	safe.sign_check(args, url_info)
	if not args['cityCode'] then
		gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'cityCode')
		return false
	end

	if not args['count'] then
		gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'count')
		return false
	end
	return true
end

local function get_hot_road_name(args)

	local key = string.format("%d:hotRoadName", args['cityCode'])

	local ok, result = redis.cmd('roadname_search', 'zrevrange', key, 0, args['count']-1)
	if not ok or not result then
		only.log('E', 'Failed to get hot roadName !')
		gosay.resp_msg(msg['SYSTEM_ERROR'])
		return false
	end

	return result
end

local function handle()

	local body = supex.get_our_body_data()
	only.log('I', 'body = ' .. scan.dump(body))
	if not body or type(body) ~= 'string' then
		return gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'])
	end

	local args = {}
	local ret = get_traffic_msg.str_split(body, '&')
	for k, v in ipairs(ret) do
		local k_v = get_traffic_msg.str_split(v, '=')
		args[k_v[1]] = k_v[2]
	end

	url_info['client_body'] = body
	url_info['app_key'] = args['appKey']
	
	if not check_parameter(args) then
		return false
	end

	local result = get_hot_road_name(args)
	if not result then
		return false
	end

	local ok, data = pcall(json.encode, result)
	if not ok then
		only.log('E', "Failed to json_encode !")
		gosay.resp_msg(msg['SYSTEM_ERROR'])
		return false
	end

	gosay.resp_msg(msg['MSG_SUCCESS_WITH_RESULT'], data)
end

