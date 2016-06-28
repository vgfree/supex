local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../../open/lib/?.lua;",
        "../../../open/apply/?.lua;",
        "../../../open/spxonly/?.lua;",
        "../../../open/linkup/?.lua;",
        "../../../open/public/?.lua;",
        "open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")

redis_api.init()

local function get_data(key, cmd, redis_num)
	if key == nil or cmd == nil then
		return nil
	end
	local ok_status, ok_ret = redis_api.cmd('redis' .. tostring(redis_num), '', cmd, key)
        if not (ok_status and ok_ret) then
        	only.log('E', 'Get data failed or no suitable data')
		return nil        
	end
	return ok_ret
end

local function set_data(redis, cmd, key, data)
	if key == nil or data == nil then
		return nil
	end
	
	if type(data) == 'table'then
		for tab_id = 1, #data do
			redis_api.cmd(redis, '', cmd, key, data[tab_id])
		end
	end
	
	if type(data) ~= 'table' then
		local ok_status, ok_ret = redis_api.cmd(redis, '', cmd, key, data)
		if not (ok_status and ok_ret) then
        		only.log('E', 'Set data failed or no suitable data')
			return nil        
		end
	end

	return ok_ret
end

local function creat_user(id)
	return tostring(os.time()) .. tostring(id)
end

local function creat_key()
	local current_time = os.time()
        local time = os.date("%Y%m%d%H", current_time)
        local interval = 10
        if interval >= 10 then
                time = time .. math.floor(os.date("%M", current_time)/10)
        end

        if interval < 10 then
                time = time .. math.floor(os.date("%M", current_time)/10) .. interval
                print(time)
        end

        local user_key = "ACTIVEUSER:" ..time

        return user_key, time
end

--[[
local function creat_key()
        local current_time = os.time()
        local time = os.date("%Y%m%d%H", current_time)
	local minute = os.date("%M", current_time)
        if minute < '30' then
                time = time .. tostring(0)--math.floor(os.date("%M", current_time)/10)
        end

	if minute >= '30' then
		time = time .. tostring(3)
	end

        if interval < 10 then
                time = time .. math.floor(os.date("%M", current_time)/10) .. interval
                print(time)
        end

        local user_key = "ACTIVEUSER:" ..time

        return user_key, time
end
]]--
local gps_user_data = {}
gps_user_data[1] = '1466044886|1466044901|508542171690587||460012202737137|160616024126|1234339042|418406815|65|29|0|QnsQkkBlen'
gps_user_data[2] = '1466044885|1466044901|508542171690587||460012202737137|160616024125|1234339042|418406815|65|29|0|QnsQkkBlen'
gps_user_data[3] = '1466044884|1466044901|508542171690587||460012202737137|160616024124|1234339042|418406815|65|29|0|QnsQkkBlen'
gps_user_data[4] = '1466044883|1466044901|508542171690587||460012202737137|160616024123|1234339042|418406815|65|29|0|QnsQkkBlen'
gps_user_data[5] = '1466044882|1466044901|508542171690587||460012202737137|160616024122|1234339042|418406815|65|29|0|QnsQkkBlen'

local url_user_data = {}
url_user_data[1] = '1466592600|mt=2576&nt=0&imei=944934944845253&imsi=460017544705948&mod=AM001&tokencode=mwuleN8MiA&gps=220616105001,11349.12420E,2317.65270N,179,7,-3;220616105000,11349.12450E,2317.65370N,173,10,-3;220616104959,11349.12440E,2317.65540N,177,10,-2;220616104958,11349.12460E,2317.65750N,171,10,-2;220616104957,11349.12440E,2317.65950N,178,9,-2|172.16.61.10|555'
url_user_data[2] = '1466592639|mt=2576&nt=0&imei=944934944845253&imsi=460017544705948&mod=AM001&tokencode=mwuleN8MiA&gps=220616105041,11349.20760E,2317.59460N,191,37,-5;220616105040,11349.20880E,2317.60030N,191,35,-5;220616105039,11349.21020E,2317.60570N,187,32,-4;220616105038,11349.21100E,2317.61120N,181,28,-4;220616105037,11349.21120E,2317.61630N,177,22,-4|172.16.61.10|555'
url_user_data[3] = '1466592701|mt=2576&nt=0&imei=944934944845253&imsi=460017544705948&mod=AM001&tokencode=mwuleN8MiA&gps=220616105141,11349.15610E,2317.37870N,181,1,-5;220616105140,11349.15570E,2317.37790N,181,0,-5;220616105139,11349.15570E,2317.37820N,181,3,-5;220616105138,11349.15610E,2317.37870N,181,0,-5;220616105137,11349.15620E,2317.37930N,181,5,-5|172.16.61.10|555'
url_user_data[4] = '1466592715|mt=2576&nt=0&imei=944934944845253&imsi=460017544705948&mod=AM001&tokencode=mwuleN8MiA&gps=220616105156,11349.15630E,2317.37840N,181,0,-1;220616105155,11349.15630E,2317.37840N,181,0,-1;220616105154,11349.15630E,2317.37840N,181,0,-1;220616105153,11349.15630E,2317.37840N,181,0,-1;220616105152,11349.15630E,2317.37840N,181,0,-1|172.16.61.10|555'
url_user_data[5] = '1466592635|mt=2576&nt=0&imei=944934944845253&imsi=460017544705948&mod=AM001&tokencode=mwuleN8MiA&gps=220616105036,11349.20990E,2317.62520N,172,18,-4;220616105035,11349.20970E,2317.62960N,164,13,-4;220616105034,11349.20900E,2317.63430N,147,8,-5;220616105033,11349.20850E,2317.63760N,91,3,-5;220616105032,11349.20810E,2317.63850N,91,3,-5|172.16.61.10|555'
url_user_data[6] = '1466592609|mt=2576&nt=0&imei=944934944845253&imsi=460017544705948&mod=AM001&tokencode=mwuleN8MiA&gps=220616105011,11349.13920E,2317.65430N,99,20,-4;220616105010,11349.13620E,2317.65460N,98,21,-4;220616105009,11349.13320E,2317.65500N,104,21,-3;220616105008,11349.12950E,2317.65460N,108,19,-3;220616105007,11349.12670E,2317.65450N,114,16,-3|172.16.61.10|555'


local function work()
	local key, time = creat_key()
	local user_tab = {}
	for id = 1, 50 do
		user_tab[id] = creat_user(id)
	end
	only.log("E", "the table = %s", scan.dump(user_tab))
	local redis = 'redis' .. os.time() % 2 
	set_data(redis, 'SADD', key, user_tab)
	
	for tab_id = 1, #user_tab do
		key = 'GPS:' .. user_tab[tab_id] .. ':' .. time
		set_data(redis, 'SADD', key, gps_user_data)
		key = 'URL:' .. user_tab[tab_id] .. ':' .. time
		set_data(redis, 'SADD', key, url_user_data)
	end
end

function handle()
	local t = os.time()
	while true do
    		local time = os.time()
    		if time - t >= 60 then
        		t = time
			work()
    		end
	end
end

handle = handle()

