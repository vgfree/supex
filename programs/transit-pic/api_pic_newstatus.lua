local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local socket    = require('socket')
local mysql_api = require('mysql_pool_api')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')

module("api_pic_newstatus", package.seeall)

function dfs_image_save( str )
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
  print("ret = %s", ret)
  return ret
end

function handle( str )
  str = string.gsub(str,"Head=picture&data=","")
  local ret = dfs_image_save(str)
  return ret
  
end
