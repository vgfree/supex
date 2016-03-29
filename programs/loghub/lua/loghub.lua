local only        = require('only')
local apis        = require('API_LIST')
local cfg         = require("cfg")
local email       = require("send_email")
local link        = require("link")
local redis_api   = require('redis_short_api')
local luakv_api   = require('luakv_pool_api')

local LOG_HUB_APPEND = cfg["LOG_HUB_APPEND"]
local LOG_HUB_NOTIFY = cfg["LOG_HUB_NOTIFY"]
local LOG_HUB_PRINTF = cfg["LOG_HUB_PRINTF"]
local LOG_HUB_FILTER = cfg["LOG_HUB_FILTER"]
local LOG_HUB_MAXLEN = cfg["LOG_HUB_MAXLEN"]


module("loghub", package.seeall)

mysql_dbname = link['OWN_DIED']['mysql']['database']

local data_tab = {
	Jan = '01',
	Feb = '02',
	Mar = '03',
	Apr = '04',
	May = '05',
	Jun = '06',
	Jul = '07',
	Aug = '08',
	Sep = '09',
	Oct = '10',
	Nov = '11',
	Dec = '12',
}


local G = {
	secret = nil,
}

local function save_logs_list(info)
	if type(info) ~= "table" then
		only.log("E", "not table type")
	end
	local filepath = cfg["LOGSV"] or "./"
	filepath = string.format("%s/%s/%s", filepath, info[1], info[2])
	filepath = string.gsub(filepath, "/+", "/")
	filepath = string.gsub(filepath, "[\n\t ]+", "")
	--print( ".....", filepath )

	local fd = io.open(filepath, "a")
	if not fd then
		os.execute( string.format("mkdir -p `dirname %s`", filepath) )
		fd = io.open(filepath, "a")
		if not fd then
			only.log("E", "open file failed")
			return
		end
	end
	fd:write( info[#info] )
	fd:close()
end

--功    能:分割loghit发来的日志字符串 并将不完整的数据保存到redis
--参    数:str:loghit发来的数据 key:redis key rule:匹配规则 num1,num2,num3:匹配规则向前找字符串的长度
--返 回 值:返回分割后的字符串数组
local function split_string(str,key,rule,num1,num2,num3)
	local number_tab = {}
	local logstr_tab = {}
	local st = 0
	local ok,ret = redis_api.cmd("private","get",key)
	redis_api.cmd("private","del",key)
	if not ok then return end
	if ret then
		str = ret .. str
	end
	while true do
		st = string.find(str,rule,st+1)
		if st == nil then break end
		table.insert(number_tab, st)
	end
	
	for i=1,#number_tab,1 do
		if i+1 > #number_tab then else
			local str = string.sub(str,number_tab[i]-num1,number_tab[i+1]-num2)
			table.insert(logstr_tab, str)
		end
	end
	
	if number_tab[#number_tab] then
		local str = string.sub(str,number_tab[#number_tab]-num3,#str)
		redis_api.cmd("private","set",key,str)
	end
	
	return logstr_tab
end

--功    能:分析统计transit 请求流量并将结果保存到redis里
function statistics_request_flow(args)
	if LOG_HUB_APPEND then
		save_logs_list( args )
	end
	
	local logstr_tab = split_string(args[#args],args[1] .. ":transitlog","(%d%d%/)",0,1,1)	
	if not logstr_tab then return end
	
	local len = #logstr_tab
	local key = ""
	for i=1,len do
		local st,ed,time,imei,flow_request = string.find(logstr_tab[i],"(.*)%+.*%=(%d+).*%|%d%|(%d+)")
		if not time then return end
		local st,ed,day,month,year = string.find(time,"(%d+)%/(.*)%/(%d+).*")
		month = data_tab[month]
		time = year .. month .. day
		key = imei .. ":" .. time
		local ok,val = redis_api.cmd("private","get",key)
		if not ok then return end
	
		if not val then
			redis_api.cmd("private","set",key,flow_request)
		else	
			val = val +flow_request
			redis_api.cmd("private","set",key,val)
		end
			
--		only.log("E","time:" .. time .. " imei:" .. imei .. " flow:" .. flow_request)
	end
end

--功   能:发送邮件
local function email_send(file, msg)
	local api_ower_tel = cfg["API_OWER_EMAIL"]

	-->send email
	for i,v in pairs(api_ower_tel) do
		local ok,ret = email.sendemail("你的程序产生了ERROR，快滚去修复!!!",file .. "ERROR LOG IS:\n" .. msg,v) 
		if not ok then
			only.log('E',"email send err :%S",tostring(ret))
		else
			only.log('I',i)
			only.log('I', "email send succ")
		end
	end
end

local G_COLOR = {
	DEFAULT         = "\x1B[0m",
	BLACK           = "\x1B[30;2m",
	RED             = "\x1B[31;2m",
	GREEN           = "\x1B[32;2m",
	YELLOW          = "\x1B[33;2m",
	BLUE            = "\x1B[34;2m",
	PURPLE          = "\x1B[35;2m",
	SKYBLUE         = "\x1B[36;2m",
	GRAY            = "\x1B[37;2m",
}

local TONE_LIST = {
	DEBUG		= G_COLOR["GREEN"],
	INFO		= G_COLOR["BLUE"],
	WARN		= G_COLOR["YELLOW"],
	ERROR		= G_COLOR["RED"],
	SYS		= G_COLOR["PURPLE"],
	FATAL		= G_COLOR["SKYBLUE"]
}

local KIND_LIST = {
	DEBUG		= 1,
	INFO		= 2,
	WARN		= 3,
	ERROR		= 4,
	SYS		= 5,
	FATAL		= 6
}

--功    能:已经发送过邮件 不在重复发送
--参    数:rank,错误级别 file,服务器标识 data，错误日志信息
--返 回 值:已经发送过返回false，没有发送过返回true
local function remove_repeat_errstr(rank,data,file)
	local st,ed,file_line = string.find(data,"%|.*%|%s+.*%|%s+(%a+.*.%a+:%s+%d+)%|")
	file_line = string.gsub(file_line,  " ", "")---delete space
	local key = file .. file_line 
	key = string.gsub(key,  " ", "")---delete space
	key = string.gsub(key, "\n", "")---delete "/n"
	local val = file .. rank .. file_line 
	val = string.gsub(val,  " ", "")---delete space
	val = string.gsub(val, "\n", "")---delete "/n"
	
	local ok,ret = luakv_api.cmd("private","","get",key)
	if not ok then return end
	if not ret then
		luakv_api.cmd("private","","set",key,val)
		luakv_api.cmd("private","","expire",key,60*60*12)
	else
		if val==ret then 
			only.log('D',"not send email because have been send")
			return false
		end
	end

	return true
end

local function output_all( tone, kind, head, data, file, rank)
	--> cut
	if #data > LOG_HUB_MAXLEN then
		data = string.sub(data, 1, LOG_HUB_MAXLEN) .. " [. . .]\n\n"
	end
	--> print
	if LOG_HUB_PRINTF and kind >= LOG_HUB_FILTER then
		os.execute(string.format('echo -ne "%s%s%s"', tone, head, data))
	end
	--> email a full msg
	if LOG_HUB_NOTIFY and head ~= "" and kind >= 4 then
		-->>err email去重
		local ok = remove_repeat_errstr(rank,data,file)
		if not ok then return end

		email_send(file ,head .. data )
	end
end

--功    能:分析日志,保存日志到本地 发现错误日志 发送邮件报警
function analysis_log( args ,filename)
	-->> storage
	if LOG_HUB_APPEND then
		save_logs_list( args )
	end

	-->> email or print
	if (not LOG_HUB_NOTIFY) and (not LOG_HUB_PRINTF) then
		return 0
	end

	-->>分割发来的数据 并将不完整的数据保存到redis
	local key = args[1] .. ":" .. filename .. ":loghub"
	key = string.gsub(key, "\n", "")---delete "/n"
	local logstr_tab = split_string(args[#args],key,"(%|%d+-%d+-%d+ %d+:%d+:%d+.%d+.*%|)",6,7,6)
	if not logstr_tab then return end
	local len = #logstr_tab
	
	for i=1,len do
		local data = logstr_tab[i]
		local tone = G_COLOR["DEFAULT"]--v
		local kind = 0
		local head = ""
		local rank_s = ""
		
		repeat
			local st,ed,more,rank,time,info= string.find(data,"(.-).*%|(.*)%|(%d+-%d+-%d+ %d+:%d+:%d+.%d+)(.*%|.*)")
			if not st then break end
			rank = string.gsub(rank,  " ", "")---delete space
			--info = string.gsub(info, "\n", "")---delete "/n"
			if not more then
				output_all( tone, kind, head, more,args[1] ,rank)
			end
			tone = TONE_LIST[ rank ]
			kind = KIND_LIST[ rank ]
			if not tone or not kind then
				--			only.log('E', "ERROR LOG LEVEL " .. rank)
				tone = TONE_LIST["DEBUG"]
				kind = KIND_LIST["DEBUG"]
			end
			head = time .. " " .. rank
			data = info
			rank_s = rank
		until false
		output_all( tone, kind, head, data,args[1] ,rank_s)
	end
	return kind 
end
