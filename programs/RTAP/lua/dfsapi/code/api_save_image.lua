local utils     = require('utils')
local cutils     = require('cutils')
local only      = require('only')
local msg       = require('msg')
local gosay     = require('gosay')
local spx_utils	= require('spx_utils')
local scan	= require('scan')
local safe	= require('safe')
local dfs_cfg 	= require('dfs_cfg')
local cjson 	= require('cjson')
local supex 	= require('supex')
local redis_api = require("redis_short_api")

module('api_save_image', package.seeall)



local function save_file_dfs(binary, cacheTime, isScaling, args)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainImage", dfs_cfg["image_domain"])		
	local url_args = 'isStorage=true&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local jpgkey = basekey .. ".jpg" 
	
	local info_tab = {
		url		= '',
		fileID		= uuid,
		fileSize	= #binary,
	}
	-->jpg
	local ok, result, memb = spx_utils.set_to_dfsdb(jpgkey, binary, cacheTime)
	only.log('D',string.format("memb is %s",memb['mark']))
	if not ok then 
		ok, result, memb = spx_utils.set_to_dfsdb_spare(jpgkey, binary, cacheTime)
		if not ok then return false, "save to tsdb failed" end
	end

	local group = memb['mark']
	local uri = string.format(url_args, group, jpgkey)
	info_tab['url'] = url_path .. "?" .. cutils.url_encode(uri)
	return true, info_tab
end

local function save_file_redis(binary, cacheTime, isScaling, args)
	local uuid = utils.create_uuid()
	local url_path = string.format("https://%s/dfsapi/v2/gainImage", dfs_cfg["group_domain"])
	local url_args = 'isStorage=false&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local jpgkey = basekey .. ".jpg" 
	local info_tab = {
		url		= '',
		fileID		= uuid,
		fileSize	= #binary,
	}
	-->jpg
	local ok, result, memb = spx_utils.set_to_redis(jpgkey, binary, cacheTime)
	if ok then
		only.log('D',string.format("jpgkey %s save %s sucessed",jpgkey,memb))
	else
		ok, result, memb = spx_utils.set_to_redis_spare(jpgkey, binary, cacheTime)
		if ok then 
			only.log('D',string.format("jpgkey %s save %s sucessed",jpgkey,memb))
		else
			return false, "save to tsdb failed" 
		end
	end

	local group = memb['mark']
	local uri = string.format(url_args, group, jpgkey)
	info_tab['url'] = url_path .. "?" .. cutils.url_encode(uri)
	return true, info_tab
end

function string:split(sep)
	local sep, fields = sep or "\t", {}
	local pattern = string.format("([^%s]+)", sep)
	self:gsub(pattern, function(c) fields[#fields+1] = c end)
	return fields
end

function check_parameter(head,body)
	if not head['appKey'] or #head['appKey'] > 10 then
		gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], 'appKey')
		return false
	end
	local sign_tab = {}
	sign_tab['appKey'] = head['appKey']
	sign_tab['sign'] = head['sign']
	sign_tab['length'] = body['length']
	if body['isStorage'] then
		sign_tab['isStorage'] = body['isStorage']
	end
	if body['cacheTime'] then
		sign_tab['cacheTime'] = body['cacheTime']
	end
	local ok = safe.sign_check(sign_tab, url_info)
	return ok
	
end
function handle()
	local file_binary, file_name = nil, nil
	local args = supex.get_our_body_table()
	if not args then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "no body")
	end
	-------------------------------------------------------------------------------------------
	local header = supex.get_our_head()
	only.log('D', 'header = %s', header)

	ret = {}
        result = string.split(header, '\r\n')
        for k, v in ipairs(result) do
                if string.match(v, "appKey") then
                        _, _, _, ret['appKey'] = string.find(v, "(%a+):%s*(.+)")
                elseif string.match(v, "sign") then
                        _, _, _, ret['sign'] = string.find(v, "(%a+):%s*(.+)")
                end
        end     

        only.log('D', 'appKey = %s', ret['appKey'])
        only.log('D', 'sign = %s', ret['sign'])
	-------------------------------------------------------------------------------------------
	for _,v in pairs(args or {}) do
		if type(v) == "table" then	
			file_binary = v['data']
			file_name = v['file_name']
			break
		end
	end
	if not file_binary then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "arg is nil")
	end
        only.log('D', "image file_name = %s, file_length = %d, file_binary = %s", file_name, tonumber(#file_binary), file_binary)  
	local st, ed, image_type = string.find(file_name, "%.(%a+.)$")
	--图片支持格式有bmp,jpg,jpeg,gif和png
	if image_type ~= 'bmp' and image_type ~= 'jpg' and image_type ~= 'jpeg' and image_type ~= 'gif' and image_type ~= 'png' then 
		only.log('E', "image format error")
		return gosay.resp_msg(msg["MSG_ERROR_IMAGE_TYPE"], "image type error")
	end	
	--check
	local url_tab = {
		type_name = 'system',
		app_key  = '',
		client_host = '',
		client_body = '',
	}
	url_tab['app_key'] = ret['appKey']

	
	local ok = check_parameter(ret,args)	
	if not ok then
		return false
	end
	if tonumber(args['length'] or 0) ~= tonumber(#file_binary) then
		only.log('E', "image length error, args_length = %s", args['length'])
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "file length")
	end

	local isStorage = args['isStorage'] 
	if not isStorage or isStorage == '' then
		isStorage = 'false'
	end
	-- 传参时是否缩放为缺省,即默认为不缩放
	local isScaling = args['isScaling']  
	if not isScaling or isScaling == '' then
		isScaling = 'false'
	end
	local cacheTime = tonumber(args['cacheTime']) 
	local ok, ret_val = nil, nil
	if isStorage == 'true' then
		ok, ret_val = save_file_dfs(file_binary, cacheTime, isScaling, args)	
		if not ok or not ret_val then
			only.log('W','file save redis failed!')
			return gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed")
		end
	elseif isStorage == 'false' then
		ok, ret_val  = save_file_redis(file_binary, cacheTime, isScaling, args)
		if not ok or not ret_val then
			only.log('W','file save dfs failed!')
			return gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed")
		end	
	else
		only.log('E', 'isStorage is error')
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "isStorage")
	end


	local ok, dfs_url_info = pcall(cjson.encode, ret_val)
	dfs_url_info = string.gsub(dfs_url_info, "\\", "")
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], dfs_url_info)
end
