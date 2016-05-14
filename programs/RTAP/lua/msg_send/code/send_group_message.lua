local only = require('only')
local redis_api = require('redis_pool_api')
local supex = require('supex')
local Coro = require("coro")
local luakv_api = require('luakv_pool_api')
local scan = require('scan')

module('weibo_send_group_message', package.seeall)



function handle()
	local args = supex.get_our_body_table()
	local GID = args["GID"]
	if GID then
			local date = os.date("%Y%m%d",os.time())
			local key = string.format('gmsgid:%s:%s',date,GID) 
			local ok, msgid = redis_api.cmd('weibo', "", 'get', key)
			if not msgid then
				redis_api.cmd('weibo', '', 'setex', key, '86400','0')
			end
			ok,msgid = redis_api.cmd('weibo', "", 'incr', key)
			key = string.format('msg:%s:%s:%s',date,GID,msgid) 
			only.log('S','key:%s,value is %s',key,args['message'])
			local ok, info = redis_api.hash_cmd('weibo_hash', key, 'setex', key, '300',args['message'])
	end
end

