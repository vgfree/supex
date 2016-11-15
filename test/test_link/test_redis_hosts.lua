-- 
-- file: test_redis_hosts.lua
-- desc: test
-- 

local redis_pool_hosts = require("redis_pool_hosts") 
local redis_died_hosts = require("redis_died_hosts") 

local function dump(tab)
	for k,v in pairs(tab) do
		print("--------------------")
		print(k)
		print("mode: " .. v["mode"])

		print("")
		-- single
		if v["mode"] == "single" then
			print("host: " .. v["host"])
			print("port: " .. v["port"])
		end

		-- multi
		if v["mode"] == "multi" then
			if v["master"] and #v["master"] ~= 0 then
				for i=1,#v["master"] do
					print("--master[" .. i .. "]")
					print("host: " .. v["master"][i]["host"])
					print("port: " .. v["master"][i]["port"])
				end
			end
			if v["slave"] and #v["slave"] ~= 0 then
				for i=1,#v["slave"] do
					print("--slave[" .. i .. "]")
					print("host: " .. v["slave"][i]["host"])
					print("port: " .. v["slave"][i]["port"])
				end
			end
		end
	end
	print("--------------------")
end

-- for test
function main()
	-- redis_pool_hosts.init()
	-- -- dump(redis_pool_hosts.POOL_HOSTS)
	-- 
	-- -- test cmd.
	-- for i=1,5 do
	-- 	local ok, ret = redis_pool_hosts.cmd("p_redis_single", "set", "ps_key"..i, "val"..i)
	-- 	if not ok then
	-- 		print("err_msg: " .. ret)
	-- 	end
	-- end

	-- for i=1,5 do
	-- 	local ok, ret = redis_pool_hosts.cmd("p_redis_multi", "set", "ps_key"..i, "val"..i)
	-- 	if not ok then
	-- 		print("err_msg: " .. ret)
	-- 	end
	-- end

	redis_died_hosts.init()
	-- dump(redis_died_hosts.DIED_HOSTS)
	for i=1,5 do
		local ok, ret = redis_died_hosts.cmd("d_redis_multi", "get", "ps_key" .. i)
		if not ok then
			print("cmd error")
		end
	end
	-- dump(redis_died_hosts.DIED_HOSTS)
end

main()
