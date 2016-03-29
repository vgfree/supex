local http_api 		= require("http_short_api")
local cjson		= require('cjson')
local APP_CONFIG_LIST 	= require('CONFIG_LIST')
local only 		= require('only')
local link 		= require("link")
local scene 		= require("scene")
local utils 		= require("utils")
local supex 		= require("supex")
local tcp_api 		= require("tcp_pool_api")
local scan 		= require('scan')

module("WORK_FUNC_LIST", package.seeall)


-->> private
OWN_HINT = {
	-->> exact local whole
	app_task_forward = {},
}

OWN_ARGS = {
	-->> exact local whole
	app_task_forward = {
		app_uri = "p2p_xxxxxxxxxxx",
	},

}

function make_scene_data( app_name )
	local info = utils.deepcopy(supex.get_our_body_table())
	info["private_data"] = scene.view( app_name )
	local data = cjson.encode( info )
	return data
end

local function scene_forward(app_name,scene,IMEI)
	local data = {}
        local send_type = {}
        local app_type = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["app_task_forward"]["scene_forward"]["app_type"]
        send_type["imei"] = IMEI
        send_type["type"] = app_type
	local send_type_string = cjson.encode(send_type)
	data[1] = 1
	data[2] = send_type_string
	if type(scene) == "table" then
		scene = cjson.encode(scene)
	end
	data[3] = scene
        local ok ,data= utils.compose_mfptp_json_request(data)
        if not ok then
                only.log("E","scene_forward is fail")
        end
	local ok,result= tcp_api.cmd("tcp1",data)
	if not ok then
		only.log('E',"send data to scenceserver is fail because tcp is fail")
	end
end

function app_task_forward( app_name )
	local IMEI = supex.get_our_body_table()["IMEI"]
	local app_uri = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["app_task_forward"]["app_uri"]
	local path = string.gsub(app_uri, "?.*", "")
	local app_srv = link["OWN_DIED"]["http"][ path ]
	local scene = make_scene_data( app_name )
	-- pcall(scene_forward,app_name ,scene,IMEI)
	local data = utils.compose_http_json_request(app_srv, app_uri, nil, scene)
	local ok = http_api.http(app_srv, data, false)
	if not ok then
		only.log("E", "app_task_forward false")
	end
end

