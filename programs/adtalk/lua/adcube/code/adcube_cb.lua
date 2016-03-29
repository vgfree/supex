local REDIS_API = require('redis_pool_api')
local supex = require('supex')
local link = require('link')
local cjson = require("cjson")
local only = require('only')
local safe = require('safe')
local ltn12 = require ('ltn12')
local http = require ('socket.http')
local socket = require('socket')
--函数:cb
--功能:回调asid对应的url
--参数:asid广告id, status用户的状态
--返回值:返回http远程调用的反馈消息
--注意

module("adcube_cb",package.seeall)


function __cb(asid, cid,citycode, status, time, appKey, sign)

    if tostring(asid)=="nil" or tostring(cid)=="nil"  or tostring(status)=="nil" or tostring(time)=="nil" or tostring(appKey)=="nil" or tostring(sign)=="nil" then
             only.log("E", "incorrect paramete!\n")
             return 1 
    end
    
    if tostring(citycode)=="nil"  then
  --check appKey  and  sign
       local tab = {}
       tab['cid']    = cid
       tab['aid']    = asid 
       tab['status'] = status
       tab['time']   = time
       tab['appKey'] = appKey
       tab['sign']   = sign

       local ret = safe.sign_check(tab)
       if not ret then
              only.log("E", "appKey or sign incorrect!\n")
              return 2
       end
   else 
       local tab = {}
       tab['cid']    = cid
       tab['aid']    = asid 
       tab['citycode']= citycode 
       tab['status'] = status
       tab['time']   = time
       tab['appKey'] = appKey
       tab['sign']   = sign

       local ret = safe.sign_check(tab)
       if not ret then
              only.log("E", "appKey or sign incorrect!\n")
              return 2
       end
   end
       citycode = tostring(citycode)
       local asid1 = "A_"..asid
       only.log("D","asid1 ="..tostring(asid1))
       local ok, res = REDIS_API.cmd("private1", "", "hget", asid1,"Cburl")
       only.log("D","url ="..tostring(res))
       if not ok or not res  then
                 only.log("E", " redis hget_aid Url do failed!\n")
                 return 3
       else            
	            return  cb(asid, cid,status, time,appKey, sign, res,citycode)
       end
end


function cb(asid,cid,status,time,appKey,sign,ural,citycode)
    only.log('D',"cid:"..type(cid))
    cid = tostring(cid)
    local str = '{"aid":' ..'\"'.. asid .. '\"' .. ',"cid":'..'\"'..cid..'\"'..',"citycode":'..'\"'..citycode..'\"'..',"status":'..'\"'..status ..'\"' .. ',"time":'..'\"'..time..'\"'..',"appKey":'..'\"' ..appKey..'\"'..'}';
          
       only.log("D","asid ="..tostring(asid))
       only.log("D","ural ="..tostring(ural))

          local response_body = {}    
          res, code = http.request{  
                           url = ural,  
                           method = "POST",  
                           headers =   
                                 {  
                                   -- ["Content-Type"] = "application/x-www-form-urlencoded",  
                                    ["Content-Type"] = "application/json",  
                                    ["Content-Length"] = #str,  
                                 },  
                           source = ltn12.source.string(str),  
                           sink = ltn12.sink.table(response_body)  
                           }  
     --    print(code)
         code = tonumber(code)
         only.log("D","sinkaaaa="..type(sink))
         only.log("D","res="..tostring(res))

         only.log("D","code="..tostring(code))
         if(code ==200 ) then
              only.log("D", " post request  success\n")
              return 5
         else
              only.log("E", " post request do failed!\n")
              return 4
          end  
end          

function retu(number)
     if number == 1 then
              local afp = supex.rgs(200)
              local string = '{"result":"nok", "error":"incorrect paramete!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
     elseif number ==2  then
              local afp = supex.rgs(200)
              local string = '{"result":"nok", "error":"appKey or sign incorrect!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
           
     elseif number ==3  then
              local afp = supex.rgs(200)
              local string = '{"result":"nok", "error":"redis opr do failed!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
          
     elseif number ==4  then
              local afp = supex.rgs(200)
              local string = '{"result":"nok", "error":"http requst do failed!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
          
     else
              local afp = supex.rgs(200)
              local string = '{"result":"ok"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
    end
end

function handle()
    only.log("D","cb interface start ...")
	local res       = supex.get_our_body_table()
	local aid       = res["aid"]
	local cid       = res["cid"]
	local citycode  = res["citycode"]
	local status    = res["status"]
	local time      = res["time"]
	local appKey    = res["appKey"]
	local sign      = res["sign"]

    only.log("D","aid ="..tostring(aid).."cid ="..tostring(cid).."status ="..tostring(status).."time ="..tostring(time).."appKey ="..tostring(appKey).."sign ="..tostring(sign))
    local number = __cb(aid, cid,citycode, status, time,appKey, sign)
    number = tonumber(number)
    only.log('D',"number:"..number)
    retu(number)
end
