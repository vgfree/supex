local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")

module("api_redis_save", package.seeall)

function handle_gson(str, mediaUrl, mediaType)
	return traffi_gson_save(str, mediaUrl, mediaType)
end
 
function handle_image(str)
	return dfs_image_save(str)
end
function handle_audio(str)
        return dfs_sound_save(str)
end
