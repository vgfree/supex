local utils = require("utils")
local only = require("only")
local gosay = require("gosay")
local redis_api = require("redis_pool_api")
local msg = require("msg")
local dk_utils = require("msgsearchapi_utils")
local supex = require("supex")

module('getcitycodevoice', package.seeall)

local function check_parameter(args)
        args["cityCode"] = tonumber( args["cityCode"] )
        if not args["cityCode"] then
                only.log("E", "cityCode is error")
                return false, "cityCode"        
        end

        args["startTime"] = tonumber( args["startTime"] )
        if not args["startTime"] or string.len(args["startTime"]) ~= 10 then
                only.log("E", "startTime is error!")
                return false, "startTime" 
        end 

        args["endTime"] = tonumber( args["endTime"] )
        if not args["endTime"] or string.len(args["endTime"]) ~= 10 then
                only.log("E", "endTime is error.")
                return false, "endTime"
        end 

        if args["startTime"] > args["endTime"] then
                args["startTime"] ,args["endTime"] = args["endTime"], args["startTime"]
        end 

        if args["endTime"] - args["startTime"] > dk_utils.timeRangeLimit then
                only.log("E", "startTime: ".. args["startTime"])
                only.log("E", "endTime: ".. args["endTime"])
                only.log("E", "time interval out of range!")
                local errlog = string.format("time interval must less than %d hours. startTime or endTime", dk_utils.timeRangeLimit/3600)
                return false, errlog
        end 

        args["startNum"] = tonumber( args["startNum"] )
        if args["startNum"] == 0 then
                return false, "startNum at least 1" 
        end 
        args["endNum"] = tonumber( args["endNum"] )
        if args["endNum"]  == 0 then
                only.log("E", "endNum is error.")
                return false, "endNum  at least 1"
        end 

        return true
end

function handle()
        local args = nil 

        if supex.get_our_body_data() then
                args = supex.get_our_body_table()
        else
                args = supex.get_our_uri_table()
        end

        -->> check parameters.
        local ok, errlog = check_parameter(args)
        if not ok then
                return gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'], errlog)
        end

        -->> get data.
        local bizid
        local hashkey
        local key
        local vtab,vvtab
        local i,j
        key = string.format("cityCodeMessage:%s:", args["cityCode"])
        hashkey = args["cityCode"]
        local t1, t2, t3 = dk_utils.parse_time(args["startTime"], args["endTime"])
        local tab = {}
        if t1 then
                ok = dk_utils.get_data_from_tsdb_range(key, hashkey, t1, t2, tab)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end
        if t3 then
                ok = dk_utils.smembers_data_from_redis_range(key, hashkey, t2, t3, tab)
                if not ok then
                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                end
        end

        if tab then
                for i,j in pairs(tab) do  
                                if (i%2 == 0) then
                                        vtab = utils.str_split(j, "@")
                                        vvtab = utils.str_split(vtab[2], "|")        
--[[
                                        for _,v in pairs(vvtab) do
                                                k = string.format("fileLocation:%s", v)
                                                filelocation = get_data_from_redis(k, v)
                                                if not filelocation then
                                                        filelocation = get_data_from_tsdb(k, curTime, hashkey)
                                                end 
                                                if filelocation then
                                                        table.insert(data_tab, filelocation)
                                                end 
                                        end 
--]]
                                end 
                end 
        end 
        local startNum = args["startNum"]
        local endNum = args["endNum"]
        local curTime = os.time()
        local url = {}

        if vvtab then
                if not startNum then
                        startNum = 1
                end
                if not endNum or endNum > #vvtab then
                        endNum = #vvtab
                end
        if startNum > #vvtab then
                startNum = #vvtab
        end
                for i=startNum,endNum do
                        k = string.format("fileLocation:%s", vvtab[i])
                        ok, filelocation = dk_utils.get_data_from_redis(k, vvtab[i])
                        if not ok then
                                return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                        end
                        if not filelocation then
                                ok, filelocation = dk_utils.get_data_from_tsdb(k, curTime, vvtab[i])
                                if not ok then
                                        return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'])
                                end
                        end 
                        if filelocation then
                                table.insert(url, filelocation)
                        end 
                        
                end                
        end 

        local str
        if not url then
                url = {}
        end
        ok, str = utils.json_encode(url)

        only.log("D", "range bizid count: " .. #url)

        return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], str)

end 
