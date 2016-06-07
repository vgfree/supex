local only = require('only')
local redis_api = require('redis_pool_api')
local supex = require('supex')
local scan = require('scan')

module('api_dc_newstatus', package.seeall)


function handle()
	local data = supex.get_our_info_data()
	msg = string.sub(data,8)
	only.log('S','msg = %s', msg)
	local time = os.time()
	local key_log = string.format('newstatuslog:%s',os.date('%Y%m%d',time))
	local value =  string.format('%s  %s',os.date('%Y-%m-%d %H:%M:%S',time),msg)
	redis_api.cmd('newstatusRedis','','LPUSH', key_log, value)
end

