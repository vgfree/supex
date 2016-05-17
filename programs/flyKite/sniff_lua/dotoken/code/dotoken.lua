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

local THIRTY    = 3600 * 24 * 30
local time 

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
    if not  args['tokenCode'] then
            only.log('E', "requrie json have nil of \"tokenCode\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"tokenCode")
    end
        args['startTime'] = tonumber(args['startTime'])
        args['endTime'] = tonumber(args['endTime'])
    if not args['startTime'] then
            only.log('E', "requrie json have nil of \"time\"")
            gosay.go_false(url_tab, msg['MSG_ERROR_REQ_ARG'],"startTime")
    end
    return args
end

function Time (t)
    local tab = os.date("*t",t)
    tab.day = tab.day - 1
    tab.hour = 23
    tab.min = 59
    tab.sec = 59
    time = os.time(tab)
end

function sleep(n)
   os.execute("sleep " .. n)
end

function handle()

    local args = supex.get_our_body_table()
    only.log('D', "args" .. scan.dump(args))
    check_parameter(args)
    local starttime = args['startTime']
    Time(args['startTime'])
    local i,sum = 1,0
    local dotokenType = "SELECT tokenCode,startTime,endTime FROM mileageInfo201604 WHERE imei = %s and startTime <= %s union SELECT tokenCode ,startTime ,endTime FROM mileageInfo201603 WHERE imei = %s and startTime >= %s"
    dotokenType  = string.format(dotokenType,args['imei'],time,args['imei'],starttime -THIRTY)
    local ok,retdata = mysql_pool_api.cmd("dotoken","select",dotokenType)
    only.log('D', "retdata" .. scan.dump(retdata))
    if not ok then
        only.log('E', "select mysql retdata  FROM mileageInfo201604 error!")
        gosay.go_false(url_tab, msg['MSG_DO_MYSQL_FAILED'])
    else
            local retcount = table.maxn(retdata)
            local x1 = os.clock()
            for k,v in pairs(retdata) do
                    local ok,miledata = domile.mileage_time_handle(args['imei'],tonumber(v["startTime"]),tonumber(v["endTime"]))
                    if ok and miledata then
                        local d = domile.get_data()
                        sleep(5)    
                        sum = sum + d   
                    end
               	    if i == retcount then
                   	     break
                    end
                    i = i + 1
            end 
            local x2 = os.clock() 
            local p = x2 -x1
            only.log('D', string.format("p = :%s", scan.dump(p)))    
    end

