-- 模块：路况
-- 功能：方向角的触发
-- 触发逻辑： 
--   1.对收到的方向角包进行两两比较，有两个方向角大于45度
--     if 最后恢复到10度以内，说明道路变化完毕，触发前方路况
--     else 则记录下来，触发路况播报，并且里程清零
--   2.不满足1的条件下，里程达到1公里触发一次路况播报,未达到1公里但时间达到3分钟，触发一次播报
--   3.道路类型是高速公路和快速路的不触发

local luakv_api 	= require('luakv_pool_api')
local redis_api 	= require('redis_pool_api')
local APP_CONFIG_LIST	= require('CONFIG_LIST')
local APP_POOL 		= require('pool')
local supex 		= require('supex')
local only		= require('only')
local cachekv 		= require('cachekv')
local cutils            = require("cutils")

module('road_traffic_handle', package.seeall)

-->>开机初始化数据
function init()
	local accountID		= supex.get_our_body_table()["accountID"]
	redis_api.cmd('owner', accountID, 'set', accountID .. ':trafficMileage', 0)
	redis_api.cmd('owner', accountID, 'set', accountID .. ':trafficTime', 0)
	luakv_api.cmd('owner', accountID, 'set', accountID .. 'cnt', 0)
end

local function direction_sub(dir1, dir2)
        local angle = math.abs(dir1 - dir2)
        return (angle <= 180) and angle or (360 - angle)
end
-->> 计算方向角
local function direction_judgment (accountID, direction, all)

	local ok,cnt = luakv_api.cmd('owner', accountID, 'get', accountID .. 'cnt', cnt)
	if not cnt then
		cnt = 0
	end
	for i=1, (all - 1) do
		local angle = direction_sub(direction[i], direction[i+1])
		if angle > 45 then
			cnt = cnt + 1
			luakv_api.cmd('owner', accountID, 'set', accountID .. 'cnt', cnt)
		end
	end
	if tonumber(cnt) >= 2 then
		if direction_sub(direction[all], direction[1]) <= 10 then
			--控制变量清零、里程清零
			luakv_api.cmd('owner', accountID, 'set', accountID .. 'cnt', 0)
			redis_api.cmd('owner', accountID, 'set', accountID .. ':trafficMileage', 0)
			--触发路况播报
			return true
		else 
			return false
		end	
	end

end

-->> 计算里程
local function mileage_count(accountID, longitude, latitude, GPSTime, all)

	local add_mileage = 0
	local add_time = 0
	add_mileage = cutils.gps_distance(longitude[1], latitude[1], longitude[all], latitude[all])	
	add_time = math.abs(GPSTime[1] - GPSTime[all] )
	redis_api.cmd('owner', accountID, 'incrbyfloat', accountID .. ':trafficMileage', add_mileage)
	redis_api.cmd('owner', accountID, 'incrbyfloat', accountID .. ':trafficTime', add_time)
						
	local ok, mileage = redis_api.cmd('owner', accountID, 'get', accountID .. ':trafficMileage')	
	if (not ok) or (not mileage) then
		return false
	end

	local ok, time = redis_api.cmd('owner', accountID, 'get', accountID .. ':trafficTime')	
	if (not ok) or (not time) then
		return false
	end
	only.log('D',"mileage:" .. mileage)
	only.log('D',"time:" .. time)
	if math.modf(mileage/1000) == 1 then
		redis_api.cmd('owner', accountID, 'set', accountID .. ':trafficMileage', 0)
		redis_api.cmd('owner', accountID, 'set', accountID .. ':trafficTime', 0)
		--触发路况播报
		return true
	elseif math.modf(mileage/1000)  ~= 1 and tonumber(time) >= 180 then
		redis_api.cmd('owner', accountID, 'set', accountID .. ':trafficMileage', 0)
		redis_api.cmd('owner', accountID, 'set', accountID .. ':trafficTime', 0)
		--触发路况播报
		return true
	else
		return false
	end

end

function handle()
	local direction		= supex.get_our_body_table()["direction"] or -1
	local accountID		= supex.get_our_body_table()["accountID"]
	local longitude 	= supex.get_our_body_table()["longitude"]
	local latitude 		= supex.get_our_body_table()["latitude"]
	local GPSTime 		= supex.get_our_body_table()["GPSTime"]
	
	local all		= #direction
	if all < 2 then 
		return false
	end
	-->> check speed
	local speed		= supex.get_our_body_table()["speed"][1]
	if not speed or tonumber(speed) == 0 then
		return false
	end
	
	-->> 道路类型为高速或者快速路不触发
	local res = APP_POOL["point_match_road_result"]
	if not res then
		only.log('D',"road result is nil")
		return false
	end
	
	local rt		= tonumber(res['RT'])
	if rt == 0 or rt == 10 then
		return false
	end
	
	-->> 判断触发路况条件是否满足
	if direction_judgment(accountID, direction, all) then
		return true
	else
		if mileage_count(accountID, longitude, latitude, GPSTime, all) then
			return true
		else
			return false
		end
	end
end
