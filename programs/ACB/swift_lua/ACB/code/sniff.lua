local only = require('only')
local supex = require('supex')
local cjson = require("cjson")
local cfg = require("cfg")

module('sniff', package.seeall)

local g_pick_time = 0

function handle()
        local data_len = 0
	local log_lat = 0
	local string_child = "/"
	local our_body_table = supex.get_our_body_table()
	local our_body_data = supex.get_our_body_data()
        local string_data = ":" .. our_body_table["IMEI"].."/"
	local distance = cfg['CHECK_DISTANCE']
        local area_bl = cfg['area_bl']
  	local interval_time = cfg['INTERVAL_TIME']
        --更新取包时间
        if g_pick_time + interval_time < os.time() then
                g_pick_time = os.time()
        end 
            
        --将表中的数据拼成字符串  
        for i=1,#our_body_table["GPSTime"] do
		--测试用调整时间
--[[		if our_body_table["GPSTime"][i] < g_pick_time then 
			our_body_table["GPSTime"][i] = (os.time() - 60) + (our_body_table["GPSTime"][i] % 60) 
		end
	
		only.log('D',"data_time;"..our_body_table["GPSTime"][i])
]]--	
		--将要取的时间点拼成字符串
                if our_body_table["GPSTime"][i] == g_pick_time then
                        data_len = data_len + 1 
			log_lat = i
                        string_data = string_data..our_body_table["GPSTime"][i]..","..our_body_table["longitude"][i]..","..our_body_table["latitude"][i].."/"
			
			only.log('D',"pick_time:"..g_pick_time)
			only.log('D',"data_time;"..our_body_table["GPSTime"][i])
			only.log('D',"now_time:"..os.time())
                end
                if i == #our_body_table["GPSTime"] then
			--将字符串长度拼到字符串中
                        string_data = tostring(data_len)..string_data
                       
			only.log('D',"pick_time:"..g_pick_time)
			only.log('D',"string_data:"..string_data)
			only.log('D',"data_len = "..data_len)    
                        if data_len > 0  then 
                                --遍历area_bl找属于哪个城市  
                                for k,v in pairs(area_bl) do
                                        if our_body_table["longitude"][log_lat] >= area_bl[k]['B1'] and our_body_table["longitude"][log_lat] <= area_bl[k]['B2'] then
	                                        if our_body_table["latitude"][log_lat] >= area_bl[k]['L1'] and our_body_table["latitude"][log_lat] <= area_bl[k]['L2'] then
                                                        string_child = string_child .. k
							string_data = tostring(distance) .. string_data
                                                        local ok = supex.diffuse(string_child,string_data, 100000, 3)
                                                                if not ok then
                                                                        only.log("E", "forward msmq msg failed!")
                                                                end 
                                                end 
                                        end 
                                end 

                        end 
                end
               
        end


--[[	local data_len = tostring(#our_body_table["GPSTime"])
	local string_data = data_len .. ":" .. our_body_table["IMEI"].."/"
	local string_child = "/" 
	local area_bl = cfg['area_bl']
	


	--遍历area_bl找属于哪个城市
	for k,v in pairs(area_bl) do
		if our_body_table["longitude"][1] >= area_bl[k]['B1'] and our_body_table["longitude"][1] <= area_bl[k]['B2'] then
			if our_body_table["latitude"][1] >= area_bl[k]['L1'] and our_body_table["latitude"][1] <= area_bl[k]['L2'] then
				string_child = string_child .. k
				--将表中的数据拼成字符串
				for i=1,#our_body_table["GPSTime"] do
					string_data = string_data..our_body_table["GPSTime"][i]..","..our_body_table["longitude"][i]..","..our_body_table["latitude"][i].."/"
				end
				local ok = supex.diffuse(string_child,string_data, 100000, 3)
					if not ok then
						only.log("E", "forward msmq msg failed!")
					end
			end
		end
	end
]]--	
--[[
	if our_body_table and our_body_table["APPLY"] and our_body_table["TIME"] then
		local mode = our_body_table["MODE"]
		local mode_list = {
			["push"] = 1,	--for lua VM
			["pull"] = 2,	--for lua VM

			["insert"] = 3,	--for task list
			["update"] = 4,	--for task list
			["select"] = 5,	--for task list
		}
		print("---------------")
		print(mode, mode_list[ mode ])

		if mode_list[ mode ] then
		print("+++++++++++++++++")
			local ok = supex.diffuse("/" .. our_body_table["APPLY"], our_body_data, our_body_table["TIME"], mode_list[ mode ])
			if not ok then
				only.log("E", "forward msmq msg failed!")
			end
		end
	end
	]]--
end

