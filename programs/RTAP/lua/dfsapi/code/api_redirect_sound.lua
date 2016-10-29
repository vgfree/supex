local utils     = require('utils')
local cutils    = require('cutils')
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

module('api_redirect_sound', package.seeall)

function handle()
	local args = supex.get_our_body_table()
	if not args or not args["url"] then
		only.log('E', 'args is nil')
		local afp = supex.rgs(400)
		return supex.over(afp)
	end
	print(args["url"])

	local geturl_host, geturl_path = string.match(args["url"], "http://(..-)/(.+)")
	local geturl_request = string.format("GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: curl/7.47.1\r\nAccept: */*\r\n\r\n", geturl_path, geturl_host)
	local ok, ret = supex.http(geturl_host, 80, geturl_request, #geturl_request)
	if not ok then
		only.log("E", "%s [FAILED]", geturl_request)
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'get url data failed')
	end
	local file_data = string.match(ret,'HTTP.-\r\n\r\n(.+)')
	

	-->check is amr
	if not utils.check_is_amr(file_data,#file_data) then
		only.log('E',"file not amr %s",file_data)
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'file is not amr')
	end
		
	local url_tab = { 
       		appKey = args['appKey'],
    	}

	url_tab['length'] = #file_data
--	local ok, secret = redis_api.cmd('public', 'hget', args['appKey'] .. ':appKeyInfo', 'secret')
--	url_tab['sign'] = utils.gen_sign(url_tab,secret)
	url_tab['isStorage'] = true
	url_tab['cacheTime'] = 300

	local file = {
		file_name = "redirectSound.amr",
		data	  = file_data,
		data_type = "application/octet-stream",
	}
do return end
	-->save
	local dfsserv = {
		host = "127.0.0.1",
		port = 3333,
	}
	local req = utils.compose_http_form_request(dfsserv,"dfsapi/v2/saveSound",nil,url_tab, "mmfile", file)	
	local ok, ret = supex.http(dfsserv.host, dfsserv.port, req, #req)
	if not ok then
		only.log("E","save to tsdb falied")
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'save dfs failed')
	end

	local ok_str = string.match(ret,'{.+}')
	local ok,ok_tab = pcall(cjson.decode, ok_str)
	local ok, ret_url = pcall(cjson.encode, ok_tab['RESULT'])
       	if not ok then
		only.log("E","ret val cjson filed")
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'cjson failed')
	end
	
	ret_url = string.gsub(ret_url, "\\", "")
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ret_url)
end
