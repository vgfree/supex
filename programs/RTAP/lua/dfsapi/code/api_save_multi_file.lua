-- save multi file
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

module('api_save_multi_file', package.seeall)

local function get_file_type(fileName)
	local fileType = nil
	for a in string.gfind(fileName or '','%.(%w+)') do
		fileType = a
	end
	return fileType
end

local successFile = {}

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

local function save_file(args)
	local fileName,fileBinary,fileType,fileLength = nil,nil,nil,nil
	for i,v in pairs(args) do
		if type(v) == "table" then
			fileName = v['file_name']
			fileBinary = v['data']
			fileType = get_file_type(fileName)
			print(fileName)
			if not fileType or string.find(fileType,"%.") or  #fileType < 1 or #fileType > 4 then
				only.log("E",string.format('--file:%s-->fileType is error:%s',fileName,fileType))
				break
			end
			local ok ,ret_url  = save_file_to_dfs(fileBinary,fileType)
			if ok then
				local fileInfo = {}
				fileInfo["fileUrl"]  = ret_url['url']
				fileInfo["fileID"]   = ret_url['fileID']
				fileInfo["fileType"] = fileType
				fileInfo["fileName"] = fileName
				fileInfo["fileSize"] = #fileBinary
				table.insert(successFile,fileInfo)
			end
		end
	end
end

function handle()
	local args  = supex.get_our_body_table()
	
	if not args then 
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],'args is nil')
	end

	if not args then 
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],'args is nil')
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
	
	save_file(args)

--	for a,v in pairs(successFile) do 
--		for a,v in pairs(v) do
--			only.log("D",'-----s--------------%s:--%s',a,v)
--		end
--	end 

	local success_file = "{}"
	if #successFile>0 then
		local ok_status, res = pcall(cjson.encode, successFile)
		if ok_status then
			success_file = res
		end
	end

	local ret_req = string.format('{"Totalcount":"%s","successfulCount":%s,"successFile":%s}',
			args['fileCount'],#successFile, success_file)
	
	ret_req = string.gsub(ret_req, "\\", "")
	
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], ret_req)
end

