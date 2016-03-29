-- txt to voice 
-- 灵云tts接口
local shm_ffi = require("shm-ffi")
local SHM_SIZE_MAX = 1024000
local SHM_BASE_IDX = 1000000

local shm_over_idx = SHM_BASE_IDX
local shm_data_idx = 2 * SHM_BASE_IDX

module("ly_tts", package.seeall)

function init(idx)
	shm_over_idx = shm_over_idx + idx
	shm_data_idx = shm_data_idx + idx
	shm_ffi.init(shm_over_idx, SHM_SIZE_MAX)
	shm_ffi.init(shm_data_idx, SHM_SIZE_MAX)
end

local work_format = [[
	local path_list = {
		"../../open/lib/?.lua;",
		"../../open/apply/?.lua;",
		"../../open/spxonly/?.lua;",
		"../../open/linkup/?.lua;",
		"../../open/public/?.lua;",
		"./lua/spxapi/deploy/?.lua;",
		"open/?.lua;",
	}
	package.path = table.concat(path_list) .. package.path
	package.cpath = "../../open/lib/?.so;" .. package.cpath
	
	local shm_ffi = require("shm-ffi")
	local utils = require("utils")
	local cutils = require("cutils")
	local jTTS = require("jTTS")
	local link = require("link")
	local jTTSServer = link["OWN_DIED"]["http"]["jTTSServer"]

	local shm_over_idx = %s
	local shm_data_idx = %s

	local ok ,file_binary, file_length = nil,nil,nil
	if not ok or file_binary == nil or #file_binary < 1  then
		shm_ffi.push(shm_data_idx, false, "err")
	end

	local volume = 9
	local speed = 5

	local txt = "%s"
	local vId ="47FF1422-796F-427F-8408-EC5FD3367729"
	local server_info = string.format("%%s:%%s",jTTSServer.host,jTTSServer.port)
	local ok, file_binary , length = jTTS.text2voice( server_info , txt ,vId, volume, speed ) 
	if not ok then
		print( string.format("jTTS.text2voice failed,%%s!",file_binary))
		shm_ffi.push(shm_data_idx, false, "err")
	end

	local uuid = cutils.uuid()
	local wavkey = uuid .. ".wav"
	local amrkey = uuid .. ".amr"
	local fd = io.open(wavkey, "w+")
	fd:write(file_binary)
	fd:close()

 	os.execute(string.format("ffmpeg -v fatal -i %%s -y  -ab 5.15k -ar 8000 -ac 1 %%s",wavkey,amrkey))
	local fd = io.open(amrkey, "r")
	if not fd then
		shm_ffi.push(shm_data_idx, false, "err")
	end
	local amr_buffer = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -rf %%s %%s", wavkey, amrkey))

	shm_ffi.push(shm_data_idx, false, amr_buffer)
	shm_ffi.push(shm_over_idx, false, "yes")
]]

function main(idle, args, txt)
	jit.opt.start(0)
	--jit.opt.start("-dce")
	--jit.opt.start("hotloop=10", "hotexit=2")
	local work_coding = string.format(work_format, shm_over_idx, shm_data_idx,txt)
	local work_string = string.format("luajit -e '%s' &", work_coding)
	print(work_string)
	os.execute(work_string)
	local data = nil
		local ok = nil
	while true do
		ok = shm_ffi.pull(shm_over_idx, true)
		if ok == "yes" then
			data = shm_ffi.pull(shm_data_idx, true)
			break
		elseif ok == "err" then
			break
		else
			if idle then
				idle( args )
			end
		end
	end
	shm_ffi.zero(shm_over_idx, true)
	shm_ffi.zero(shm_data_idx, true)
	return data
end
