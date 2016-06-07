--版权声明：暂无
--文件名称：AdCube_get.lua
--创建者：王盼盼
--创建日期：2015/12/01
--文件描述：从redis中查询得到所需的content , typ 和 aid

local only       =   require('only')
local supex      =   require('supex')
local redis_api  =   require('redis_pool_api')
local cjson      =   require('cjson')
local socket     =   require('socket')
local safe       =   require('safe')
local http_api   =   require('http_short_api')
local utils      =   require('utils')
local scan      =   require('scan')

module('adcube_get', package.seeall)

local IFQueryDistrictInfo= link.OWN_DIED.http.IFQueryDistrictInfo
IFQueryDistrictInfo.path = "mapapi/v2/IFQueryDistrictInfo"
local dex = 0

local function get_citycode(Cid,lat,lng,appKey)
        local tab_info = {}
        tab_info["longitude"] = lng
        tab_info["latitude"]  = lat 
        tab_info["appKey"]    = appKey 
        local ok_sign         =utils.gen_sign(tab_info)
        local body = string.format('sign=%s&appKey=%s&longitude=%s&latitude=%s',ok_sign,appKey,lng,lat)
        local post_body = utils.post_data(IFQueryDistrictInfo.path,IFQueryDistrictInfo, body)

        local ret_body = http_api.http(IFQueryDistrictInfo, post_body, true)

        if not ret_body then
            only.log("E", Cid .. "http request do failed!\n")
            local afp = supex.rgs(200)
            local string = '{"ERRORCODE":"ME25001", "RESULT":"http request  do failed!"}' .. '\n'
            supex.say(afp, string)
            return supex.over(afp,"application/json")
        end

        only.log("W","********http request return is :%s",ret_body)
        local body = string.match(ret_body, '{.*}')
        only.log("D", Cid .. "body = " .. body)
	local ret_string = body
        if body then
            ok, jo = pcall(cjson.decode, body)
            if ok and jo then
                if jo["ERRORCODE"]  == "ME24902" then
                    only.log("E", Cid .. "http body do failed!\n")
                    local afp = supex.rgs(200)
                    local string = '{"ERRORCODE":"ME25002", "RESULT":"http body do failed!"}' .. '\n'
                    supex.say(afp, string)
                    return supex.over(afp,"application/json")
                elseif jo["ERRORCODE"] ~= "0" then
                    only.log("E", "ret: %s",ret_string)
                    local afp = supex.rgs(200)
                    --local string = '{"ERRORCODE":"ME25002", "RESULT":"http body do failed!"}' .. '\n'
                    local string = ret_string
                    supex.say(afp, string)
                    return supex.over(afp,"application/json")
                end
            else
                only.log("E", Cid .. "http body do  failed!\n")
                local afp = supex.rgs(200)
                local string = '{"ERRORCODE":"ME25002", "RESULT":"http body do failed!"}' .. '\n'
                supex.say(afp, string)
                return supex.over(afp,"application/json")
            end
        else
            only.log("E", Cid .. "http body do  failed!\n")
            local afp = supex.rgs(200)
            local string = '{"ERRORCODE":"ME25002", "RESULT":"http body do failed!"}' .. '\n'
            supex.say(afp, string)
            return supex.over(afp,"application/json")
        end

        local citycode_info = jo['RESULT']


        citycode = citycode_info['cityCode']
        only.log("D", "citycode = " .. citycode)
        return citycode 
end


function AdCube_get(Cid, lat, lng, typ, appKey, sign, speed, dir, time)
    if not Cid or not lat or not lng or not typ or not appKey or not sign or not speed or not dir or not time then
        only.log("E", "incorrect paramete!\n")
        local afp = supex.rgs(200)
        local string = '{"ERRORCODE":"ME25003", "RESULT":"incorrect paramete!"}' .. '\n'
        supex.say(afp, string)
        return supex.over(afp,"application/json")
    end

    local id_content

    --check appKey and sign
    local info ={} 
    info['cid'] = Cid
    info['lat'] = lat
    info['lng'] = lng
    info['typ'] = typ
    info['appKey'] = appKey
    info['speed'] = speed
    info['dir'] = dir
    info['time'] = time
    info['sign'] = sign
    local ret = safe.sign_check(info)
    if not ret then
        only.log("E", Cid .. "appKey or sign incorrect!\n")
        local afp = supex.rgs(200)
        local string = '{"ERRORCODE":"ME25004", "RESULT":"appKey or sign incorrect!"}' .. '\n'
        supex.say(afp, string)
        return supex.over(afp,"application/json")
    else
    
        --将gps转换成相应citycode
        math.randomseed(os.time())

        --根据GPS，获得aid
        local ok, id_info = redis_api.cmd("private1", '', "SMEMBERS", "GPS")
        if not ok or not id_info then
            only.log("E", Cid .. " redis id_info do failed!\n")
            local afp = supex.rgs(200)
            local string = '{"ERRORCODE":"ME25005", "RESULT":"other errors!"}' .. '\n'
            supex.say(afp, string)
            return supex.over(afp,"application/json")
        end

        local a = 1
        local content_array1 = {}

        local lat = tonumber(lat)
        local lng = tonumber(lng)
        --根据aid查询相应广告信息
        for i=1, #id_info do
            local ok, value_tab = redis_api.cmd("private1", '', "HGETALL", id_info[i])
            if not ok or not value_tab  then
                only.log("E", Cid .. " redis value_tab do failed!\n")
                local afp = supex.rgs(200)
                local string = '{"ERRORCODE":"ME25005", "error":"other errors!"}' .. '\n'
                supex.say(afp, string)
                return supex.over(afp,"application/json")
            end

            local X0 = tonumber(value_tab['X0'])
            local X1 = tonumber(value_tab['X1'])
            local Y0 = tonumber(value_tab['Y0'])
            local Y1 = tonumber(value_tab['Y1'])
            local Content = value_tab['Content']
            local ctype = value_tab['Typ']
            local Url = value_tab['Url']
            local Adtime = value_tab['Adtime']
            if (Y1<=lng) and (lng<=Y0) and (X0<=lat) and (lat<=X1) then
                if typ == ctype then
                    local t = os.date('%X', os.time())
                    local h,m,s = string.match(t,"(%d+):(%d+):(%d+)")
                    time = h * 3600 + m * 60 + s
                    local str1 = utils.str_split(Adtime, '|')
                    for s=1,#str1 do
                        local str2 = utils.str_split(str1[s], '-')
                        local h1,m1 = string.match(str2[1],"(%d+):(%d+)")
                        local h2,m2 = string.match(str2[2],"(%d+):(%d+)")
                        local time1 = h1 * 3600 + m1 * 60
                        local time2 = h2 * 3600 + m2 * 60
                        local time = tonumber(time)
                        local time1 = tonumber(time1)
                        local time2 = tonumber(time2)
                        if (time1 <= time) and (time <= time2) then
                            local value = {id = id_info[i], content = Content}
                            content_array1[a] = value
                            a = a + 1
                        end
                    end
                end
            end
            local ret = math.random(1,#content_array1)
            id_content = content_array1[ret]
        --    only.log("D","id_content= "..tostring(id_content))
        end
        local citycode = "nil" 
        --根据citycode查询aid
        if not id_content then
             
        
            only.log("D","appKey="..tostring(appKey))
            citycode = get_citycode(Cid,lat,lng,appKey)
            only.log("D","citycode = "..tostring(citycode))
            local ok, id_info = redis_api.cmd("private1", '', "SMEMBERS", citycode)
            if not ok or not id_info then
                only.log("E", Cid .. " redis id_info do failed!\n")
                local afp = supex.rgs(200)
                local string = '{"ERRORCODE":"ME25005", "error":"other errors!"}' .. '\n'
                supex.say(afp, string)
                return supex.over(afp,"application/json")
            end

            local content_array2 = {}
            local j = 1

            --根据aid查询相应广告信息
            for i=1, #id_info do
                only.log("D","#id_info="..tostring(#id_info).."id_info[i]="..tostring(id_info[i]))
                local ok, value_tab = redis_api.cmd("private1", '', "HGETALL", id_info[i])
                if not ok or not value_tab  then
                    only.log("E", Cid .. " redis value_tab do failed!\n")
                    local afp = supex.rgs(200)
                    local string = '{"ERRORCODE":"ME25005", "RESULT":"other errors!"}' .. '\n'
                    supex.say(afp, string)
                    return supex.over(afp,"application/json")
                end
                only.log("D","%s",scan.dump(value_tab))
                local Content = value_tab['Content']
                local ctype = value_tab['Typ']
                local Url = value_tab['Url']
                local Adtime = value_tab['Adtime']
             --   only.log("D","ccccaid ="..tostring(id_info[i]))
                if typ == ctype then
                    local t = os.date('%X', os.time())
                    local h,m,s = string.match(t,"(%d+):(%d+):(%d+)")
                    time = h * 3600 + m * 60 + s
                    local str1 = utils.str_split(Adtime, '|')
                 --   print("str1 = " .. #str1)
                   --         only.log("D","ddddaid ="..tostring(id_info[i]))
                    for s=1,#str1 do
                        local str2 = utils.str_split(str1[s], '-')
                   --     print("str2[1] = " .. str2[1])
                 --       print("str2[2] = " .. str2[2])
                        local h1,m1 = string.match(str2[1],"(%d+):(%d+)")
                        local h2,m2 = string.match(str2[2],"(%d+):(%d+)")
                        local time1 = h1 * 3600 + m1 * 60
                        local time2 = h2 * 3600 + m2 * 60
                        local time = tonumber(time)
                        local time1 = tonumber(time1)
                        local time2 = tonumber(time2)
                        if (time1 <= time) and (time <= time2) then
               --             only.log("D","aid ="..tostring(id_info[i]))
                            local value = {id = id_info[i], content = Content}
             --               only.log("D",scan.dump(value))

                            content_array2[j] = value
           --                 only.log("D",scan.dump(content_array2[j]))
                            j = j + 1
                        end
                    end
                end
            end
            local ret = math.random(1,#content_array2)
            id_content = content_array2[ret]
         --   only.log("D",scan.dump(id_content))
        end

        if not id_content then
            only.log("E", Cid .. " content does not exist!\n")
            local afp = supex.rgs(200)
            local string = '{"ERRORCODE":"ME25006", "RESULT":"content does not exist!"}' .. '\n'
            supex.say(afp, string)
            return supex.over(afp,"application/json")
        end

        local ok, ret = pcall(cjson.encode, id_content)
        if not ok or not ret then
            only.log("E", Cid .. " redis ret do failed!\n")
            local afp = supex.rgs(200)
            local string = '{"ERRORCODE":"ME25005", "RESULT":"other errors!"}' .. '\n'
            supex.say(afp, string)
            return supex.over(afp,"application/json")
        end      

        local afp = supex.rgs(200)
        only.log("D","get total"..dex)
        --local strs = string.format('{"aid":"%s", "content":"%s"}', id_content['id'], id_content['content']) 
        local str  = tostring(id_content['id'])
        local strs = string.gsub(str , "A_", "")
        only.log("D","strs="..tostring(strs))
        local strs1 = tostring(id_content['content']) 
        local strs2 = tonumber(typ)
        --local string = string.format( '{"result":"ok","citycode":"%s", "aid":"%s","typ":"%d", "content":%s}', citycode,strs,strs2,strs1) .. '\n' 
        local tmp_str = utils.random_string(10)
	local time_str = os.time()
	local mid = time_str .. tmp_str
	local ok = redis_api.cmd("private1", "", "HINCRBY","mid:" .. strs .. mid,"cbtimes","0")
        if not ok  then
            only.log("E","hset mid redis do failed!\n")
        end

        local string = string.format( '{"ERRORCODE":"0","RESULT":{"aid":"%s","mid:%s":"%s","typ":"%d","content":%s}}', strs, strs, mid,strs2,strs1) .. '\n' 
        supex.say(afp, string)
        dex = dex +1 
        only.log("D","get adverstring success"..strs) 
        return supex.over(afp,"application/json")

    end
end

function string:split(sep)
	local sep, fields = sep or "\t", {}
	local pattern = string.format("([^%s]+)", sep)
	self:gsub(pattern, function(c) fields[#fields+1] = c end)
	return fields
end

function handle()
	only.log("D","get interface start ...")
	local head = supex.get_our_head()
	local result = string.split(head, '\r\n')
	local ret = {}

	local ret_k,ret_v
	for k, v in ipairs(result) do
		ret_k,ret_v = string.match(v, '(%a+):%s*(.+)')
		if ret_v then
			ret[ret_k]=ret_v
		end
        end
	local data = supex.get_our_body_table()
	local lat = data["lat"]
	local lng = data["lng"]
	local typ = data["typ"]
	local dir = data["dir"]
	local speed = data["speed"]
	local cid = ret['cid'] or data["cid"]
	local appKey = ret['appKey'] or data["appKey"]
	local sign = ret['sign'] or data["sign"]
	local time = ret['time'] or data["time"]
    only.log("D","%s","cid ="..tostring(cid).."lat ="..tostring(lat).."lng ="..tostring(lng).."typ ="..tostring(typ).."appKey ="..tostring(appKey).."sign ="..tostring(sign).."speed ="..tostring(speed).."dir ="..tostring(dir).."time ="..tostring(time))
    AdCube_get(cid, lat, lng, typ, appKey, sign, speed, dir, time)
end

