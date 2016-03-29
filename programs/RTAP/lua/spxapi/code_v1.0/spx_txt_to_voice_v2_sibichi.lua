-- txt to voice save to dstb
-- 思必驰接口
local utils     = require('utils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local gosay     = require('gosay')
local sha       = require('sha1')
local link	= require('link')
local ffmpeg    = require('lua_ffmpeg')
local redis_api = require('redis_short_api')
local cjson    = require('cjson')

local host_s = "s.api.aispeech.com"
local port_s = 80
local appkey_s = "139762545900025f" 
local secretkey_s = "0588ddbd719e6962375510881f648c1e" 

local dfsSaveSound = link.OWN_DIED.http.dfsSaveSound

local serv = {
	host = dfsSaveSound.host,
	port = dfsSaveSound.port,
	}

local post_format =  "POST /api/v3.0/score HTTP/1.0\r\n" ..
"Host:%s:%s\r\n" ..
"Content-Length:%d\r\n" ..
"Content-Type:text/plain\r\n\r\n%s" 

---- 思必驰接口发言人
local speechAnnouncer_tab = {
	syn_chnsnt_anonyf   = "anony",
	syn_chnsnt_zhilingf = "zhiling",
}
local speechAnnouncer_index = {
	syn_chnsnt_anonyf   = 0,
	syn_chnsnt_zhilingf = 1,
}

module('spx_txt_to_voice_v2_bk', package.seeall)

local function check_mp3_head( data_binary )
	if not data_binary then return false end
	if string.byte(data_binary,1) == 0xff then return true end
end

local function check_is_amr(binary, length)
    if not binary then return false end
    if not length  or tonumber(length) < 15 then return false end
    local file_title = { [1] = "#!AMR\n", [2] = "#!AMR-WB\n",[3] = "#AMR_MC1.0\n", [4] = "#AMR-WB_MC1.0\n" }
    local is_amr = false
    local head_length = 0 
    for i, v in pairs(file_title) do
        if string.sub(binary,1,#v) == v then
            is_amr = true
            head_length = #v
            break
        end
    end
    return is_amr, head_length
end

local function txt_to_voice(speechRate, speechAnnouncer,  speechVolume , txt)
	if not txt then return false,nil end
	if #txt < 1 then return false,nil end
	local data_format = [[{"cmd":"start","param":{"app":{"applicationId":"%s","timestamp":"%s","sig":"%s"},"audio":{"audioType":"mp3","channel": 1,"sampleRate":16000,"sampleBytes":2},"request":{ "coreType":"cn.sent.syn","speechRate":%s,"speechVolume":%d,"res": "%s","refText":"%s"}}}]]
	local timestamp = os.time()
	local sign_string = string.format("%s%s%s", appkey_s,timestamp,secretkey_s)
	local sign_result = sha.sha1(sign_string)
	local body_result = string.format(data_format,appkey_s,timestamp,sign_result,speechRate, speechVolume, speechAnnouncer, txt)
	local req = string.format(post_format, host_s, tostring(port_s) , #body_result, body_result )
	local ok,ret = supex.http(host_s, port_s, req, #req)
	if not ok then
		--only.log('D',req)
		--only.log('E',string.format('host: %s , port : %s http post get txt_voice failed!!', host_s, port_s) )
		only.log('E',string.format('[spx][txt:%s][speech failed]',txt))
		return false,nil
	end

	if not ret then
		--only.log('D',req)
		--only.log('E','http post succed but return failed!!')
		return false,nil
	end

	local data_split = string.find(ret,'\r\n\r\n')
	if not data_split then
		return false,nil
	end
	local data_head = string.sub(ret,1,data_split)
	local data_binary = string.sub(ret,data_split+4,#ret)
	if  not ( string.find(data_head,'Content%-Type:% application/octet%-stream') 
		or string.find(data_head, 'Content%-Type:% audio/mpeg' )
		or check_mp3_head(data_binary) ) then
		--only.log('D','request succed,but get Content-Type: audio/mpeg,application/octet-stream and check_mp3_head failed!')
		return false,nil
	end
	return true,data_binary
end

function get_body()-----获取数据
	local args = supex.get_our_body_table()
	if not args or not next(args) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'args')
	end
	if not args['text'] then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'text')
	end
	return args
end

function work_with_text(text)-----处理文本
	local txt = text
	if not txt or string.len(txt) < 1 or string.len(txt) > 160*3 then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'text')
	end

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

function speechset()-----speech参数设置
	local speechRate = 1.0
	local speech_Rate_str = string.format("%.1f",speechRate)

	local speechVolume = 100

	local speech_Announcer_str = 'syn_chnsnt_anonyf'
	local is_ok = false
	for i,v in pairs(speechAnnouncer_tab) do 
		if tostring(speechAnnouncer) == tostring(v) then
			speech_Announcer_str = i
			is_ok = true
		end
	end

	if is_ok == false then
		speech_Announcer_str = 'syn_chnsnt_anonyf'
	end

	return speech_Rate_str,speech_Announcer_str,speechVolume
end

function handle()
	local args = get_body()
	
	local file = {
		file_name = "txt2voice.amr",
		data	  = nil,
		data_type = "application/octet-stream",
	}

	local tab = {
		appKey = args['appKey'],
		length = 0,
		
	}
	local txt = work_with_text(args['text'])

	local speech_Rate_str,speech_Announcer_str,speechVolume = speechset()

	--------txt --->---mp3
	local ok,file_binary = txt_to_voice(speech_Rate_str,speech_Announcer_str, speechVolume ,txt)
	if not ok or file_binary == nil or #file_binary < 1  then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'txt to voice')
	end
	
	-->>mp3 to amr
	local amr_buffer,amr_len ,amr_error = ffmpeg.mp32amr(file_binary,#file_binary)
	if not amr_buffer then
		only.log('E',string.format("mp3 to voice failed %s",amr_error))
      		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], amr_error)
	end
	
	-->>check is amr
	if not check_is_amr(amr_buffer,amr_len) then
		only.log('E',"mp3 to voice succed, but file is not amr format ")
      		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "file is not amr")
	end
		
	file['data'] = amr_buffer
	tab['length'] = #amr_buffer
	local ok, secret = redis_api.cmd('public', 'hget', args['appKey'] .. ':appKeyInfo', 'secret')
	tab['sign'] = utils.gen_sign(tab,secret)
	local req = utils.compose_http_form_request(serv,"dfsapi/v2/saveSound",nil,tab, "mmfile", file)	
	local ok, ret = supex.http(serv.host, serv.port, req, #req)
	if not ok then
		only.log("E","save to tsdb falied")
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'save dfs failed')
	end
	
	local ok_str = string.match(ret,'{.+}')
	local ok,ok_tab = pcall(cjson.decode, ok_str)
        if ok then
                only.log('D','[spx][txt:%s][successs]', ok_tab['RESULT']['url'])
                return supex.spill(gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ok_tab['RESULT']['url']))
        else
                return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'cjson.decode failed')
        end

end
