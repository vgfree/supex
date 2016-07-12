--local libhttps = require("libhttps")
local utils = require('utils')
--local app_utils = require('app_utils')
local redis_api = require('redis_short_api')
local only = require('only')
local cutils = require('cutils')
local cjson = require('cjson')
local http_api = require('http_short_api')
local apis = require('API_LIST')
local cfg = require("cfg")

local LOG_HUB_APPEND = cfg["cli_info"]["LOG_HUB_APPEND"]
local LOG_HUB_NOTIFY = cfg["cli_info"]["LOG_HUB_NOTIFY"]
local LOG_HUB_PRINTF = cfg["cli_info"]["LOG_HUB_PRINTF"]
local LOG_HUB_FILTER = cfg["cli_info"]["LOG_HUB_FILTER"]
local LOG_HUB_MAXLEN = cfg["cli_info"]["LOG_HUB_MAXLEN"]


module("loghub", package.seeall)

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


local function sms_send(msg)
--	do return end
	local _st, _ed = string.find(msg, "%]% %(%w+%)% %-%-%>")
	if not _st then return end

	local api_name = string.sub(msg, _st + 3, _ed - 5)
	--print(api_name)
	local msg_main = string.sub(msg, _ed, -1) 
	local sha = require('sha1')
	local msgid =  sha.sha1(msg_main)
	--print(msgid)
	local keyid = api_name .. ":" .. msgid
	local ok,val = redis_api.cmd("logs", "get", keyid)
	if ok and val then return end

	local keyct = api_name .. ":" .. "smsSendCount"
	ok,val = redis_api.cmd("logs", "get", keyct)
	if not ok then return end
	if not val then
		redis_api.cmd("logs", "setex", keyct, cfg["cli_info"]["LOG_HUB_SMS_SPACE"],1)
		redis_api.cmd("logs", "setex", keyid, cfg["cli_info"]["LOG_HUB_SMS_SPACE"],keyid, msg_main)
	else
		if tonumber(val) >= cfg["cli_info"]["LOG_HUB_MAXSMS"] then return end

		local ok,ttl = redis_api.cmd("logs", "ttl", keyct) -->FIXME
		redis_api.cmd("logs", "setex", keyct, ttl,tonumber(val) + 1)
		redis_api.cmd("logs", "setex", keyid, cfg["cli_info"]["LOG_HUB_SMS_SPACE"],msg_main)
	end


	-->> send sms
	if cfg["cli_info"]["USE_FETHION"] == 1 then
		local api = "2.smsfx.sinaapp.com/send.php"
		local url = api .. '?' .. string.format("tel=%s&pwd=%s&aim=%s&text=%s", 
		cfg["cli_info"]["FETION_TEL"], cfg["cli_info"]["FETION_PWD"], cfg["cli_info"]["FETION_AIM"], msg)
		only.log('I', url)

		local request = string.format("GET /%s HTTP/1.1\r\nUser-Agent: curl/7.27.0\r\nAccept: */*\r\nAccept-Language: zh-cn\r\nContent-Type: application/x-www-form-urlencoded\r\nHost: 2.smsfx.sinaapp.com\r\nConnection: close\r\n\r\n", url)
		only.log('D', request)
		ret = libhttps.https("2.smsfx.sinaapp.com", 443, request, string.len(request))
		only.log('D', ret)

	else
		local api_ower = apis[api_name]
		local api_ower_tel
		if api_ower then
			api_ower_tel = cfg["cli_info"]["API_OWER_TEL"][api_ower]
		else
			api_ower_tel = cfg["cli_info"]["API_OWER_TEL"]["unknow"]
		end

		if not G["secret"] then
			ok,G["secret"] = redis_api.cmd("public", "get",  cfg["cli_info"]["agent"] .. ':secret')
		end
		-->> start to send sms
		local srv = cfg["srv_info"]["sms"]
		local api = "sms.json"
		local tb_kv = {}
		tb_kv["agent"] = cfg["cli_info"]["agent"]
		tb_kv["serviceType"] = cfg["cli_info"]["serviceType"]
		tb_kv["receiver"] = api_ower_tel
		tb_kv["content"] = msg
		local sign_val = app_utils.gen_sign(tb_kv, G["secret"])
		tb_kv["sign"] = sign_val
		-->> urlcode
		tb_kv["content"] = cutils.url_encode(msg)

		local ok,data = pcall(cjson.encode, tb_kv)
		if not ok then return nil end
		only.log('I', data)
		local ret = http_api.http(srv, utils.post_data(api, srv, data))
		only.log('I', tostring(ret))
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
	WARNNING	= G_COLOR["YELLOW"],
	ERROR		= G_COLOR["RED"],
	SYSTEM		= G_COLOR["PURPLE"]
}

local KIND_LIST = {
	DEBUG		= 1,
	INFO		= 2,
	WARNNING	= 3,
	ERROR		= 4,
	SYSTEM		= 5
}

local function output_all( tone, kind, head, data )
	--> cut
	if #data > LOG_HUB_MAXLEN then
		data = string.sub(data, 1, LOG_HUB_MAXLEN) .. " [. . .]\n\n"
	end
	--> print
	if LOG_HUB_PRINTF and kind >= LOG_HUB_FILTER then
		os.execute(string.format('echo -ne "%s%s%s"', tone, head, data))
	end
	--> sms a full msg
	if LOG_HUB_NOTIFY and head ~= "" and kind >= 4 then
		sms_send( head .. data )
	end
end

function handle( args )
	-->> storage
	if LOG_HUB_APPEND then
		save_logs_list( args )
	end

	-->> sms or print
	if (not LOG_HUB_NOTIFY) and (not LOG_HUB_PRINTF) then
		return 0
	end

	local data = args[ #args ]
	local tone = G_COLOR["DEFAULT"]--v
	local kind = 0
	if kind == 0 then kind = 1 end--FIXME
	local head = ""
	repeat
		local st, ed, more, time, rank, info = string.find(data, "(.-)(%d%d%d%d%-%d%d%-%d%d %d%d%:%d%d%:%d%d) %[(%u+)%](.*)")
		if not st then break end

		if more then
			output_all( tone, kind, head, more )
		end

		tone = TONE_LIST[ rank ]
		kind = KIND_LIST[ rank ]
		if not tone or not kind then
			only.log('E', "ERROR LOG LEVEL " .. rank)
			tone = TONE_LIST["DEBUG"]
			kind = KIND_LIST["DEBUG"]
		end
		head = string.sub(data, st + #more, ed - #info)

		data = info
	until false
	output_all( tone, kind, head, data )
	return kind
end
