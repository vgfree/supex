--txt to voice
-- 灵云tts
local utils     = require('utils')
local cutils    = require('cutils')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local gosay     = require('gosay')
local cjson     = require('cjson')
local jTTS      = require("jTTS")
local redis_api = require("redis_short_api")
local supex	=require("supex")

module('spx_txt_to_voice', package.seeall)

local jTTSServer = link["OWN_DIED"]["http"]["jTTSServer"]

local function check_mp3_head( data_binary )
	if not data_binary then return false end
	if string.byte(data_binary,1) == 0xff then return true end
end

function check_is_amr(binary, length)
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

--文本转语音 txt to wav
local function txt_to_voice(txt , speechVolume,  speechSpeed )
	local volume = tonumber(speechVolume) or 9
	local speed = tonumber(speechSed) or 5

	local server_info = string.format("%s:%s",jTTSServer.host , jTTSServer.port)
	local vId ="47FF1422-796F-427F-8408-EC5FD3367729"
	local ok, buffer , length = jTTS.text2voice( server_info , txt ,vId, volume, speed ) 
	if ok and buffer and #buffer > 100 then
		return true, buffer, length
	else
		only.log('W','jTTS.text2voice failed,%s!',buffer)
		return nil
	end
end

--处理文本
function work_with_text(text)
	text = utils.url_decode(text)
	text = string.gsub(text,"'","")
	
	local txt = text
	if not txt or string.len(txt) < 1 or string.len(txt) > 160*3 then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],"text")
	end

	---- 替换标点符号,替换双引号,避免转码错误
	txt = string.gsub(txt,',','，')
	txt = string.gsub(txt,'%.','。')
	txt = string.gsub(txt,':','：')
	txt = string.gsub(txt,"\"","")
	txt = string.gsub(txt,"\"","")
	txt = string.gsub(txt,"“","")
	txt = string.gsub(txt,"”","")
	
	only.log('D',string.format("%s",txt))
	
	return txt
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
		return nil, nil,"wav to amr failed!"
	end
	local amr_buffer = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -f %s %s", wavkey, amrkey))
	return amr_buffer,#amr_buffer,nil

end

function handle()
	local args = supex.get_our_body_table()
	if not args then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],"args is nil")
	end

	if not args['text'] then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],"text")
	end

	local txt = work_with_text(args['text'])
	if not txt then return end

	only.log('D',"%s",txt)

	-->txt to wav
	local ok ,file_binary, file_length = txt_to_voice(txt , args['speechVolume'], args['speechSpeed'] )
	if not ok or file_binary == nil or #file_binary < 1  then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],"txt to voice")
	end
	
	local amr_buffer,amr_len ,amr_error = wav2amr(file_binary)
	if not amr_buffer or not check_is_amr(amr_buffer,amr_len) then
		only.log('E',string.format("wav to amr failed %s",amr_error))
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], amr_error)
	end
	
	local ret_url = save_amr_to_redis(amr_buffer,txt)
	
	local ret_url = string.format('{"url":"%s"}',ret_url)
	only.log('D',string.format('[spx][txt:%s][successs]', txt))
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ret_url)
	
end
