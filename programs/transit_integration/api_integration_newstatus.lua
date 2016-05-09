local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")

module("api_integration_newstatus", package.seeall)

local Head = {
	appkey		= '3710394550',
	sign            = '',
	accountId	= 'xxxxxxxxxx',
	timestamp       = '1445625467',
}

local function full_table_withType(str, mediaUrl, mediaType)
	local body = {
        	longitude = '0',
        	latitude  = '0',
        	direction = 0,
        	GPSTime   = 1460536084,
        	url       = 'nil',
        	mediatype = 'nil',
        	speed     = 0,
        	altitude  = '0',
        	model     = 'V141224_64',
	}
	
	local secret = 'B771BFCE2A1688023312514129ABCE9DACAB71C6'
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
		body['longitude'] = tostring(tab_info['RESULT']['content']['longitude'])
		body['latitude'] = tostring(tab_info['RESULT']['content']['latitude'])
	end
	if mediaType == 'amr' then
		body['longitude'] = tostring(tab_info['RESULT']['videoList'][1]['lngLatList'][1]['longitude'])
                body['latitude'] = tostring(tab_info['RESULT']['videoList'][1]['lngLatList'][1]['latitude'])
	end
	local media_url = utils.escape_redis_text(mediaUrl)
	body['url'] = tostring(media_url)
        body['mediatype'] = tostring(mediaType)
	body['speed'] = tonumber(tab_info['RESULT']['speed'])
        body['altitude'] = tostring(tab_info['RESULT']['content']['altitude'])
	body['direction'] = tab_info['RESULT']['content']['direction']
	body['model'] = tab_info['RESULT']['mod']
	body['GPSTime'] = os.time()
	Head['sign'] = utils.gen_sign(Head, secret)
	Head['accountId'] = tab_info['RESULT']['accountID']
	only.log('D','%s',scan.dump(body))
	return body
end

local function trafii_save_request(data)
	local trafiiServer = link["OWN_DIED"]["http"]["saveRtrPicBySgid"]
	local req_data = utils.compose_http_json_request({host = "mapapi.daoke.me", port = 80}, "rtrTraffic/saveRtrPicBySgid", Head, data)
	print(req_data)
        local ret = http_api.http(trafiiServer, req_data , true)
	print(ret)	
end

function traffi_gson_save(str, mediaUrl, mediaType)
	local jsonStr = string.gsub(str, 'json\r\n', '')
	local data = full_table_withType(jsonStr, mediaUrl, mediaType)
	trafii_save_request(data)
end

function dfs_image_save(str)
	local pstr = string.gsub(str, 'jpg\r\n', '')
	--Send picture to dfs server
	local dfsServer = link["OWN_DIED"]["http"]["dfsapi/v2/saveImage"]

	local file = {
		file_name = 'ClayMore.jpg',
		data = pstr,
		data_type = 'application/octet-stream',
	}

	local tab = {
		appKey = '3710394550',
		length = tostring(#pstr),
		sign = '711096F76AD42DD3E5E16B1657626AF8A385E7CD',
		isStorage = 'true',
		file_name = 'ClayMore.jpg',
		data = pstr,
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
                appKey = '2290837278',
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
