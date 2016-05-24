local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('weibo_recv_single_message', package.seeall)




function handle()
	local args = supex.get_our_body_table()
	local UID = args["UID"]
	local ok = nil
	local message = nil
	local label = nil 
	if UID then
		local redis_eval_command = string.format(" local tab = redis.call('zrange','%s:weiboPriority',0,0)  if tab and #tab > 0 then redis.call('zrem','%s:weiboPriority',tab[1])  return tab[1] end  return nil ", UID, UID)

		repeat
			ok, label = redis_api.hash_cmd('weibo_hash',UID, 'EVAL', redis_eval_command, 0)
			if not ok or not label then
				break
			end
			ok, message = redis_api.cmd('weibo',"" , 'GET', label .. ":weibo")
			if not ok then
				break
			end
		until message
		
		if message then
			local cur_time = os.time()
			only.log('S','time=%s,UID=%s,bizid=%s',cur_time,UID,label)
			local afp = supex.rgs(200)
			supex.say(afp, message)
			return supex.over(afp, "text/plain")
		else
			local afp = supex.rgs(404)
			return supex.over(afp)
		end
	end
end

