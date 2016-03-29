--spx txt to voice
local utils     = require('utils')
local cutils    = require('cutils')
local utils    	= require('utils')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local supex      = require('supex')
local scan      = require('scan')
local gosay     = require('gosay')
local cjson     = require('cjson')
local redis_api = require("redis_short_api")
local ly_tts 	= require('ly_tts')
local speech	= require('speech')
local unisound  = require('unisound')
local wstxt     = require('wstxt')
local luakv_api   = require('luakv_pool_api')

module('spx_txt_to_voice', package.seeall)

ly_tts.init(supex["__TASKER_NUMBER__"])

math.randomseed(os.time())

local dfsSaveSound = link.OWN_DIED.http.dfsSaveSound
local dfsserv = {
	host = dfsSaveSound.host,
	port = dfsSaveSound.port,
	}

function work_with_text(txt)-----处理文本

	txt = utils.url_decode(txt)
	txt = string.gsub(txt,"'","")

	---- 替换标点符号,替换双引号,避免转码错误
	txt = string.gsub(txt,',','，')
	txt = string.gsub(txt,'%.','。')
	txt = string.gsub(txt,':','：')
	txt = string.gsub(txt,"\"","")
	txt = string.gsub(txt,"\"","")
	txt = string.gsub(txt,"“","")
	txt = string.gsub(txt,"”","")
	--only.log('D',string.format("%s",txt))
	return txt
end

function handle()
	local args = supex.get_our_body_table()
	local cacheret = supex.get_our_body_data() 

	if utils.get_sha_tab_count(args) == 0 then
		args = supex.get_our_uri_table()
		cacheret = supex.get_our_uri_args()
	end
	if not args then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],"args is nil")
	end

	if not args['text'] or string.len(args['text']) < 1 or string.len(args['text']) > 160*3 then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],"text")
	end

	local ok,ret = luakv_api.cmd("owner","","get",cacheret)
	if ok and ret then
		only.log('D',"cache results")
		return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ret)
	end

	local txt = work_with_text(args["text"])
	only.log('D',"txt is : %s",txt)

 	-->speech/unisound/lytts
	local policy = args["policy"]
	local timenow,lasttime,randnum =0,0,0
	if not policy then
		-->Generate random number
		while (true) do
			timenow=os.time()
			if timenow~=lasttime then
				randnum = math.random(1, 3)
				break
			end
			lasttime=timenow
		end
	end
	-->txt to voice
	local ok, file_data = nil, nil
	if policy == "lytts" or randnum == 1 then
		-->ly_tts txt to voice
		file_data = ly_tts.main(lua_default_switch, supex["__TASKER_SCHEME__"],txt)
		if not file_data then 
			only.log('E',"ly_tts txt2voice feild %s ",file_data)
			return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "ly_tts txt2voice feild ")
		end
	elseif policy == "speech" or randnum == 2 then
		-->speech txt to voice
		ok , file_data = speech.handle(txt) 	
		if not ok then 
			only.log('E',"speech txt2voice feild %s ",file_data)
			return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "speech txt2voice feild ")
		end
	elseif policy == "unisound" or randnum == 3 then
		-->unisound txt to voice
		ok, file_data = unisound.handle(txt)
		if not ok then 
			only.log('E',"unisound txt2voice feild %s ",file_data)
			return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "unisound txt2voice feild ")
		end
--	elseif policy == "wstxt" or randnum == 4 then
--		-->wstxt txt to voice
--		ok, file_data = wstxt.handle(txt)
--		if not ok then 
--			only.log('E',"wstxt  txt2voice feild %s ",file_data)
--			return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "wstxt txt2voice feild ")
--		end
	end
	
	-->check is amr
	if not utils.check_is_amr(file_data,#file_data) then
		only.log('E',"txt2voice succ but file not amr %s",file_data)
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'txt2voice succ but file is not amr')
	end
		
	local url_tab = { 
       		appKey = args['appKey'],
    	}

	url_tab['length'] = #file_data
	local ok, secret = redis_api.cmd('public', 'hget', args['appKey'] .. ':appKeyInfo', 'secret')
	url_tab['sign'] = utils.gen_sign(url_tab,secret)
	url_tab['isStorage'] = args['isStorage'] or false
	url_tab['cacheTime'] = tonumber(args['cacheTime']) 

	local file = {
		file_name = "txt2voice.amr",
		data	  = file_data,
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

	-->cache results
	luakv_api.cmd("owner","","set",cacheret,ret_url)
	
	if cacheTime then
		cacheTime = cacheTime - 10
	end
	luakv_api.cmd("owner","","expire",cacheret,cacheTime or 60*60*12)

	only.log('D',string.format('[spx][txt:%s][successs]', txt))
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ret_url)
end
