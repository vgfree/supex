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
--功能:回调aid对应的url
--参数:aid广告id, status用户的状态
--返回值:返回http远程调用的反馈消息
--注意

module("adcube_cb",package.seeall)


function __cb(aid, cid,report, time, appKey, sign)

    if tostring(aid)=="nil" or tostring(cid)=="nil"  or tostring(report)=="nil" or tostring(time)=="nil" or tostring(appKey)=="nil" or tostring(sign)=="nil" then
             only.log("E", "incorrect paramete!\n")
             return 1 
    end
    
  --check appKey  and  sign
       local tab = {}
       tab['cid']    = cid
       tab['aid']    = aid 
       tab['report'] = report
       tab['time']   = time
       tab['appKey'] = appKey
       tab['sign']   = sign

       local ret = safe.sign_check(tab)
       if not ret then
              only.log("E", "appKey or sign incorrect!\n")
              return 2
       end

       local aid1 = "A_"..aid
       only.log("D","aid1 ="..tostring(aid1))
       local ok, ret_url = REDIS_API.cmd("private1", "", "hget", aid1,"Cburl")
       only.log("D","url ="..tostring(ret_url))
       if not ok or not ret_url  then
                 only.log("E", " redis hget_aid Url do failed!\n")
                 return 3
       else            
	         return  cb(aid, cid,report,time,appKey, sign, ret_url)
       end
end


function cb(aid,cid,report,time,appKey,sign,ret_url)
    only.log('D',"cid:"..type(cid))
    cid = tostring(cid)
    local str = '{"aid":' ..'\"'.. aid .. '\"' .. ',"cid":'..'\"'..cid..'\"'..',"report":'..report..',"sign":'..'\"'..sign ..'\"' .. ',"time":'..'\"'..time..'\"'..',"appKey":'..'\"' ..appKey..'\"'..'}';
          
       only.log("D","aid ="..tostring(aid))
       only.log("D","ret_url ="..tostring(ret_url))

          local response_body = {}    
          res, code = http.request{  
                           url = ret_url,  
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

         only.log("D","code="..tostring(code))
         if(code ==200 ) then
              only.log("D", " post request  success\n")
              return 5
         else
              only.log("E", " post request do failed!\n")
              return 4
          end  
end          

function re_back(number)
     if number == 1 then
              local afp = supex.rgs(200)
              local string = '{"ERRORCODE":"ME25003", "RESULT":"incorrect paramete!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
     elseif number ==2  then
              local afp = supex.rgs(200)
              local string = '{"ERRORCODE":"ME25004", "RUSULT":"appKey or sign incorrect!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
           
     elseif number ==3  then
              local afp = supex.rgs(200)
              local string = '{"ERRORCODE":"ME25005", "RESULT":"other errors!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
          
     elseif number ==4  then
              local afp = supex.rgs(200)
              local string = '{"ERRORCODE":"ME25001", "RESULT":"https request do failed!"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
          
     else
              local afp = supex.rgs(200)
              local string = '{"ERRORCODE":"0","RESULT":"ok"}' .. '\n'
              supex.say(afp, string)
              return supex.over(afp)
    end
end

function handle()
    only.log("D","cb interface start ...")
	local res       = supex.get_our_body_table()
	local aid       = res["aid"]
	local cid       = res["cid"]
	local report    = res["report"]
	local time      = res["time"]
	local appKey    = res["appKey"]
	local sign      = res["sign"]

    only.log("D","aid ="..tostring(aid).."cid ="..tostring(cid).."report ="..tostring(report).."time ="..tostring(time).."appKey ="..tostring(appKey).."sign ="..tostring(sign))
    local number = __cb(aid, cid,report,time,appKey, sign)
    number = tonumber(number)
    only.log('D',"number:"..number)
    re_back(number)
end
