local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')

module("api_pic_newstatus", package.seeall)

local Head = {
	appkey		= '',
	accountid	= '',
	timestamp	= '',
	sign		= '',
}

local Body = {
	longitude	= '',
	latitude	= '',
	direction	= '',
	GPSTime		= '',
	url		= '',
	mediatype	= '',
	speed		= '',
	altitude	= '',
	model		= '',
}

function full_table_withType(str, mediaUrl, mediaType)
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
		print(Body.longitude)
		Body.latitude = tab_info['RESULT']['mediaList'][1]['latitude']
		print(Body.latitude)
	end
	if mediaType == 'amr' then
		Body.longitude = tab_info['RESULT']['videoList'][1]['lngLatList'][1]['longitude']
                print(Body.longitude)
                Body.latitude = tab_info['RESULT']['videoList'][1]['lngLatList'][1]['latitude']
		print(Body.latitude)
	end
	Body.url = mediaUrl
	print(Body.url)
        Body.mediatype = mediaType
	print(Body.mediatype)
	Body.speed = tab_info['RESULT']['speed']
	print(Body.speed)
        Body.altitude = tab_info['RESULT']['altitude']
	print(Body.altitude)
end

function traffi_gson_save(str, mediaUrl, mediaType)
	full_table_withType(str, mediaUrl, mediaType)
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
	return traffi_gson_save(str, mediaUrl, mediaType)
end
 
function handle_image(str)
	return dfs_image_save(str)
end
function handle_audio(str)
        return dfs_sound_save(str)
end
