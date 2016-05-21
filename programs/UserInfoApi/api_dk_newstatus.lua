local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local zmq	= require("zmq")
local api_redis_save = require("api_redis_save")

module("api_dk_newstatus", package.seeall)

local ctx = zmq.init(1)
local s = ctx:socket(zmq.PUSH)

local function parseFirstFrame(table)
	if table[1] == 'status' then	
		api_redis_save.loginServerInfoSave(table)
	end
	if table[1] == 'setting' then
		api_redis_save.appServerInfoSave(table)
	end
end

local function sendToSettingServer(table)
	s:connect("tcp://localhost:5559")
	s:send_table(table)
end

function handle(table)
	parseFirstFrame(table)
	sendToSettingServer(table)
end
