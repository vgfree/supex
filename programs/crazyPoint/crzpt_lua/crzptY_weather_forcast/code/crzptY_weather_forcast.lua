local only = require('only')
local plan = require('plan')
local scan = require('scan')
local utils = require('utils')
local link = require('link')
local weibo = require('weibo')
local http_api = require("http_short_api")
local crzpt		= require('crzpt')


module('crzptY_weather_forcast', package.seeall)

--[[
local function main_entry( ... )
print("hello world!")
end
]]--


function origin()
	only.log("D", "init start ... .. .")-- how to mount task once?
end

function origin()
	only.log("D", "init start ... .. .")-- how to mount task once?
end


function lookup( jo )
	print("xxxx")
end

function accept( jo )
	--[[
	--> make lua
	local delay = 240
	local time = (os.time() + 8*60*60 + delay) % (24*60*60)
	local pidx = plan.make( nil, main_entry, time, true )
	--> mount C
	if pidx and crzpt["_FINAL_STAGE_"] then
	local ok = plan.mount( pidx )
	only.log("I", string.format("mount pidx %s %s", pidx, ok))
	end
	]]--

end

--[[
local function user_control(app_name,accountID)
	local accountID = accountID
	local ctl = weibo.DRI_APP_LIST[app_name]
	if not ctl then
		return true;
	end
	if not weibo.check_driview_subscribed_msg(accountID, ctl.no) then
		only.log('D',ctl.text)
		return false;
	end
	return true;
end
]]--



function handle( jo )

	print( scan.dump(jo) )
	local accountID = jo["ARGS"]["data"]["accountID"]
	local info_table = jo["ARGS"]["data"]
	if not info_table then
		only.log('E',"info_table is nil")
		return false
	end

	local path = "p2p_weather_forcast"
	local app_srv = link["OWN_DIED"]["http"][ path ]
  	local app_uri = "p2p_weather_forcast"
	local data = utils.compose_http_json_request(app_srv, app_uri, nil, info_table)
	http_api.http(app_srv, data, false)
end

