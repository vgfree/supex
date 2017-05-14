local num_cores = 0

local function get_num_cores()
        if num_cores == 0 then
                f = io.popen("nproc")
                num_cores = tonumber(f:read("*n"))
                f:close()
        end

        return num_cores
end

local function get_affinity_mask(number)
        local num_cores = 8 --get_num_cores()
        local thread_affinity = {}

        thread_affinity[1] = ""
        thread_affinity[2] = ""
        
        for i=1, num_cores do
                thread_affinity[1] = (i == (num_cores + 1 - (number*2 - 1))) and (thread_affinity[1] .. "1") or (thread_affinity[1] .. "0")
                thread_affinity[2] = (i == (num_cores + 1 - number*2)) and (thread_affinity[2] .. "1") or  (thread_affinity[2] .. "0")
        end

        return thread_affinity
end

for i = 1, 4 do
	local a = get_affinity_mask(i)
	print(a[1], a[2])
end
