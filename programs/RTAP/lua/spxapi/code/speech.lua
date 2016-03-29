-- txt to voice 
-- 思必驰接口

local utils     = require('utils')
local cutils    = require('cutils')
local supex	= require('supex')
local only      = require('only')
local sha       = require('sha1')
local link	= require('link')

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
module('speech', package.seeall)

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

function handle(txt)
	local speech_Rate_str,speech_Announcer_str,speechVolume = speechset()
	
	--------txt --->---mp3
	local ok,file_binary = txt_to_voice(speech_Rate_str,speech_Announcer_str, speechVolume ,txt)
	if not ok or file_binary == nil or #file_binary < 1  then
		return false,"txt to voice"
	end
	
	--------mp3 --->---amr
	local amr_buffer,amr_error = mp32amr(file_binary)
	if not amr_buffer then
		only.log('E',string.format("mp3 to amr failed %s",amr_error))
		return false,amr_error
	end
	
	return true,amr_buffer
end
