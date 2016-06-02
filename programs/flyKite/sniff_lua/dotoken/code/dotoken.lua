local only = require('only')
local utils = require('utils')
local link  = require ("link")
local msg = require('msg')
local map = require('map')
local json = require('cjson')
local supex = require('supex')
local lua_decoder = require('libluadecoder')
local http_api = require('http_short_api')
local utils = require('utils')
local socket = require('socket')
local cfg = require('cfg')
local scan = require('scan')
local func_search_poi = require('func_search_poi')
local mysql_pool_api = require('mysql_pool_api')
local domile = require('domile')
local ONEDAY = 60 *60*24

module('dotoken', package.seeall)

--功能作用：参数检查
--参数    ：accountID,imei,startTime,endTime
--返回值  ：返回 ok，false 
local function check_parameter(args)
    only.log('D', '###$$$>>>>check args ')
        -->> safe check
       --safe.sign_check(args, url_tab)
    if not args['accountID']  then
        only.log('E', "requrie json have nil of \"accountID\"")
        gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"accountID")
    elseif string.len(args['accountID']) == 10 then
        local ok, imei = redis_pool_api.cmd('private', 'get', string.format('%s:IMEI', args['accountID']))
        if not ok or not imei then
            only.log('E', "get IMEI error")
            gosay.go_false(url_tab, msg['MSG_DO_REDIS_FAILED'])
        end
        args['imei'] = imei
    elseif string.len(args['accountID']) == 15 then
        local ok, accountID = redis_pool_api.cmd('private', 'get', string.format('%s:accountID', args['accountID']))
        if not ok or not accountID then
            only.log('E', "get accountID error")
            accountID = ''
        end
        args['imei'] = args['accountID']
        args['accountID'] = accountID›  
    end
    if not args['tokenCode'] then
            only.log('E', "requrie json have nil of \"tokenCode\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"tokenCode")
    end
        args['startTime'] = tonumber(args['startTime'])
        args['endTime'] = tonumber(args['endTime'])
    if not args['endTime'] then
            only.log('E', "requrie json have nil of \"time\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"endTime")
    end
        args['count'] = tonumber(args['count'])  
    if not args['count'] then
            only.log('E', "requrie json have nil of \"count\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"count")
    end    
    return args
end

local function mileageinfo_of_time(t)    
    local tab = os.date("*t",t)
    tab.hour = 0
    tab.min = 0
    tab.sec = 0
    et = os.time(tab)
    et = et -1
    return et
end

local function from_mileagetable_get_tokenCodeinfo(args) 

    local tokenCodeinfo = {}
    local etdate = os.date("*t",args['endTime']) 
    local st = args['endTime'] - args['count'] * ONEDAY
    local stdate = os.date("*t",st)
    local count = etdate.month - stdate.month 
    if count < 0 then
        count = (etdate.month + 12) - stdate.month 
    end
        count = count + 1 
    local monthcount = etdate.month
    local et = mileageinfo_of_time(args['endTime'])
    if etdate.year < 2016 then
        return
    end
    for i = count,1,-1 do
        if  monthcount >= 10 and monthcount <= 12 then     
            monthinfo = string.format("mileageInfo%s%s",etdate.year,monthcount)
        else
            monthinfo = string.format("mileageInfo%s0%s",etdate.year,monthcount)
        end
        local dotokenType = "SELECT tokenCode,startTime,endTime FROM %s WHERE imei = %s and startTime >= %s and startTime <= %s"
        local ok,retdata = mysql_pool_api.cmd("dotoken","select",string.format(dotokenType,monthinfo,args['imei'],st,et)) 
       only.log('D',"retdata"..scan.dump(retdata))
	    if not ok and not retdata then
            only.log('E', "select mysql retdata  FROM mileageInfo error!")
            gosay.go_false(url_tab, msg['MSG_DO_MYSQL_FAILED'])
        end
        table.insert(tokenCodeinfo,retdata)
        monthcount = monthcount - 1
        if monthcount == 0 then
           etdate.year = etdate.year - 1
           monthcount = 12    
        end

    end 
    only.log('D',"tokenCodeinfo"..scan.dump(tokenCodeinfo))
    return true,tokenCodeinfo
end

function http_send_mileageinfo(imei,startTime,endTime,tokenCode)

    local body_info = {imei = imei,startTime = startTime,endTime = endTime,tokenCode = tokenCode,}
    local serv = link["OWN_DIED"]["http"]["mileageinfopost"]
    local body = utils.gen_url(body_info)
    only.log('D',"body"..scan.dump(body))
    local data = "POST /domile HTTP/1.0\r\n" ..                                                                                                                     
    "User-Agent: curl/7.33.0\r\n" ..
    "Content-Type: application/x-www-form-urlencoded\r\n" ..
    "Connection: close\r\n" ..
    "Content-Length:" .. #body .. "\r\n" ..
    "Accept: */*\r\n\r\n" ..
    body
    local ok,ret = supex.http(serv['host'],serv['port'],data,#data)
    if not ok or not ret then return nil end
end


function do_tokenCodeinfo_get_mileagedata(args,tokencodeinfo)  
    for k1,v1 in pairs(tokencodeinfo) do
        for k,v in pairs(v1) do          
           http_send_mileageinfo(args['imei'],v["startTime"],v["endTime"],v["tokenCode"]) 
        end      
    end
    return true 
end


function handle()

    local args = supex.get_our_body_table()
    only.log('D', "args" .. scan.dump(args))
    local args = check_parameter(args)
    local ok,tokencodeinfo = from_mileagetable_get_tokenCodeinfo(args)
    only.log('D',"tokencodeinfo"..scan.dump(tokencodeinfo))
    if not ok then
    only.log('D', "get tokenCodeinfo is failed")
    end
   local ok = do_tokenCodeinfo_get_mileagedata(args,tokencodeinfo)
    if ok then
    only.log('D', "get whole mileagedata is succeed")
    end

end
