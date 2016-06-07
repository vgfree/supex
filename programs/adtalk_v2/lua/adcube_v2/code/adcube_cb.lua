local REDIS_API = require('redis_pool_api')
local supex = require('supex')
local link = require('link')
local cjson = require("cjson")
local only = require('only')
local safe = require('safe')
local ltn12 = require ('ltn12')
local http = require ('socket.http')
local socket = require('socket')
local scan = require('scan')
--函数:cb
--功能:回调aid对应的url
--参数:aid广告id, status用户的状态
--返回值:返回http远程调用的反馈消息
--注意

module("adcube_cb",package.seeall)


function __cb(args)

	for k,v in ipairs(args) do
		if not v then
			only.log("E", "incorrect paramete!\n")
			return 1
		end
	end

	--check appKey  and  sign
	--[[
	local ret = safe.sign_check(tab)
	if not ret then
	only.log("E", "appKey or sign incorrect!\n")
	return 2
	end
	]]--
	local aid1 = "A_"..args['aid']
	only.log("D","aid1 ="..tostring(aid1))
	local ok, ret_url = REDIS_API.cmd("private1", "", "hget", aid1,"Cburl")
	only.log("D","url ="..tostring(ret_url))
	if not ok or not ret_url  then
		only.log("E", " redis hget_aid Url do failed!\n")
		return 3
	else
		return  cb(args, ret_url)
	end
end


function cb(args,ret_url)
	local ok,str = pcall(cjson.encode, args)
	if not ok then
		only.log('E', "failed to encode result")
	end
	--local str = '{"aid":' ..'\"'.. aid .. '\"' .. ',"cid":'..'\"'..cid..'\"'..',"report":'..report..',"sign":'..'\"'..sign ..'\"' .. ',"time":'..'\"'..time..'\"'..',"appKey":'..'\"' ..appKey..'\"'..'}';

	only.log("D","ret_url ="..tostring(ret_url))

	local response_body = {}
	res, code = http.request{
		url = ret_url,
		method = "POST",
		headers =
		{
			["Content-Type"] = "application/json",
			["Content-Length"] = #str,
		},
		source = ltn12.source.string(str),
		sink = ltn12.sink.table(response_body)
	}
	code = tonumber(code)
	local ret_num = 0
	only.log("D","code="..tostring(code))
	if(code ==200 ) then
		only.log("D", " post request  success\n")
		args["cbstatus"]=200
		ret_num =  5
	else
		only.log("E", " post request do failed!\n")

		args["cbstatus"]=400
		ret_num = 4
	end
	local ok,ret_str = pcall(cjson.encode, args)
	local key = "mid:" .. args['aid'] .. ":" .. args['mid']
	local ok,cbtimes = REDIS_API.cmd("private1", "", "HINCRBY", key,"cbtimes","1")
	local value = "cb"..cbtimes
	local ok = REDIS_API.cmd("private1", "", "hset", key, value ,ret_str)
	if not ok then
		only.log("E", " redis hset cb do failed!\n")
	end
	only.log("D","%s",ret_str)
	return ret_num
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

function string:split(sep)
	local sep, fields = sep or "\t", {}
	local pattern = string.format("([^%s]+)", sep)
	self:gsub(pattern, function(c) fields[#fields+1] = c end)
	return fields
end

function handle()
	only.log("D","cb interface start ...")
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
	local args = {}

	local res       = supex.get_our_body_table()
	args['aid']       = res["aid"]
	args['report']    = res["report"]
	args['mid']       = res["mid"] or "midempty"
	args['cid']       = ret['cid'] or res["cid"]
	args['time']      = ret['time'] or res["time"]
	args['appKey']    = ret['appKey'] or res["appKey"]
	args['sign']      = ret['sign'] or res["sign"]
	only.log('D','args is %s',scan.dump(args))

	local number = __cb(args)
	number = tonumber(number)
	only.log('D',"number:"..number)
	re_back(number)
end
