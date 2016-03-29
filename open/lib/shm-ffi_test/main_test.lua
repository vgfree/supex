package.path = "../?.lua;" .. package.path
local shm_ffi = require("shm-ffi")

shm_ffi.init(1000, 30000000)
shm_ffi.init(2000, 30000000)



function tts()
	os.execute("luajit work_test.lua &")
--print("111111111111111111")
	local data = nil
	while true do
--print("222222222222222222")
		local ok = shm_ffi.pull(1000, true)
		if ok == "yes" then
--print("333333333333333333")
			data = shm_ffi.pull(2000, true)
			break
		elseif ok == "err" then
--print("444444444444444444")
			break
		else
--			print(ok)
		end
	end
	shm_ffi.zero(1000, true)
	shm_ffi.zero(2000, true)
	return data
end


print(tts())
