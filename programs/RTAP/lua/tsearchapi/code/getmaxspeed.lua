local decod = require('libluadecoder')
local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("tsearchapi_utils")
local utils = require("utils")
local supex = require("supex")

module('getmaxspeed', package.seeall)

local timeLimit = 48*60*60

local function check_parameter(args)
        -- check imei
        if (not utils.is_number(args["imei"])) or (string.len(args["imei"]) ~=15)
                        or (string.find(args["imei"],"0") == 1) then
                only.log("E","imei is error!")
                return false,"imei"
        end

        --check startTime
        args["startTime"] = tonumber(args["startTime"])
        if not args["startTime"] or string.len(args["startTime"]) ~= 10 then
                only.log("E","startTime is error!")
                return false,"startTime"
        end

        --check endTime
        args["endTime"] = tonumber(args["endTime"])
        if not args["endTime"] or string.len(args["endTime"]) ~= 10 then
                only.log("E","startTime is error!")
                return false,"endTime"
        end

        --FIXME:if the timeLimit more the 48 hour,then return false
        if args["endTime"]-args["startTime"] > timeLimit then
                only.log("E","startTime: ".. args["startTime"])
                only.log("E","endTime: "..args["endTime"])
                only.log("E","time interval out of range!")
                local errlog = string.format("time interval must less than %d hours.startTime or endTime",48*60*60/3600)
                return false,errlog
        end
        return true
end

--input: imei,startTime,endTime
--output:maxspeed
--

local function get_maxspeed_from_tsdb(imei, startTime, endTime)
        local tsdbmax = 0
        local jumpTime = 5*60*60
        
        --jumpTime:avgerage 5 hour take data 
        for i=startTime,endTime,jumpTime do
                local prekey,skey,ekey

                prekey = "GPS:"..imei..":"
                skey = string.format("%s%d",os.date("%Y%m%d%H",i),os.date("%M",i)/10)
                if  (i+jumpTime < endTime) then
                        ekey = string.format("%s%d",os.date("%Y%m%d%H",i+jumpTime),os.date("%M",i+jumpTime)/10)
                else
                        ekey = string.format("%s%d",os.date("%Y%m%d%H",endTime),os.date("%M",endTime)/10)
                end

                only.log("D","tsdb-cmd:lrange " .. prekey .. " " .. skey .. " " .. ekey)
                local ok,tab = redis_api.only_cmd("tsdb", "lrange", prekey, skey, ekey)
                if not ok then
                        only.log("E","lrange tsdb error.  ")
                        return false
                end

                local tab_value = {}
                for i=2,#tab,i+2 do
                        table.insert(tab_value,tab[i])
                end
                local tab1 = decod.decode(#tab_value,tab_value)

                if not tab1 then
                        only.log("E","tab1 decode error.")
                        return false 
                end

                for i=1,#tab1 do
                        if #tab1[i] ~= 12 then
                                only.log("E", "gps format is error.")
                                return false
                        end
                        tab1[i][11] = tonumber(tab1[i][11])
                        only.log("D","speed:" .. tab1[i][11])
                        only.log("E","speed: " .. tab1[i][11])
                        if tab1[i][11] > tsdbmax then
                                tsdbmax = tab1[i][11]
                        end
                end
                only.log("D","maxspeed is: "..tsdbmax)
        end
        return true, tsdbmax
end


function handle()
        local args = nil 

        if supex.get_our_body_data() then
                args = supex.get_our_body_table()
        else
                args = supex.get_our_uri_table()
        end

        local ok,errlog = check_parameter(args)
        if not ok then
                return gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'],errlog)
        end
        local t1,t2,t3 = dk_utils.parse_time(args["startTime"],args["endTime"])

        local maxspeed = 0
        ok, maxspeed = get_maxspeed_from_tsdb(args["imei"],t1,t2)
        if not ok then
                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
        end
        
        only.log("D","maxspeed:" .. maxspeed)
        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],maxspeed)
end
