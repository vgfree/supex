local utils     = require('utils')
local cutils    = require('cutils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local gosay     = require('gosay')
local spx_utils = require('spx_utils')
local cjson     = require('cjson')
local dfs_cfg 	= require('dfs_cfg')

module('api_save_file', package.seeall)

--最大重复上传次数
local REPEAT_UPLOAD_COUNT = 10

--默认重复上传次数
local DEFAULT_UPLOAD_COUNT = 3

local function save_file_to_dfs(binary,fileType)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainfile", dfs_cfg["file_domain"])
	local url_args = 'group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local key = basekey .. "." .. fileType 
	local info_tab = {
		url		= '',
		fileID		= uuid,
		fileSize	= #binary,
	}
	-->amr
	local ok, result, memb = spx_utils.set_to_dfsdb(key, binary)
	if not ok then 
		ok, result, memb = spx_utils.set_to_dfsdb_spare(key, binary)
		if not ok then return false, "save to tsdb failed" end
	end

	local group = memb['mark']
	local uri = string.format(url_args, group, key)
	info_tab['url'] = url_path .. "?" .. cutils.url_encode(uri)
	return true, info_tab
end

local function check_parameter(args)

	local fileType = args['fileType']
	if not fileType or string.find(fileType,"%.") or  #fileType < 1 or #fileType > 4 then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "fileType")
	end
	local retry = tonumber(args['retry']) or DEFAULT_UPLOAD_COUNT
	if retry > REPEAT_UPLOAD_COUNT then 
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "retry")
	end  
	
	--check
	local url_tab = { 
		type_name = 'system',
		app_key = '',
		client_host = '',
		client_body = '',
   	 }
	
	url_tab['app_key'] = args['appKey']

--	safe.sign_check(args,url_tab)

	args['retry'] = retry
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
	
	check_parameter(args)

	if tonumber(#file_binary <= 0) or tonumber(args['length'] or 0) ~= tonumber(#file_binary) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "file length")
	end

	local ok_status,ret_url = nil,nil
	for i = 1,tonumber(args['retry']) do 
		ok_status,ret_url  = save_file_to_dfs(file_binary,args['fileType'])
		if ok_status and ret_url then
			break
		else
			if i == tonumber(args['retry']) then 
				only.log('W','file save dfs failed!')
				return supex.spill(gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed"))
			end
		end
	end
	
	local ok, dfs_url_info = pcall(cjson.encode, ret_url)
	dfs_url_info = string.gsub(dfs_url_info, "\\", "")
	print(dfs_url_info)
	
	return supex.spill(gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], dfs_url_info))
end
