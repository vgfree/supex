local only          = require('only')
local supex         = require('supex')
local socket        = require('socket')
local utils         = require('utils')
local redis_api     = require('redis_pool_api')
local safe          = require('safe')
module('adcube_del', package.seeall)
--[[                                                                                                                                                                                                                                    
函数名：AdCube_del
功  能：删除广告号
参  数：aid 广告号 ,appKey app标识 , sign 签名
返回值：true 成功删除   false 删除失败
]]--

local function AdCube_del(aid,appKey,sign)
    
    if tostring(aid) ~="nil" and tostring(appKey) ~= "nil" and tostring(sign) ~= "nil"  then
 
          local tab     = {}
  	      tab["appKey"] = appKey
   	      tab["sign"]   = sign
   	      tab["aid"]   = aid
   	 --检查 appKey  与  sign
          local  value  = safe.sign_check(tab)
          if  not value  then
               only.log("E"," check_parameter(appKey,sign) is not correct ")
	           local afp = supex.rgs(200)
               local string = '{"result":"nok","error":"check_parameter(appKey.sign) is not correct"}'.."\n"
               supex.say(afp,string)
               return supex.over(afp)
          end

     
          only.log("D","aid = %s\n",aid)
          local aid1 = "A_"..aid
 	      local ok,aid_table  = redis_api.cmd('private1','','hgetall',aid1)
     	  if not ok or not aid_table.Typ  then
      	       only.log("E",  " hgetall aid failed! "..aid);
	           local afp = supex.rgs(200)
               local string = '{"result":"nok","error":"hgetall aid failed"}'.."\n"
               supex.say(afp,string)
               return supex.over(afp)
    	  end

 	      if aid_table.Diff ~= nil then
               local ok,ret = redis_api.cmd('private1','','srem','GPS',aid1)               
               if not ok or not ret then
          	       only.log("E",  " srem_GPS aid failed! "..aid);
	               local afp = supex.rgs(200)
                   local string = '{"result":"nok","error":"srem_GPS aid failed"}'.."\n"
                   supex.say(afp,string)
                   return supex.over(afp)
               end

               local ok,ret = redis_api.cmd('private1','','del',aid1)
               if not ok or not ret then
	               only.log("E",  "hdel_gps  aid failed! "..aid);
	               local afp = supex.rgs(200)
                   local string = '{"result":"nok","error":"hdel_gps aid failed"}'.."\n"
                   supex.say(afp,string)
                   return supex.over(afp)
   	           end

	      else
  	           local Citycode = aid_table.Citycode
               Citycode = tostring(Citycode)
               local tab_city = utils.str_split(Citycode,'|')
               for i=1,#tab_city do
 	               local ok,ret = redis_api.cmd('private1','','srem',tab_city[i],aid1)               
	               if not ok or not ret then
    	                 only.log("E",  " srem_tab_city aid failed! "..aid);
	                     local afp = supex.rgs(200)
                         local string = '{"result":"nok","error":"srem_tab_city aid failed"}'.."\n"
                         supex.say(afp,string)
                         return supex.over(afp)
	               end
               end   
  	           local ok,ret = redis_api.cmd('private1','','del',aid1)
  	           if not ok or not ret then
   	               only.log("E",  "hdel_Citycode  aid failed! "..aid);
	               local afp = supex.rgs(200)
                   local string = '{"result":"nok","error":"hdel_Citycode aid failed"}'.."\n"
                   supex.say(afp,string)
                   return supex.over(afp)
	           end
             
	       end
   	           local afp = supex.rgs(200)
               local string = '{"result":"ok"}'.."\n"
               supex.say(afp,string)
               only.log("D","del adverstring success"..tostring(aid))
               return supex.over(afp)
     else
           only.log("E","The format is not correct")
           local afp = supex.rgs(200)
           local string = '{"result":"nok","error":"The format is not correct"}'.."\n"
           supex.say(afp,string)
           return supex.over(afp)
     end
end



function handle()
	    only.log("D","del interface start ...")
   
        local data        = supex.get_our_body_table()
        local aid         = data['aid']
        local appKey      = data['appKey']
        local sign        = data['sign']
        only.log("D","aid ="..tostring(aid))
        AdCube_del(aid,appKey,sign)

end
