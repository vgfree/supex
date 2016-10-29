-- voice file from fastdfs to tsdb 

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

module('api_save_sound', package.seeall)

local function save_file_dfs(binary, cacheTime, type_val)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainSound", dfs_cfg["group_domain"])
	local url_args = 'isStorage=true&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local amrkey = basekey .. ".amr" 
	----------------------------------------------------------------------------------------
	-->以uuid.key存放在本地
	local tmp_voicekey = uuid .. "." .. type_val
	local tmp_amrkey = uuid .. ".amr"
	--convert format
	local amr_binary = binary
	if tostring(type_val) == "mp3" or tostring(type_val) == "wav" then
		local fd = io.open(tmp_voicekey, "w+")
		fd:write(binary)
		fd:close()
		os.execute(string.format("ffmpeg -i %s -y -ab 5.15k -ar 8000 -ac 1 %s", tmp_voicekey, tmp_amrkey))
		local fd = io.open(tmp_amrkey, "r")
		amr_binary = fd:read("*a")
		fd:close()
	
		os.execute(string.format("rm -f %s %s", tmp_voicekey, tmp_amrkey))
	else
		tmp_amrkey = tmp_voicekey
	end
	----------------------------------------------------------------------------------------
	--return infomation
	local info_tab = {
		url		= '',
		fileID		= uuid,
		fileSize	= #amr_binary,
	}
	-->amr
	local ok, result, memb = spx_utils.set_to_dfsdb(amrkey, amr_binary, cacheTime)	
	if not ok then 
		only.log('E', string.format("file is %s",amrkey))
		ok, result, memb = spx_utils.set_to_dfsdb_spare(amrkey, amr_binary, cacheTime)
		if not ok then return false, "save to tsdb failed" end
	end

	local group = memb['mark']
	local uri = string.format(url_args, group, amrkey)
	info_tab['url'] = url_path .. "?" .. cutils.url_encode(uri)
	return true, info_tab
end

local function save_file_redis(binary, cacheTime, type_val)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainSound", dfs_cfg["group_domain"])
	local url_args = 'isStorage=false&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local amrkey = basekey .. ".amr" 
	----------------------------------------------------------------------------------------
	-->以uuid.key存放在本地
	local tmp_voicekey = uuid .. "." .. type_val
	local tmp_amrkey = uuid .. ".amr"
	--convert format
	local amr_binary = binary
	if tostring(type_val) == "mp3" or tostring(type_val) == "wav" then
		local fd = io.open(tmp_voicekey, "w+")
		fd:write(binary)
		fd:close()
		os.execute(string.format("ffmpeg -i %s -y -ab 5.15k -ar 8000 -ac 1 %s", tmp_voicekey, tmp_amrkey))
		local fd = io.open(tmp_amrkey, "r")
		amr_binary = fd:read("*a")
		fd:close()
	
		os.execute(string.format("rm -f %s %s", tmp_voicekey, tmp_amrkey))
	else
		tmp_amrkey = tmp_voicekey
	end

	----------------------------------------------------------------------------------------
	--return infomation
	local info_tab = {
		url		= '',
		fileID		= uuid,
		fileSize	= #amr_binary,
	}
	-->amr
	local ok, result, memb = spx_utils.set_to_redis(amrkey, amr_binary, cacheTime)
	if not ok then 
		ok, result, memb = spx_utils.set_to_redis_spare(amrkey, amr_binary, cacheTime)
		if not ok then return false, "save to tsdb failed" end
	end
	
	local group = memb['mark']
	local uri = string.format(url_args, group, amrkey)
	info_tab['url'] = url_path .. "?" .. cutils.url_encode(uri)
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
	if tonumber(args['length'] or 0) ~= tonumber(#file_binary) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "file length")
	end
	--filter
	local type_val = args["Type"] 
	if not type_val or type_val == '' then
		type_val = 'amr'
	end
	--语音支持格式有amr,mp3,wav
	if type_val ~= 'amr' and type_val ~= 'mp3' and type_val ~= 'wav' then 
		only.log('E', "voice format error")
		return gosay.resp_msg(msg["MSG_ERROR_VOICE_TYPE"], "voice type error")
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

	local isStorage = args['isStorage'] 
	if not isStorage or isStorage == '' then
		isStorage = 'false'
	end
	local cacheTime = tonumber(args['cacheTime']) 	
	local ok, ret_val = nil, nil
	if isStorage == 'true' then
		ok, ret_val = save_file_dfs(file_binary, cacheTime, type_val)	
		if not ok or not ret_val then
			only.log('W','file save redis failed!')
			return gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed")
		end
	elseif isStorage == 'false' then
		ok, ret_val  = save_file_redis(file_binary, cacheTime, type_val)
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
