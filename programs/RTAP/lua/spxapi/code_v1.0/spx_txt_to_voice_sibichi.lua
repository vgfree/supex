-- txt to voice 
-- 思必驰接口

local utils     = require('utils')
local cutils    = require('cutils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local gosay     = require('gosay')
local sha       = require('sha1')
local link	= require('link')
local redis_api = require('redis_short_api')
local ffmpeg    = require('lua_ffmpeg')

local host_s = "s.api.aispeech.com"
local port_s = 80
local appkey_s = "139762545900025f" 
local secretkey_s = "0588ddbd719e6962375510881f648c1e" 

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

module('spx_txt_to_voice', package.seeall)

local function check_mp3_head( data_binary )
	if not data_binary then return false end
	if string.byte(data_binary,1) == 0xff then return true end
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
		only.log('E',string.format('[spx][txt:%s][speech failed]',txt))
		return false,nil
	end

	if not ret then
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
		return false,nil
	end
	return true,data_binary
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

function save_amr_to_redis(amr_buffer,txt)
	local uuid = cutils.uuid()
	keyamr = uuid .. ".amr"
	keytxt = uuid .. ":" .. "keytxt"
	redis_api.cmd("tmpvoice","setex",keytxt,24*60*60,txt)
	redis_api.cmd("tmpvoice","setex",keyamr,300,amr_buffer)

	ret_url = "redis://" .. keyamr
	return ret_url
end

function handle()
	local args = supex.get_our_body_table()
	if not args or not next(args) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'args')
	end
	if not args['text'] then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'text')
	end

	local txt = work_with_text(args['text'])

	local speech_Rate_str,speech_Announcer_str,speechVolume = speechset()

	--------txt --->---mp3
	local ok,file_binary = txt_to_voice(speech_Rate_str,speech_Announcer_str, speechVolume ,txt)
	if not ok or file_binary == nil or #file_binary < 1  then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'txt to voice')
	end
	
	--------mp3 --->---amr
	local amr_buffer,amr_len ,amr_error = ffmpeg.mp32amr(file_binary,#file_binary)
	if not amr_buffer then
		only.log('E',string.format("mp3 to amr failed %s",amr_error))
		return gosay.resp_msg(msg["MSG_ERROR_MP32VOICE"],amr_error)
	end
	
	local ret_url = save_amr_to_redis(amr_buffer,txt)
	
	local ret_url = string.format('{"url":"%s"}',ret_url)
	only.log('D',string.format('[spx][txt:%s][successs]', txt))
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ret_url)
end
