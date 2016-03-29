local only              = require('only')
local json		= require('cjson')
local supex		= require('supex')
local link     	 	= require('link')
local http_short  	= require("http_short_api")
local luakv_api		= require("luakv_pool_api")
local msg		= require('msg')
local PUBLIC_FUNC_LIST  = require('PUBLIC_FUNC_LIST')

module('publicentry', package.seeall)

--功	能：判断是否在格网里
--参	数：accountID,grid：格网
--返 回 值：满足返回true和grid对应的值,否则返回false
function is_in_grid(accountID, grid)
	local ok, value = luakv_api.cmd('FiveThousandths',accountID, 'hgetall', grid)
	if not ok then
        	only.log("E", "can't get code from luakv!")
              	return false
        end
	if not value then
		only.log("D", "not get grid")
		return false
	else
		return true,value
	end
end

--功	能：判断是否超过一个小时
--参	数：accountID
--返 回 值：满足返回true,否则返回false
function over_one_hour(accountID)
	local ok, time = luakv_api.cmd('owner', accountID, 'get', accountID .. ":time")
	if not ok or not time then
		return true
	elseif os.time() - tonumber(time) > 60*60 then
		return true
	else
		return false
	end
end

function compose_http_json_request(serv, path, header, args)
        -->set headers
        local str_headers
        local def_headers = 'Content-Type: application/x-www-form-urlencoded\r\n'
        local tab_headers = {}
        if type(header) == "table" then
                if not header["Content-Type"] then
                        header["Content-Type"] = "application/x-www-form-urlencoded"
                end
                for k,v in pairs(header) do
                        table.insert(tab_headers, k .. ": " .. v .. "\r\n")
                end
                if #tab_headers > 0 then
                        str_headers = table.concat(tab_headers)
                end
        end

        -->set request
        local http_format = 'POST /%s HTTP/1.0\r\n' ..
        'Host: %s%s\r\n' ..
        '%s' ..
        'Content-Length: %d\r\n\r\n%s'

        if type(args) == "table" then
                local ok, info = pcall(cjson.encode, args)
                if not ok then
                        only.log('E', info)
                        return nil
                end

                return string.format(http_format, path, serv["host"], serv["port"] and (":" .. serv["port"]) or "", str_headers or def_headers, #info, info)
        else
                return string.format(http_format, path, serv["host"], serv["port"] and (":" .. serv["port"]) or "", str_headers or def_headers, #args, args)
        end
end

function handle()
	local collect	= supex.get_our_body_table()["collect"]
	if collect ~= true then
		local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_ERROR_COLLECT"])
		PUBLIC_FUNC_LIST.send_respond(str)
		return false
	end

	local accountID = supex.get_our_body_table()["accountID"]
	local imei 	= supex.get_our_body_table()["IMEI"] 
	local longitude = supex.get_our_body_table()["longitude"][1] 
	local latitude 	= supex.get_our_body_table()["latitude"][1]

        if not longitude or not latitude then
                only.log('E',"grid is nil")
		local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_ERROR_GRID"])
		PUBLIC_FUNC_LIST.send_respond(str)
        	return false
        end
	local lon, lat = PUBLIC_FUNC_LIST.grid_five_thousandths(longitude, latitude)
	local grid = string.format('%d&%d',tonumber(lon),tonumber(lat))	
	only.log('D', grid)
	local ok, value = is_in_grid(accountID, grid)

	if not ok or not value then
		local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_ERROR_IN_GRID"])
		PUBLIC_FUNC_LIST.send_respond(str)
		return false
	end

	if not over_one_hour(accountID) then
		local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_ERROR_TIME"])
		PUBLIC_FUNC_LIST.send_respond(str)
		return false
	end
	-->触发
	luakv_api.cmd('owner',accountID, 'set', accountID .. ":time", os.time())		
	local body_data = {}
	local pointList = {}
	for k, v in pairs(value) do
		local tb = {
			gridid    = k,
			gridtype  = v
		}
		table.insert(pointList, tb)
	end

	body_data["accountID"] = accountID
	body_data["imei"]      = imei
	body_data["point_list"] = pointList
	local grid_data = json.encode(body_data)
	grid_data = 'deviceList=' .. grid_data
	only.log('D', grid_data)

	local app_server = link["OWN_DIED"]["http"]["club/appointmentPhoto/getNearDeviceOfPoint"]
	local data = compose_http_json_request(app_server, "club/appointmentPhoto/getNearDeviceOfPoint", nil, grid_data)
	only.log('D',data)
	local ok_status = http_short.http(app_server, data, false)
	only.log('D',ok_status)
        if not ok_status then
              	only.log('D',string.format("post data %s:%s failed!",app_server.host,app_server.port))
		local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_DO_HTTP_FAILED"])
		PUBLIC_FUNC_LIST.send_respond(str)
		return false
        end 

	local str = PUBLIC_FUNC_LIST.to_json(msg["MSG_SUCCESS"])
	PUBLIC_FUNC_LIST.send_respond(str)
	return true

end
