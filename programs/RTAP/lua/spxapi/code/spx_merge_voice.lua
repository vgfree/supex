-- merge voice
-- 合并音频文件

local utils     = require('utils')
local cutils    = require('cutils')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local gosay     = require('gosay')
local cjson     = require('cjson')
local redis_api = require('redis_short_api')
local supex      = require('supex')

module('spx_merge_voice', package.seeall)

local dfsSaveSound = link.OWN_DIED.http.dfsSaveSound
local dfsserv = {
	host = dfsSaveSound.host,
	port = dfsSaveSound.port,
	}

-- [
--     {
--         "index": "1",
--         "msgType": "link",
--         "value": "http://g4.tweet.daoke.me/group4/M07/00/50/c-dJT1UGRCWAUFq8AAARPSt5mpE640.amr",
--         "default": "",
--         "filesize":"0",
--         "isAllowFailed": "0"
--     },
--     {
--         "index": "2",
--         "msgType": "redis_variable",
--         "value": "private|get|zHnlgrNpiq:nicknameURL",
--         "default": "http://g4.tweet.daoke.me/group4/M05/E7/93/c-dJT1SUWsmAKkAfAAAGedUbVVc708.amr",
--         "filesize":"0",
--         "isAllowFailed": "1"
--     },
--     {
--         "index": "5",
--         "msgType": "redis_fixation",
--         "value": "zHnlgrNpiq|nicknameURL",
--         "default": "http://g4.tweet.daoke.me/group4/M05/E7/93/c-dJT1SUWsmAKkAfAAAGedUbVVc708.amr",
--         "filesize":"0",
--         "isAllowFailed": "1"
--     },
--     {
--         "index": "6",
--         "msgType": "mp3_binary",
--         "value": "",
--         "default": "",
--         "filesize":"20496",
--         "isAllowFailed": "1"
--     },
--     {
--         "index": "7",
--         "msgType": "wav_binary",
--         "value": "",
--         "default": "",
--         "filesize":"20496",,
--         "isAllowFailed": "1"
--     },
--     {
--         "index": "8",
--         "msgType": "amr_binary",
--         "value": "",
--         "default": "",
--         "filesize":"20496",
--         "isAllowFailed": "1"
--     }
-- ]

function mp32amr(file_binary)
	local uuid = cutils.uuid()
	local mp3key = uuid .. ".mp3"
	local amrkey = uuid .. ".amr"
	local fd = io.open(mp3key, "w+")
	fd:write(file_binary)
	fd:close()

 	os.execute(string.format("ffmpeg -v fatal -i %s -y  -ab 5.15k -ar 8000 -ac 1 %s",mp3key,amrkey))
	local fd = io.open(amrkey, "r")
	if not fd then
		return nil, "mp3 to amr failed!"
	end
	local amr_buffer = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -rf %s %s", mp3key, amrkey))
	return amr_buffer,nil

end

function wav2amr(file_binary)
	local uuid = cutils.uuid()
	local wavkey = uuid .. ".wav"
	local amrkey = uuid .. ".amr"
	local fd = io.open(wavkey, "w+")
	fd:write(file_binary)
	fd:close()

 	os.execute(string.format("ffmpeg -v fatal -i %s -y  -ab 5.15k -ar 8000 -ac 1 %s",wavkey,amrkey))
	local fd = io.open(amrkey, "r")
	if not fd then
		return nil ,"wav to amr failed!"
	end
	local amr_buffer = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -f %s %s", wavkey, amrkey))
	return amr_buffer,nil

end

local function convert_file_to_amr(file_binary, file_format)
	file_format = string.lower(file_format)
	if file_format == "amr" then
		if utils.check_is_amr(file_binary,#file_binary) then
			return true,file_binary
		end
	elseif file_format == "mp3" then
			local amr_buffer,amr_len ,amr_error = mp32amr(file_binary)
			if not amr_buffer then
				only.log('E',string.format("mp3 to voice failed %s",amr_error))
				return nil,nil
			end
			return true,amr_buffer
	elseif file_format == "wav" then
		if utils.check_is_wav(file_binary,#file_binary) then
			local amr_buffer,amr_len ,amr_error = wav2amr(file_binary)
			if not amr_buffer then
				only.log('E',string.format("wav to voice failed %s",amr_error))
				return nil,nil
			end
			return true,amr_buffer
		end
	end
	return nil,nil
end

---- 根据file_url获取文件内容下发
function get_file_data_by_http(file_url, max_lenth)
	if not file_url then return false,'url is error' end 
	local tmp_domain = string.match(file_url,'http://[%w%.]+:?%d*/')
	if not tmp_domain then return false,'domain get failed' end 
	local domain = string.match(tmp_domain,'http://([%w%.]+:?%d*)/')
	local urlpath = string.sub(file_url,#tmp_domain,#file_url)
	if not urlpath then return false,'' end
	local host = domain
	local port = 80
	if string.find(domain,':') then
		host = string.match(domain,'([%w%.]+)')
		port = string.match(domain,':(%d+)')
		port = tonumber(port) or 80
	end
	return get_file_by_url(host, port , urlpath, max_lenth )
end

function get_file_by_url(host_name, host_port , file_url, max_lenth)

	local data_format =  'GET %s HTTP/1.0\r\n' ..
						'HOST:%s:%s\r\n\r\n'
	local req = string.format(data_format,file_url,host_name,host_port)
	local ok,ret = supex.http(host_name ,host_port , req, #req, 1, max_lenth or 0)
	if not ok then
		only.log('E',string.format('-->--host:%s  port:%s get dfs data when send get request failed!', host_name, host_port ) )
		return false,'' 
	end
	local split = string.find(ret,'\r\n\r\n')
	if not split then
		return false , '' 
	end
	local head = string.sub(ret,1,split)
	local file_data = string.sub(ret,split+4,#ret)
	return true , file_data
end

--link
-- {
-- 	"index": "1",
-- 	"msgType": "link",
-- 	"value": "tttt://g4.tweet.daoke.me/group4/M07/00/50/c-dJT1UGRCWAUFq8AAARPSt5mpE640.amr",
-- 	"default": "",
-- 	"filesize":"0",
-- 	"isAllowFailed": "0"
-- },
local function get_link_function( value, default, filesize )
	
	-- local value_1 = string.gsub(value,"%%","%%%%")
	-- local default_1 = string.gsub(default,"%%","%%%%")
	-- local filesize_1 = string.gsub(filesize,"%%","%%%%")
	-- only.log('E',string.format("[get_link_function] %s, %s, %s  " , value_1, default_1, filesize_1 ))
	
	local file_type = string.sub(value,-3)
	if not file_type then
		return false,nil
	end

	local ok,  binary = get_file_data_by_http(value,tonumber(filesize) )
	if ok and binary then
		---- change file format to amr 
		return convert_file_to_amr(binary, file_type)
	end

	if default and (string.find(tostring(default),"http://")) then
		file_type = string.sub(default,-3)
		ok,  binary = get_file_data_by_http(default,tonumber(filesize) )
		if ok and binary then
			---- change file format to amr 
			return convert_file_to_amr(binary, file_type)
		end
	end
end

local function get_link_binary(tab_info)
	for i, v in pairs(tab_info) do
		only.log('D',"[get_link_binary] %s  %s  " , i , v )
	end
	return get_link_function(tab_info['value'],tab_info['default'],tab_info['filesize'])
end


--     {
--         "index": "2",
--         "msgType": "redis_variable",
--         "value": "private|get|zHnlgrNpiq:nicknameURL",
--         "default": "http://g4.tweet.daoke.me/group4/M05/E7/93/c-dJT1SUWsmAKkAfAAAGedUbVVc708.amr",
--         "filesize":"0",
--         "isAllowFailed": "1"
--     },
function get_redis_variable_binary(tab_info)
	local parameter = tab_info['value']
	local par_tab = utils.str_split(parameter,"|")
	if not par_tab or #par_tab ~= 3 then
		only.log('E',"[get_redis_variable_binary] parameter is error , %s ", parameter )
		return false,nil
	end

	--only.log('D',string.format("variable====tab[1]:%s,tab[2]:%s,tab[3]:%s",par_tab[1],par_tab[2],par_tab[3]))

	local ok, value = redis_api.cmd(par_tab[1], par_tab[2], par_tab[3])
	if not ok or not value then
		only.log('E',"[get_redis_variable_binary] parameter redis get failed , %s , val:%s  ", parameter , value)
		return false,nil
	end
	return get_link_function(value , tab_info['default'],tab_info['filesize'])
end

--     {
--         "index": "5",
--         "msgType": "redis_fixation",
--         "value": "zHnlgrNpiq|nicknameURL",
--         "default": "http://g4.tweet.daoke.me/group4/M05/E7/93/c-dJT1SUWsmAKkAfAAAGedUbVVc708.amr",
--         "filesize":"0",
--         "isAllowFailed": "1"
--     },
function get_redis_fixation_binary(tab_info)
	local parameter = tab_info['value']
	local par_tab = utils.str_split(parameter,"|")
	if not par_tab or #par_tab ~= 2 then
		only.log('E',string.format("[get_redis_fixation_binary] parameter is error , %s ", parameter ))
		return false,nil
	end

	--only.log('D',string.format("fixation==tab[1]:%s,tab[2]:%s,tab[3]:%s",par_tab[1],par_tab[2],par_tab[3]))
	local tmp_info = {
		nicknameURL = { [1] = "private", [2] = "get",[3] = "%s:nicknameURL" } ,
		groupURL    = { [1] = "private", [2] = "get",[3] = "%s:channelNameUrl" } ,
	}

	local value = par_tab[1]
	local key = par_tab[2]

	if not tmp_info[key] then
		only.log('E',"[get_redis_fixation_binary] parameter is error , %s [nicknameURL|key]", parameter )
	end

	local ok, value = redis_api.cmd(tmp_info[key][1], tmp_info[key][2], string.format(tmp_info[key][3],value))
	if not ok or not value then
		only.log('E',"[get_redis_fixation_binary] parameter redis get failed , %s , val:%s  ", parameter , value)
		return false,nil
	end
	return get_link_function(value , tab_info['default'],tab_info['filesize'])
end

--     {
--         "index": "6",
--         "msgType": "mp3_binary",
--         "value": "",
--         "default": "",
--         "filesize":"20496",
--         "isAllowFailed": "1"
--     },
--     {
--         "index": "7",
--         "msgType": "wav_binary",
--         "value": "",
--         "default": "",
--         "filesize":"20496",,
--         "isAllowFailed": "1"
--     },
--     {
--         "index": "8",
--         "msgType": "amr_binary",
--         "value": "",
--         "default": "",
--         "filesize":"20496",
--         "isAllowFailed": "1"
--     }

local function get_really_binary( tab_info, binary )
	local file_type = string.lower(tab_info['msgType'])
	if file_type == "amr_binary" then
		return true,binary
	elseif file_type == "wav_binary" then
		return convert_file_to_amr(binary,"wav")
	elseif file_type == "mp3_binary" then
		return convert_file_to_amr(binary,"mp3")
	end
end


function handle()
	local args = supex.get_our_body_table()
	
	local file_name,file_binary = nil,nil
	for i,v in pairs(args) do
		if type(v) == "table" then
			file_name = v['file_name']
			file_binary = v['data']
			break
		end
	end
	
	if not args then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "arg is nil")
	end
    	
	if not args['appKey'] then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "appKey")
    	end

	local tab = {
		type_name = 'system',
		app_key  = '',
		client_host = '',
		client_body = '',
	}
	tab['app_key'] = args['appKey']	
	--	safe.sign_check(args,tab)

	local parameter = args['parameter']
	local ok , tab_info = pcall(cjson.decode,parameter)
	if not ok or not tab_info or #tab_info < 1 then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "parameter")
	end

	-->sort of index
	sort_func = function(a, b) return (tonumber(b["index"]) or 0) > (tonumber(a["index"]) or 0)end
        table.sort(tab_info, sort_func )

        local tab_fun = {
    		link = get_link_binary,
    		redis_variable = get_redis_variable_binary,
    		redis_fixation = get_redis_fixation_binary,
    		mp3_binary = get_really_binary,
    		wav_binary = get_really_binary,
    		amr_binary = get_really_binary,
	}

	local tab_buffer = {}
	for k ,v in pairs(tab_info) do
		
		only.log('D',string.format("[loop] %s ,%s,%s  ", k, v, v['msgType']) )

		if not tab_fun[v['msgType']] then
			return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "msgType")
		end
		if (string.find(v['msgType'] ,"binary") )  then
			if not file_binary then
				return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], v['msgType']  .. " binary not exists")
			end
			local ok, tmp_buffer = tab_fun[v['msgType']]( v, file_binary )
			if tostring(v['isAllowFailed']) == "0" and ( not ok or not tmp_buffer  ) then
				return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], v['msgType']  .. " get binary failed")
			end
			if tmp_buffer then
				table.insert(tab_buffer,tmp_buffer)
			end
		else
			local ok, tmp_buffer = tab_fun[v['msgType']]( v )
			if tostring(v['isAllowFailed']) == "0" and ( not ok or not tmp_buffer  ) then
				return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], v['msgType']  .. " get binary failed")
			end
			if tmp_buffer then
				table.insert(tab_buffer,tmp_buffer)
			end
		end
	end

	local amr_buffer = ''
	local amr_len  = 0
	for i, v in pairs(tab_buffer) do
		amr_buffer = amr_buffer .. v
	end
	amr_len = #amr_buffer

	if not utils.check_is_amr(amr_buffer,amr_len) then
		only.log('D',"mp3 to voice succed, but file is not amr format ")
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], v['msgType']  .. "file is not amr")
	end

	--only.log('D',string.format(" file_len:%d, merged multimedia amr_len:%d  " , file_len,amr_len))
	local url_tab = { 
       		appKey = args['appKey'],
    	}

	url_tab['length'] = #amr_buffer
	local ok, secret = redis_api.cmd('public', 'hget', args['appKey'] .. ':appKeyInfo', 'secret')
	url_tab['sign'] = utils.gen_sign(url_tab,secret)
	url_tab['isStorage'] = args['isStorage'] or false
	url_tab['cacheTime'] = tonumber(args['cacheTime']) 

	local file = {
		file_name = "txt2voice.amr",
		data	  = amr_buffer,
		data_type = "application/octet-stream",
	}

	-->save
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

	only.log('D',string.format('[spx][txt:%s][successs]', txt))
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ret_url)
end
