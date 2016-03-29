-- txt to voice 
-- 云知声接口
local utils     = require('utils')
local cutils    = require('cutils')
local supex	= require('supex')
local only      = require('only')
local sha       = require('sha1')
local link	= require('link')
local tts 	= require('luatts')
local data2wav  = require('data2wav')

local APP_KEY = "vyokebtp3llfkxr7jf7wnj3rnygfjynjpmkqnwad"
local SECRET_KEY = "65e0bad0282c05fdc1058789dced301a" 

module('unisound', package.seeall)

function wav2amr(wavkey)
	local uuid = cutils.uuid()
	local amrkey = uuid .. ".amr"
 	
	os.execute(string.format("ffmpeg -v fatal -i %s -y  -ab 5.15k -ar 8000 -ac 1 %s",wavkey,amrkey))
	local fd = io.open(amrkey, "r")
	if not fd then
		return nil, "wav 2 amr field!"
	end
	local amr_buffer = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -rf %s %s", wavkey, amrkey))
	return amr_buffer,nil
end

function handle(txt)
	--------txt --->---wav
	local handle = tts.tts_create(APP_KEY, SECRET_KEY);
	local uuid = cutils.uuid()
	local file = uuid .. ".wav"
	local d2w = data2wav.data2wav_create(file)
	
	tts.tts_text_put(handle, txt);
	local data = nil;
	local len = nil;
	local iscontinue = true;
	repeat 
		data, len, iscontinue = tts.tts_get_result(handle);
		if len~= 0 then
			data2wav.data2wav_put(d2w, data, len)
			only.log("D",string.format("got a little data %p, and the length of data was %d .", data, len))
		else
			only.log("D","no data, try again.")
		end
	until not iscontinue

	data2wav.data2wav_transfer(d2w)
	tts.tts_destroy(handle)
	
	--------wav --->---amr
	local amr_buffer,amr_error = wav2amr(file)
	if not amr_buffer then
		only.log('E',string.format("mp3 to amr failed %s",amr_error))
		return false,amr_error
	end
	
	return true,amr_buffer
end
