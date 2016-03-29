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

module('api_save_video', package.seeall)

function save_image_to_dfs(jpgkey, jpg_binary, cacheTime)
	local url_path = string.format("http://%s/dfsapi/v2/gainImage", dfs_cfg["image_domain"])		
	local url_args = 'isStorage=true&group=%s&file=%s'
	local ok, result, memb = spx_utils.set_to_dfsdb(jpgkey, jpg_binary, cacheTime)
	if not ok then 
		ok, result, memb = spx_utils.set_to_dfsdb_spare(jpgkey, jpg_binary, cacheTime)
	end
	local group = memb['mark']
	local uri = string.format(url_args, group, jpgkey)
	local url = url_path .. "?" .. cutils.url_encode(uri)
	return true, url
end

function save_image_to_redis(jpgkey, jpg_binary, cacheTime)
	local url_path = string.format("http://%s/dfsapi/v2/gainImage", dfs_cfg["image_domain"])		
	local url_args = 'isStorage=false&group=%s&file=%s'

	local ok, result, memb = spx_utils.set_to_redis(jpgkey, jpg_binary, cacheTime)
	if not ok then 
		ok, result, memb = spx_utils.set_to_redis_spare(jpgkey, jpg_binary, cacheTime)
	end
	local group = memb['mark']
	local uri = string.format(url_args, group, jpgkey)
	local url = url_path .. "?" .. cutils.url_encode(uri)
	return true, url
end

local function save_file_dfs(binary, cacheTime)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainVideo", dfs_cfg["video_domain"])
	local url_args = 'isStorage=true&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local mp4key = basekey .. ".mp4" 
	local jpgkey = basekey .. ".jpg"

	----------------------------------------------------------------------------------------
	-->以basekey.mp4存放在本地
	local tmp_mp4key = uuid .. ".mp4"
	local tmp_jpgkey = uuid .. ".jpg"
	local fd = io.open(tmp_mp4key, "w+")
	fd:write(binary)
	fd:close()

	-->截取图片并存放本地
	os.execute(string.format("ffmpeg -i %s -an -y -f image2 -ss 0 -vframes 1 %s", tmp_mp4key, tmp_jpgkey))
	-->图片缩放
--	os.execute(string.format("convert -resize '100x100^' %s %s", tmp_jpgkey, tmp_jpgkey))

	local fd = io.open(tmp_jpgkey, "r")
	if not fd then
		return false, "must be mp4 file"
	end
	local jpg_binary = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -f %s %s", tmp_mp4key, tmp_jpgkey))
	----------------------------------------------------------------------------------------

	local info_tab = {
		mp4_url		= '',
		jpg_url		= '',
		fileID		= uuid,
		mp4Size		= #binary,
		jpgSize		= #jpg_binary,
	}
	
	-->mp4
	local ok, result, memb = spx_utils.set_to_dfsdb(mp4key, binary, cacheTime)
	if not ok then 
		ok, result, memb = spx_utils.set_to_dfsdb_spare(mp4key, binary, cacheTime)
		if not ok then return false, "save to tsdb failed" end
	end
	local group = memb['mark']
	local uri = string.format(url_args, group, mp4key)
	info_tab['mp4_url'] = url_path .. "?" .. cutils.url_encode(uri)

	-->jpg
	local ok, jpg_url = save_image_to_dfs(jpgkey, jpg_binary, cacheTime)
	if not ok or not jpg_url then
		only.log('E','jpg save dfs failed')
		return false, ''
	end
	info_tab['jpg_url'] = jpg_url	

	return true, info_tab
end	

local function save_file_redis(binary, cacheTime)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainVideo", dfs_cfg["group_domain"])
	local url_args = 'isStorage=false&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local mp4key = basekey .. ".mp4" 
	local jpgkey = basekey .. ".jpg"
	----------------------------------------------------------------------------------------
	-->以basekey.mp4存放在本地
	local tmp_mp4key = uuid .. ".mp4"
	local tmp_jpgkey = uuid .. ".jpg"
	local fd = io.open(tmp_mp4key, "w+")
	fd:write(binary)
	fd:close()

	-->截取图片并存放本地
	os.execute(string.format("ffmpeg -i %s -an -y -f image2 -ss 0 -vframes 1 %s", tmp_mp4key, tmp_jpgkey))
	-->图片缩放
--	os.execute(string.format("convert -resize '100x100^' %s %s", tmp_jpgkey, tmp_jpgkey))

	local fd = io.open(tmp_jpgkey, "r")
	if not fd then
		return false, "must be mp4 file"
	end
	local jpg_binary = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -f %s %s", tmp_mp4key, tmp_jpgkey))
	----------------------------------------------------------------------------------------
	local info_tab = {
		mp4_url		= '',
		jpg_url		= '',
		fileID		= uuid,
		mp4Size		= #binary,
		jpgSize		= #jpg_binary,
	}
	-->mp4
	local ok, result, memb = spx_utils.set_to_redis(mp4key, binary, cacheTime)
	if not ok then 
		ok, result, memb = spx_utils.set_to_redis_spare(mp4key, binary, cacheTime)
		if not ok then return false, "save to tsdb failed" end
	end
	local group = memb['mark']
	local uri = string.format(url_args, group, mp4key)
	info_tab['mp4_url'] = url_path .. "?" .. cutils.url_encode(uri)

	-->jpg
	local ok, jpg_url = save_image_to_redis(jpgkey, jpg_binary, cacheTime)
	if not ok or not jpg_url then
		only.log('E','jpg save dfs failed')
		return false, ''
	end
	info_tab['jpg_url'] = jpg_url	
	return true, info_tab

end

function handle()
	local file_binary = nil
	local args = supex.get_our_body_table()
	for _,v in pairs(args or {}) do
		if type(v) == "table" then
			file_binary = v['data']
			break
		end
	end
	if not args or not file_binary then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "arg is nil")
	end
	
	--check
	local url_tab = {
		type_name = 'system',
		app_key  = '',
		client_host = '',
		client_body = '',
	}
	url_tab['app_key'] = args['appKey']	
	--	safe.sign_check(args,url_tab)
	if tonumber(args['length'] or 0) ~= tonumber(#file_binary) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "file length")
	end
	
	local isStorage = args['isStorage'] 
	if not isStorage or isStorage == '' then
		isStorage = 'false'
	end
	local cacheTime = tonumber(args['cacheTime']) 	
	local ok, ret_val = nil, nil
	if isStorage == 'true' then
		ok, ret_val = save_file_dfs(file_binary, cacheTime)	
		if not ok or not ret_val then
			only.log('W','file save redis failed!')
			return gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed")
		end
	elseif isStorage == 'false' then
		ok, ret_val  = save_file_redis(file_binary, cacheTime)
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
