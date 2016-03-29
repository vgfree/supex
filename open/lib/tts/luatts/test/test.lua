package.cpath = "../lib/?.so;" .. package.cpath
local tts 		= require('luatts')
local data2wav           = require('data2wav')

local function handle()
	local APP_KEY = "vyokebtp3llfkxr7jf7wnj3rnygfjynjpmkqnwad"
	local SECRET_KEY = "65e0bad0282c05fdc1058789dced301a" 
	
	local handle = tts.tts_create(APP_KEY, SECRET_KEY);
	local file = "./file.wav"
	local d2w = data2wav.data2wav_create(file)
	
	tts.tts_text_put(handle, '上海语境汽车信息技术有限公司abc');
	local data = nil;
	local len = nil;
	local iscontinue = true;
	repeat 
		data, len, iscontinue = tts.tts_get_result(handle);
		if len~= 0 then
			data2wav.data2wav_put(d2w, data, len)
			print(string.format("got a little data %p, and the length of data was %d .", data, len));
		else
			print("no data, try again.");
		end
	until not iscontinue

	data2wav.data2wav_transfer(d2w)
	tts.tts_destroy(handle)
end

do
	handle();
end
