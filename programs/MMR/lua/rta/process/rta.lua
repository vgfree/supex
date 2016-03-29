local redis_api = require('redis_pool_api')

module('rta',package.seeall)

local accelerationKey  = ''
local directionKey = ''

local accel_de_left_field = { 'acceleration:left:-1','acceleration:left:-2',
'acceleration:left:-3','acceleration:left:-4','acceleration:left:-5',
'acceleration:left:-6','acceleration:left:-7','acceleration:left:-8',
'acceleration:left:-9','acceleration:left:-10',
'acceleration:left:-11-20','acceleration:left:-:abnormal'}
local accel_de_right_field = { 'acceleration:right:-1','acceleration:right:-2',
'acceleration:right:-3','acceleration:right:-4','acceleration:right:-5',
'acceleration:right:-6','acceleration:right:-7','acceleration:right:-8',
'acceleration:right:-9','acceleration:right:-10',
'acceleration:right:-11-20','acceleration:right:-:abnormal'}
local accel_ac_left_field = { 'acceleration:left:1','acceleration:left:2',
'acceleration:left:3','acceleration:left:4','acceleration:left:5',
'acceleration:left:6','acceleration:left:7','acceleration:left:8',
'acceleration:left:9','acceleration:left:10',
'acceleration:left:11+20','acceleration:left:+:abnormal'}
local accel_ac_right_field = { 'acceleration:right:1','acceleration:right:2',
'acceleration:right:3','acceleration:right:4','acceleration:right:5',
'acceleration:right:6','acceleration:right:7','acceleration:right:8',
'acceleration:right:9','acceleration:right:10',
'acceleration:right:11+20','acceleration:right:+:abnormal'}
local accel_ac_str_line  = {'acceleration:1','acceleration:2','acceleration:3',
'acceleration:4','acceleration:5','acceleration:6','acceleration:7',
'acceleration:8','acceleration:9','acceleration:10',
'acceleration:11+20','acceleration:+:abnormal'}
local accel_de_str_line  = {'acceleration:-1','acceleration:-2','acceleration:-3',
'acceleration:-4','acceleration:-5','acceleration:-6','acceleration:-7',
'acceleration:-8','acceleration:-9','acceleration:-10',
'acceleration:-11-20','acceleration:-:abnormal'}
local accel_zero = {'acceleration:0'}

local direction_diff = {'turn0010','turn1120','turn2130','turn3140','turn4150','turn5160','turn6170','turn7180','turn8190'}

local accel = {}
local dirCount = {}

local function direction_sub ( dir1, dir2 )
	local angle = math.abs( dir1 - dir2 )
	return (angle <= 180) and angle or (360 - angle)
end

-- direction "str" 表示为 直线,"left"表示为向左,"right"表示为向右
local function handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,direction)

	local acc_value_int 
	local field

	if this_point_time == last_point_time  then
		--这是异常
		return 
	end

        if this_point_time - last_point_time >2 then
                return
        end

	if this_point_speed == last_point_speed then
		if accel[accel_zero[1]] then
			accel[accel_zero[1]] = accel[accel_zero[1]] +1
		else
		    accel[accel_zero[1]] = 1
		end
		--only.log('D','accel_zero 00 ::'..accel[accel_zero[1]])

		return
	end

	local acceleration_value = (this_point_speed - last_point_speed) *1000/3600/(this_point_time-last_point_time)
	only.log ('D','acceleration_value:'..acceleration_value)

	if acceleration_value == 0 then
		return
	else
		if acceleration_value >0 then
			if acceleration_value <= 10 then
				if acceleration_value%1 >0.5 then
					acc_value_int = math.ceil(acceleration_value)
				else
					acc_value_int = math.floor(acceleration_value)
				end
				field = math.abs(acc_value_int)
			elseif acceleration_value >10 and acceleration_value <= 20 then
				field = 11
			elseif acceleration_value >20 then
				field = 12
			end
		else
			if acceleration_value >=-10 then
				if acceleration_value%1 >0.5 then
					acc_value_int =  math.ceil(acceleration_value)
				else
					acc_value_int =  math.floor(acceleration_value)
				end
				field = math.abs(acc_value_int)
			elseif acceleration_value <-10 and acceleration_value >= -20 then
				field = 11
			elseif acceleration_value < -20 then
				field = 12
			end
		end
	end
	only.log ('D',"acc field:"..field)

	if field == 0 then
		if accel[accel_zero[1]] then
			accel[accel_zero[1]] = accel[accel_zero[1]] +1
		else
		    accel[accel_zero[1]] = 1
		end
		--only.log('D','accel_zero  field 00 '..accel[accel_zero[1]])
		return 
	end

	if direction == 'str' then
		only.log ('D','****str')
		if acceleration_value >0 then
			local accel_str = accel_ac_str_line[field]
			if accel[accel_ac_str_line[field]] then				
				accel[accel_ac_str_line[field]] = accel[accel_ac_str_line[field]] +1
			else
				accel[accel_ac_str_line[field]] = 1
			end
		else
			if accel[accel_de_str_line[field]] then				
				accel[accel_de_str_line[field]] = accel[accel_de_str_line[field]] +1
			else
				accel[accel_de_str_line[field]] = 1
			end
		end
	elseif direction == 'left' then
		--only.log ('D','****left')
		if acceleration_value >0 then
			--only.log ('D','****left'..accel_ac_left_field[field])
			if accel[accel_ac_left_field[field]] then				
				accel[accel_ac_left_field[field]] = accel[accel_ac_left_field[field]] +1
			else
				accel[accel_ac_left_field[field]] = 1
			end
		else
			--only.log ('D','****left'..accel_de_left_field[field])
			if accel[accel_de_left_field[field]] then				
				accel[accel_de_left_field[field]] = accel[accel_de_left_field[field]] +1
			else
				accel[accel_de_left_field[field]] = 1
			end
		end
	elseif direction == 'right' then
		if acceleration_value >0 then
			--only.log ('D','****right'..accel_ac_right_field[field])
			if accel[accel_ac_right_field[field]] then				
				accel[accel_ac_right_field[field]] = accel[accel_ac_right_field[field]] +1
			else
				accel[accel_ac_right_field[field]] = 1
			end
		else
			--only.log ('D','****right'..accel_de_right_field[field])
			if accel[accel_de_right_field[field]] then				
				accel[accel_de_right_field[field]] = accel[accel_de_right_field[field]] +1
			else
				accel[accel_de_right_field[field]] = 1
			end
		end
	end
end

local function handle_direction(last_point_time,last_point_direction,this_point_time,this_point_direction)

	if this_point_time == last_point_time then
		return
	end
	if this_point_time - last_point_time > 2 then
		return
	end
	if this_point_direction == -1 or last_point_direction == -1 then
		return
	end
	if this_point_direction == last_point_direction then
		return
	end

	local dir_v = direction_sub(last_point_direction,this_point_direction)
	if dir_v >= 90 then
		return
	end
	local dir_order = math.ceil(dir_v/10)

	if dirCount[direction_diff[dir_order]] then
		dirCount[direction_diff[dir_order]]	= dirCount[direction_diff[dir_order]] +1
	else
	    dirCount[direction_diff[dir_order]]   = 1
	end
end

local function handle_acceleration(last_point_time,last_point_direction,last_point_speed,this_point_time,this_point_direction,this_point_speed)
	only.log ('D','last_point_time:'..last_point_time)
	only.log ('D','this_point_time:'..this_point_time)
	only.log ('D','last_point_direction:'..last_point_direction)
	only.log ('D','this_point_direction:'..this_point_direction)
	only.log ('D','last_point_speed:'..last_point_speed)
	only.log ('D','this_point_speed:'..this_point_speed)

	-- 方向在这个范围内不算转弯
	local  direction_range_value = 5 
	--local  direction_falge_value = 180

	if last_point_direction == -1 and this_point_direction == -1 then
		--加速度为0
		--only.log ('D',0)
		--only.log ('D','two direction -1')
		return 
	elseif last_point_direction == -1 and this_point_direction ~= -1 then
		if this_point_speed == 0 then 
			-- 加速度为0
			--only.log ('D',0)
			--only.log ('D','last_point_direction -1,this_point_speed 0 ')
			return 
		else
			--直线加速度
			only.log ('D','this ac over 0')
			handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
		end
	elseif last_point_direction ~= -1 and this_point_direction == -1 then
		if last_point_speed == 0 then
			--加速度为0
			--only.log ('D',0)
			--only.log ('D','last_point_speed 0,this_point_speed -1')
			return 
		else
			--直线加速
			--only.log ('D','this dc less than ')
			handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
		end
	elseif last_point_direction ~= -1 and this_point_direction ~= -1 then
		--only.log ('D','two direction not -1')
		if last_point_speed == 0 or this_point_speed ==0 then
			--直线加速
			--only.log ('D','two speed or 0')
			handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
		elseif last_point_speed  ~= 0 and this_point_speed ~= 0 then
			--就要判断方向
			local direction_diff_value 

			if last_point_direction >= 0 and last_point_direction <= 89 then 
				--only.log ('D','[0-89]')
				if this_point_direction >250 then
					last_point_direction = last_point_direction + 360 
					--only.log ('D',last_point_direction)
					direction_diff_value = last_point_direction - this_point_direction 
					if direction_diff_value <= direction_range_value then 
						--直线加速
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
					else
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'left')
					end
				else
					if this_point_direction <= last_point_direction then
						direction_diff_value = last_point_direction - this_point_direction
						if direction_diff_value <= direction_range_value then
							--直线加速
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
						else
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'left')
						end
					else
						direction_diff_value = this_point_direction - last_point_direction
						if direction_diff_value <= direction_range_value then
							--直线加速
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
						else
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'right')
						end
					end
				end
			elseif last_point_direction >= 90 and last_point_direction <= 269 then
				--only.log ('D','[90-269]')
				if this_point_direction <= last_point_direction then
					direction_diff_value = last_point_direction - this_point_direction 
					if direction_diff_value <= direction_range_value then
						--直线加速
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
					else
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'left')
					end
				else
					direction_diff_value = this_point_direction - last_point_direction 
					if direction_diff_value <= direction_range_value then
						--直线加速
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
					else
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'right')
					end
				end
			elseif last_point_direction >= 270 and last_point_direction <= 359 then 
				--only.log ('D','[270-359]')
				if this_point_direction <= 100 then
					this_point_direction = this_point_direction + 360
					direction_diff_value = this_point_direction - last_point_direction
					if direction_diff_value <= direction_range_value then
						--直线加速
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
					else
						handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'right')
					end
				else
					if this_point_direction <= last_point_direction then
						direction_diff_value = last_point_direction - this_point_direction
						if direction_diff_value <= direction_range_value then
							--直线加速
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
						else
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'left')
						end
					else
						direction_diff_value = this_point_direction - last_point_direction
						if direction_diff_value <= direction_range_value then
							--直线加速
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'str')
						else
							handle_do(last_point_time,this_point_time,last_point_speed,this_point_speed,'right')
						end
					end
				end
			end
		end
	end
end


local function SetFinish(data_pack,accelerationKey,directionKey)
	local set_finish_key = 'FinshedSet'
	local set_process_key = 'unFinshedSet'

	local set_finish_dir_key = 'FinshedDirSet'
	local set_process_dir_key = 'unFinshedDirSet'

	if data_pack['lastTokenCode'] then

		local lastAccelerationKey = data_pack['IMEI']..':'..data_pack['lastTokenCode']..':'..'RTA'
		local lastDirectionKey = data_pack['IMEI']..':'..data_pack['lastTokenCode']..':'..'DIR'

		only.log('D','lastAccelerationKey:'..lastAccelerationKey)
		only.log('D','lastDirectionKey:'..lastDirectionKey)

		local ok_sadd,sadd_value = redis_api.cmd('RTA',data_pack['IMEI'] or '','sadd',set_finish_key,lastAccelerationKey)

		if not ok_sadd then
			only.log('D','sadd FinshedSet is error')
		end

		local ok_sadd,sadd_value = redis_api.cmd('RTA',data_pack['IMEI'] or '','sadd',set_finish_dir_key,lastDirectionKey)
		if not ok_sadd then
			only.log('D','sadd FinshedDirSet is error')
		end


		local ok_zrem,zrem_value = redis_api.cmd('RTA',data_pack['IMEI'] or '','zrem',set_process_key,lastAccelerationKey)

		if not ok_zrem then
			only.log('D','zrem unFinshedSet is error')
		end

		local ok_zrem,zrem_value = redis_api.cmd('RTA',data_pack['IMEI'] or '','zrem',set_process_dir_key,lastDirectionKey)
		if not ok_zrem then
			only.log('D','zrem unFinshedDirSet is error')
		end


	else
		--程序运行接受首包,没有"lastTokenCode"
		only.log('D','lastTokenCode is not exists')
	end
end

local function SetProcess(data_pack,accelerationKey,directionKey)
	--this is RTA SortedSet
	local set_process_key = 'unFinshedSet'
	local ok_zadd,zadd_value = redis_api.cmd('RTA',data_pack['IMEI'] or '','zadd',set_process_key,data_pack['endTime'],accelerationKey)
	if not ok_zadd then
		only.log('D','zadd unFinshedSet is error')
	end
    --this is DIR SortedSet
	local set_process_dir_key = 'unFinshedDirSet' 
	local ok_zadd,zadd_value = redis_api.cmd('RTA',data_pack['IMEI'] or '','zadd',set_process_dir_key,data_pack['endTime'],directionKey)
	if not ok_zadd then
		only.log('D','zadd unDirFinshedSet is error')
	end
end

local function accelerationFun(data_pack)
	local IMEI = data_pack['IMEI']
	local tokenCode = data_pack['tokenCode']
	accelerationKey = IMEI..':'..tokenCode..':'..'RTA'
	directionKey  = IMEI..':'..tokenCode..':'..'DIR'


	only.log ('D',accelerationKey)
	only.log ('D',directionKey)

	if data_pack['isFirst'] then
		--lastTokenCode 已经完成
		SetFinish(data_pack,accelerationKey,directionKey)
	else
		--lastTokenCode 没有完成
		SetProcess(data_pack,accelerationKey,directionKey)
	end

	local this_point_number = #(data_pack['points'])	

	for this_point_order = 1,this_point_number do
		if this_point_order == 1 then
			if data_pack['lastPoint'] ~= nil then
				local last_point_time = data_pack['lastPoint']['GPSTime']
				local last_point_direction = data_pack['lastPoint']['direction']
				local last_point_speed = data_pack['lastPoint']['speed']
				local this_point_time = data_pack['points'][this_point_order]['GPSTime']
				local this_point_direction = data_pack['points'][this_point_order]['direction']
				local this_point_speed = data_pack['points'][this_point_order]['speed']
				handle_acceleration(last_point_time,last_point_direction,last_point_speed,this_point_time,this_point_direction,this_point_speed)
				handle_direction(last_point_time,last_point_direction,this_point_time,this_point_direction)
			end
		else
				local last_point_time = data_pack['points'][this_point_order-1]['GPSTime']
				local last_point_direction = data_pack['points'][this_point_order-1]['direction']
				local last_point_speed = data_pack['points'][this_point_order-1]['speed']
				local this_point_time = data_pack['points'][this_point_order]['GPSTime']
				local this_point_direction = data_pack['points'][this_point_order]['direction']
				local this_point_speed = data_pack['points'][this_point_order]['speed']
				handle_acceleration(last_point_time,last_point_direction,last_point_speed,this_point_time,this_point_direction,this_point_speed)
				handle_direction(last_point_time,last_point_direction,this_point_time,this_point_direction)
		end
	end
    --处理RTA
	if next(accel) ~= nil then
        	local accel_key = {}
        	local accel_order = 0
        	for key in pairs(accel) do 
        		accel_order =accel_order +1
        		accel_key[accel_order] = key
        	end
        	ok_hmget,hmget_value = redis_api.cmd('RTA',IMEI or '','hmget',accelerationKey,unpack(accel_key))
        	if not ok_hmget then
        		only.log('D','accel hmget error')
				return
			end
		--需要初始化
		if next(hmget_value) == nil then

			local accel_de_left_field_array = {}
			local de_left_order = 0
			for i ,value in ipairs(accel_de_left_field)  do
				de_left_order = de_left_order + 1
				accel_de_left_field_array[de_left_order] = value
				de_left_order = de_left_order + 1
				if accel[value] then
					accel_de_left_field_array[de_left_order] = accel[value]
				else
					accel_de_left_field_array[de_left_order] = 0 
				end
			end

			local accel_de_right_field_array = {}
			local de_right_order = 0
			for i ,value in ipairs(accel_de_right_field)  do
				de_right_order = de_right_order + 1
				accel_de_right_field_array[de_right_order] = value
				de_right_order = de_right_order + 1
				if accel[value] then
					accel_de_right_field_array[de_right_order] = accel[value]
				else
					accel_de_right_field_array[de_right_order] = 0 
				end
			end

			local accel_ac_left_field_array = {}
			local ac_left_order = 0
			for i ,value in ipairs(accel_ac_left_field)  do
				ac_left_order = ac_left_order + 1
				accel_ac_left_field_array[ac_left_order] = value
				ac_left_order = ac_left_order + 1
				if accel[value] then
					accel_ac_left_field_array[ac_left_order] = accel[value] 
				else
					accel_ac_left_field_array[ac_left_order] = 0 
				end
			end

			local accel_ac_right_field_array = {}
			local ac_right_order = 0
			for i ,value in ipairs(accel_ac_right_field)  do
				ac_right_order = ac_right_order + 1
				accel_ac_right_field_array[ac_right_order] = value
				ac_right_order = ac_right_order + 1
				if accel[value] then
					accel_ac_right_field_array[ac_right_order] = accel[value] 
				else
					accel_ac_right_field_array[ac_right_order] = 0 
				end
			end

			local accel_ac_str_line_array = {}
			local ac_str_order = 0
			for i ,value in ipairs(accel_ac_str_line) do
				ac_str_order = ac_str_order + 1
				accel_ac_str_line_array[ac_str_order] = value
				ac_str_order = ac_str_order + 1
				if accel[value] then
					accel_ac_str_line_array[ac_str_order] = accel[value] 
				else
					accel_ac_str_line_array[ac_str_order] = 0 
				end
			end

			local accel_de_str_line_array = {}
			local de_str_order = 0
			for i ,value in ipairs(accel_de_str_line) do
				de_str_order = de_str_order + 1
				accel_de_str_line_array[de_str_order] = value
				de_str_order = de_str_order + 1
				if accel[value] then
					accel_de_str_line_array[de_str_order] =  accel[value] 
				else
					accel_de_str_line_array[de_str_order] = 0 
				end
			end

			local accel_zero_array = {}
			for i , value in ipairs(accel_zero) do
				table.insert(accel_zero_array,value)
	            if accel[value] then
					table.insert(accel_zero_array,accel[value])
	            else
					table.insert(accel_zero_array,0)
				end
			end
            
			ok_zero,value_zero = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_zero_array))
			ok_de_left,value_de_left = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_de_left_field_array))
			ok_ac_left,value_ac_left = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_ac_left_field_array))
			ok_ac_right,value_ac_right = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_ac_right_field_array))
			ok_de_right,value_de_right = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_de_right_field_array))
			ok_de_str,value_de_str = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_de_str_line_array))
			ok_ac_str,value_ac_str = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_ac_str_line_array))
		end
		if next(hmget_value) ~= nil then

			for i,value in ipairs(hmget_value) do
				accel[accel_key[i]]  = accel[accel_key[i]] + value
				--only.log('D','hmget_value **::'..accel_key[i]..':%%:'..accel[accel_key[i]])
			end

			local accel_array = {}
			local accel_array_order = 0

			for key ,value in pairs(accel) do
				accel_array_order = accel_array_order + 1
				accel_array[accel_array_order] = key
				accel_array_order = accel_array_order + 1
				accel_array[accel_array_order] = value 
			end
			ok_hmset,hmset_value = redis_api.cmd('RTA',IMEI or '','hmset',accelerationKey,unpack(accel_array))
		end
	end
	--处理Dir
	if next(dirCount) ~= nil then
		local dir_key = {}
		for key in pairs(dirCount) do
			table.insert(dir_key,key)
		end
		ok_hmget,hmget_value = redis_api.cmd('RTA',IMEI or '','hmget',directionKey,unpack(dir_key))
		if not ok_hmget then
			only.log('D','dirCount hmget error')
			return
		end
		--需要初始化
		if next(hmget_value) == nil then
			local dir_field_array = {}
			for i,value in ipairs(direction_diff) do
				table.insert(dir_field_array,value)
	            if dirCount[value] then	
					table.insert(dir_field_array,dirCount[value])
	            else
					table.insert(dir_field_array,0)
				end
			end
			local ok_dir,value_dir = redis_api.cmd('RTA',IMEI or '','hmset',directionKey,unpack(dir_field_array))
		end
		--
		if next(hmget_value) ~= nil then
			for i,value in ipairs(hmget_value) do
				dirCount[dir_key[i]] = dirCount[dir_key[i]] + value
			end
			
			local dir_array = {}
			for key,value in pairs(dirCount) do
				table.insert(dir_array,key)
				table.insert(dir_array,value)
			end
			ok_hmset,hmset_value = redis_api.cmd('RTA',IMEI or '','hmset',directionKey,unpack(dir_array))
		end
	end

end


function handle(data_pack)
	only.log('D',scan.dump(data_pack))

	accel = {}
    dirCount = {}
	accelerationFun(data_pack)
end
