--微软的第三方接口是TTS接口

local cutils    = require('cutils')
local only      = require('only')
local link      = require('link')
local supex	= require('supex')

local wsapi = link["OWN_DIED"]["http"]["wsapiServer"]

module('wstxt', package.seeall)
 
local function wsapi_txt_2_wav( text )

	local post_data = 'POST /wsapitext HTTP/1.1\r\n' ..
					'Host:%s:%s\r\n' ..
					'Content-Length:%d\r\n' ..
					'Content-Type:application/x-www-form-urlencoded\r\n\r\n%s'

	local tmp = string.format('{"voi_param":{"voi_uuid":"aaaaaaaa","voi_text":"%s"}}',  text  )
	local req = string.format(post_data,wsapi.host,wsapi.port,#tmp,tmp)

	only.log('D',req)

	local ok , ret = supex.http(wsapi.host,wsapi.port,req,#req)
	if not ok or not ret then
		only.log('E',string.format("call wstxt2voice failed, %s:%s %s", wsapi.host,wsapi.port, text ) )
		return false
	end
	
	local data = string.match(ret,"(RIFF.*)")
	if not data then
		only.log('E',"wsapi data failed, binary is error %s", ret )
		return false
	end
	return data
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

function handle(txt)
	local file_binary = wsapi_txt_2_wav(txt)
	if not file_binary then
		return false,"txt to voice err"
	end


	local amr_buffer,amr_len,amr_error = wav2amr(file_binary)
	if not amr_buffer then
		only.log('E', "wav to voice failed %s", amr_error)
		return false,amr_error
	end
	
	return true,amr_buffer
end
