local cutils    = require('cutils')
local utils    	= require('utils')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local supex     = require('supex')
local scan      = require('scan')
local gosay     = require('gosay')
local cjson     = require('cjson')
local Coro	= require("coro");
local lhttp_api = require('lhttp_pool_api')

module('api_data_forwarding', package.seeall)

local tasks = {
	{ id = 1, lhttp = "lhttp1", path = "/spx_txt_to_voice"},
	{ id = 2, lhttp = "lhttp2", path = "/spx_txt_to_voice"},
}

local ret_tab = {}

local function work( coro, args )
	
	print("send data")
	local server = link['OWN_POOL']['lhttp'][args.usr.lhttp]
	local data = utils.compose_http_form_request(server, args.usr.path, nil, args, nil, nil)	
	local idle = coro.fastswitch
	lhttp_api.reg( idle, coro )
	local ok, info = lhttp_api.cmd(args.usr.lhttp, "", "origin", data)
	print(ok, info["data"])
	print("recv data")

	table.insert(ret_tab,info['data'])
        return ok
end



local function idle( coro, idleable )
	if not idleable then 
		print("\x1B[1;35m".."IDLE~~~~".."\x1B[m");
	else
		--coro:fastswitch();
		if not coro:isactive() then
			print("\x1B[1;33m".."coro:stop()".."\x1B[m");
			coro:stop();
			return
		end
	end
end

local function forward_task(args)
	local coro = Coro:open(true)
	for i=1,#tasks do
		args["usr"] = tasks[i]
		coro:addtask(work, coro, args)
	end

        if coro:startup(idle, coro, true) then
                print("\x1B[1;32m".."Tasks execute success.".."\x1B[m")
        else
                print("\x1B[1;32m".."Tasks execute failure.".."\x1B[m")
        end
        coro:close()
end

function handle()
	local args = supex.get_our_body_table()

	if utils.get_sha_tab_count(args) == 0 then
		args = supex.get_our_uri_table()
	end
	
	if not args then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"],"args is nil")
	end

	forward_task(args)
	
	
	local ok, ret_url = pcall(cjson.encode, ret_tab)
	ret_url = string.gsub(ret_url, "\\", "")
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"],ret_url)
end
