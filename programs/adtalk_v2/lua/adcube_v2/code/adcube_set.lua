local only          = require('only')
local supex         = require('supex')
local socket        = require('socket')
local redis_api     = require('redis_pool_api')
local utils         = require('utils')
local safe          = require('safe')
module('adcube_set', package.seeall)

--[[
函数名：calculate
功  能：计算方圆diff公里的两个坐标
参  数：lng 经度 , lat 纬度 , diff 1km or 2km
返回值：两个坐标点
]]--


local function calculate(lng,lat,diff)
      lat  = tonumber(lat)
      lng  = tonumber(lng)
      diff = tonumber(diff)
      local index = diff/100
      local x0 = lat   - index 
      local x1 = lat   + index
      local y0 = lng  + index
      local y1 = lng  - index
      return x0,x1,y1,y0

end

--[[
函数名：Adcube_set
功  能：来一条广告,生成aid号,并以 城市类型 或 GPS类型 保存到redis
参  数：content           文本：广告内容 
        typ               类型：1冠名、2尾标、3纯广（皆属声音广告）   
        lng               经度 
        lat               纬度 
        diff              范围：1km  或  2km
        cburl             cburl字符串
        appKey            app标识
        sign              签名
返回值：aid               广告号 
        false             失败
]]--



local function Adcube_set(content,typ,citycode,adtime,lng,lat,diff,cburl,appKey,sign)
     
    local SET_AID = "SET_AID" 
    local GPS     = "GPS"
    if tostring(appKey) ~= "nil" and tostring(sign) ~= "nil" and tostring(content) ~= "nil" and tostring(adtime) ~= "nil" and tostring(typ) ~= "nil" and tostring(cburl) ~= "nil"  then


          if (tostring(citycode) ~="nil" and tostring(lng) =="nil" and tostring(lat) =="nil" and tostring(diff) =="nil") or (tostring(citycode) =="nil" and tostring(lng) ~="nil" and tostring(lat) ~= "nil" and tostring(diff) ~="nil") then
            
                    -- 取 aid 广告号
                    local ok,aid  = redis_api.cmd( 'private1','','incr',SET_AID)
                    if  not ok  then
                          only.log("E","incr SET_AID false ")
                          local afp = supex.rgs(200)
                          local string = '{"ERRORCODE":"ME25005","RESULT":"other errors!"}'.."\n"
                          supex.say(afp,string)
                          return supex.over(afp)
                   --       return false
                    end  
                    local caid = "A_"..aid
                    if tostring(citycode) == "nil"   then
----[[
                         local tab     = {} 
                         tab["appKey"] = appKey
                         tab["sign"]   = sign
                         tab["content"]   = content
                         tab["adtime"] = adtime
                         tab["typ"]   = typ
                         tab["lng"]   = lng
                         tab["lat"]   = lat
                         tab["cburl"] = cburl
                         tab["diff"]  = diff
                      --检查 appKey  与  sign

                         local  value  = safe.sign_check(tab)
                         if  not value  then
                                 only.log("E"," check_parameter(appKey,sign) is not correct ")
                                 local afp = supex.rgs(200)
                                 local string = '{"ERRORCODE":"ME25004","RESULT":"appKey or sign incorrect!"}'.."\n"
                                 supex.say(afp,string)
                                 return supex.over(afp)
                          end
 --         ]]--        
                    -- 集合存储 key ： GPS ,value : aid
                          local ok,gps = redis_api.cmd( 'private1','', 'sadd', GPS ,caid)
                          if not ok or not gps then
                               only.log("E","sadd GPS aid false "..aid)
                               local afp = supex.rgs(200)
                               local string = '{"ERRORCODE":"ME25005","RESULT":"other errors!"}'.."\n"
                               supex.say(afp,string)
                               return supex.over(afp)
                          end
                          
                    -- 计算坐标
                          local x0,x1,y1,y0  = calculate(lng,lat,diff)

                    -- 哈希存储  表名：aid    字段：Content,Typ,Url,X0,X1,Y1,Y0,Diff
                          local ok,gps_aid = redis_api.cmd( 'private1','', 'hmset', caid ,'Content',content,'Adtime',adtime,'Typ',typ,'Cburl',cburl,'X0',x0,'X1',x1,'Y1',y1,'Y0',y0,'Diff',diff)
                          if not ok  or not gps_aid then
                               only.log("E","hmget aid ....GPS false "..aid)
                               local ok,gps = redis_api.cmd( 'private1','', 'srem', GPS ,caid)
                               if not ok or not gps then
                                     only.log("E","srem GPS aid false "..aid)
                               end
                               local afp = supex.rgs(200)
                               local string = '{"ERRORCODE":"ME25005","RESULT":"other errors!"}'.."\n"
                               supex.say(afp,string)
                               return supex.over(afp)
                          end

                    else
----[[
                          local tab1     = {} 
                          tab1["appKey"] = appKey
                          tab1["sign"]   = sign
                          tab1["content"]   = content
                          tab1["adtime"] = adtime 
                          tab1["typ"]   = typ
                          tab1["citycode"]   = citycode
                          tab1["cburl"] = cburl
                    --检查 appKey  与  sign

                          local  value  = safe.sign_check(tab1)
                          if  not value  then
                              only.log("E"," check_parameter(appKey,sign) is not correct ")
                              local afp = supex.rgs(200)
                              local string = '{"ERRORCODE":"ME25004","RESULT":"appKey or sign incorrect!"}'.."\n"
                              supex.say(afp,string)
                              return supex.over(afp)
                          end
  --        ]]--         
                   -- 集合存储 key  : citycode,value : aid
                          citycode = tostring(citycode)
                          local tab_city = utils.str_split(citycode,'|')
                          local tab_dex  = #tab_city
                          for i=1,tab_dex do
                               local ok,city = redis_api.cmd( 'private1','' ,'sadd', tab_city[i] ,caid)
                               if not ok  or not city then
                                   only.log("E","sadd tab_city aid false "..aid)
                                   local afp = supex.rgs(200)
                                   local string = '{"ERRORCODE":"ME25005","RESULT":"other errors!"}'.."\n"
                                   supex.say(afp,string)
                                   return supex.over(afp)
                              end
                          end
                  --         哈希存储  表名：aid    字段：Content,Typ,Url
                          local ok,city_aid = redis_api.cmd( 'private1','', 'hmset', caid ,'Citycode',citycode,'Content',content,'Adtime',adtime,'Typ',typ,'Cburl',cburl)
                          if not ok  or not city_aid then
                               only.log("E","hmset aid ....citycode false "..aid)
                               for i=1,tab_dex do
                                 local ok,city = redis_api.cmd( 'private1','' ,'srem', tab_city[i] ,caid)
                                 if not ok  or not city then
                                       only.log("E","srem tab_city aid false "..aid)
                                 end
                               end  
                               local afp = supex.rgs(200)
                               local string = '{"ERRORCODE":"ME25005","RESULT":"other errors!"}'.."\n"
                               supex.say(afp,string)
                               return supex.over(afp)
                          end

                      end
                  aid = tostring(aid)
                  local afp = supex.rgs(200) 
                  local string =string.format('{"ERRORCODE":"0","RESULT":{"aid":"%s"}}'.."\n",aid)
                  supex.say(afp,string)
                  only.log("D","set advertising success  "..aid)
                  return supex.over(afp)
          else
              only.log("E","The format is not correct")
              local afp = supex.rgs(200)
              local string = '{"ERRORCODE":"ME25003","RESULT":"incorrect paramete!"}'.."\n"
              supex.say(afp,string)
              return supex.over(afp)
          end
    else
         only.log("E","The parameter is not correct")
         local afp = supex.rgs(200)
         local string = '{"ERRORCODE":"ME25003","RESULT":"incorrect paramete!"}'.."\n"
         supex.say(afp,string)
         return supex.over(afp)
    end          
end







function handle()
	    only.log("D","start set interface...")
        local data        = supex.get_our_body_table()
        local content     = data['content']
        local typ         = data['typ']
        local cburl       = data['cburl']
        local citycode    = data['citycode']
        local adtime      = data['adtime'] 
        local lng         = data['lng']
        local lat         = data['lat'] 
        local diff        = data['diff']
        local appKey      = data['appKey']
        local sign        = data['sign']
        only.log("D","content="..tostring(content).."typ="..tostring(typ).."cburl="..tostring(cburl).."citycode="..tostring(citycode))

        only.log("D","adtime="..tostring(adtime).."lng="..tostring(lng).."lat="..tostring(lat).."diff="..tostring(diff).."appKey="..tostring(appKey).."sign="..tostring(sign))
        Adcube_set(content,typ,citycode,adtime,lng,lat,diff,cburl,appKey,sign)
 
end

