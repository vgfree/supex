local shm_ffi = require("shm-ffi")

local SHM_SIZE_MAX = 1024000
local SHM_BASE_IDX = 1000000

local shm_over_idx = SHM_BASE_IDX
local shm_data_idx = 2 * SHM_BASE_IDX

module("tts", package.seeall)

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
	
		"open/?.lua;",
	}
	package.path = table.concat(path_list) .. package.path
	
	local shm_ffi = require("shm-ffi")

	local shm_over_idx = %s
	local shm_data_idx = %s

	os.execute("sleep 2")--FIXME

	shm_ffi.push(shm_data_idx, false, "0ello worldaaaaaaaaaaa")
	shm_ffi.push(shm_over_idx, false, "yes")
]]

function main(idle, args)
	jit.opt.start(0)
	--jit.opt.start("-dce")
	--jit.opt.start("hotloop=10", "hotexit=2")
	local work_coding = string.format(work_format, shm_over_idx, shm_data_idx)
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
