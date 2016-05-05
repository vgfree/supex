local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")

module("api_integration_newstatus", package.seeall)

local Head = {
	appkey		= '2582535051',
	sign            = '4480DC5A12F77ECCC0E52D9A94374EAFC46ADAE2',
	accountId	= 'xxxxxxxxxx',
	timestamp       = '1445625467',
}


local hello = {
	longitude = '121.351609',
	latitude  = '31.220259',
	direction = 95,
	GPSTime   = 1460536084,
	url	  = 'http://image.xinmin.cn/2016/04/13/20160413084847727.jpg',
	mediatype = 'jpg',
	speed	  = 30,
	altitude  = '20.1',
	model	  = 'V141224_64',
}

local function full_table_withType(str, mediaUrl, mediaType)
	local Body = {
		longitude = '',
		latitude  = '',
		direction = 95,
		GPSTime	  = 1460536084,
		url	  = '',
		mediatype = '',
		speed	  = '',
		altitude  = '',
		model	  = 'V141224_64',
	}
	local ok , tab_info = pcall(cjson.decode, str)
        if not ok then
                print("cjson decode failed")
                return false
        end
        only.log('D',scan.dump(tab_info))
        local errorCode = tab_info['ERRORCODE']
        if errorCode ~= '0' then
                print("errorCode is %s", errorCode)
                return false
        end

	if mediaType == 'jpg' then
		Body.longitude = tab_info['RESULT']['mediaList'][1]['longitude']
		Body.latitude = tab_info['RESULT']['mediaList'][1]['latitude']
	end
	if mediaType == 'amr' then
		Body.longitude = tab_info['RESULT']['videoList'][1]['lngLatList'][1]['longitude']
                Body.latitude = tab_info['RESULT']['videoList'][1]['lngLatList'][1]['latitude']
	end
	Body.url = mediaUrl
        Body.mediatype = mediaType
	Body.speed = tab_info['RESULT']['speed']
        Body.altitude = tab_info['RESULT']['altitude']
	only.log('D', scan.dump(Body))
	for k,v in pairs(Body) do
		print(v)
	end
	return Body
end

local function trafii_save_request( data )
	--Head['Content-Type'] = 'application/x-www-form-urlencoded;charset=utf-8'
	--Head['User-Agent'] = 'curl/7.40.0'
	--Head['Accept'] =  '*/*'
	local trafiiServer = link["OWN_DIED"]["http"]["saveRtrPicBySgid"]
	local req_data = utils.compose_http_json_request({host = "mapapi.daoke.me"}, "rtrTraffic/saveRtrPicBySgid", Head, data)
	print(req_data)
        --local ret = libhttps.https("mapapi.daoke.me", 443, req_data, string.len(req_data))
        local ret = http_api.http(trafiiServer, req_data , true)
        --local ok, ret = supex_http_api('mapapi.daoke.me', '80', data, #data)
	print(ret)	
end

function traffi_gson_save(str, mediaUrl, mediaType)
	local data = full_table_withType(str, mediaUrl, mediaType)
	trafii_save_request( data )
end

function dfs_image_save(str)
	--Send picture to dfs server
	local dfsServer = link["OWN_DIED"]["http"]["dfsapi/v2/saveImage"]

	local file = {
		file_name = 'ClayMore.jpg',
		data = str,
		data_type = 'application/octet-stream',
	}

	local tab = {
		appKey = '2290837278',
		length = tostring(#str),
		sign = '711096F76AD42DD3E5E16B1657626AF8A385E7CD',
		isStorage = 'true',
		file_name = 'ClayMore.jpg',
		data = str,
	}

	local req_data = utils.compose_http_form_request(dfsServer, 'dfsapi/v2/saveImage', nil, tab, "mmfile", file)
	local ret = http_api.http(dfsServer, req_data , true)
	if not ret then
		print("[FAILED] HTTP post failed! feedback api")
	end

	local ret_str = string.match(ret, '{.+}')
	local ok , ret_info = pcall(cjson.decode, ret_str)
	if not ok then
                print("cjson decode failed")
                return false
        end
	--print(ret_str)
	--only.log('D',scan.dump(ret_info))
	return ret_info['RESULT']['url']
end

function dfs_sound_save(str)
	--Send voice to dfs server
	local dfsServer = link["OWN_DIED"]["http"]["dfsapi/v2/saveSound"]
	
        local file = {
                file_name = 'testsound.amr',
                data = str,
                data_type = 'application/octet-stream',
        }

	local tab = {
                appKey = '2973785773',
                length = tostring(#str),
                sign = '711096F76AD42DD3E5E16B1657626AF8A385E7CD',
                isStorage = 'true',
                file_name = 'testsound.amr',
                data = str,
        }
	
	local req_data = utils.compose_http_form_request(dfsServer, 'dfsapi/v2/saveSound', nil, tab, "mmfile", file)
	print("enter dfs_sound_save, req_data");
        local ret = http_api.http(dfsServer, req_data , true)
	if not ret then
		print("[FAILED] HTTP post failed! feedback api")
        end
        print("ret = %s", ret)
        return ret
end

function handle_gson(str, mediaUrl, mediaType)
	print(imageURL)
	return traffi_gson_save(str, mediaUrl, mediaType)
end
 
function handle_image(str)
	return dfs_image_save(str)
end
function handle_audio(str)
        return dfs_sound_save(str)
end
