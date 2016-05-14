local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('weibo_recv_single_message', package.seeall)
local default_channel = '10086'

local function get_message(GID,UID)
	local umsgid = nil
	local gmsgid = nil
	local message = nil
	local date = os.date("%Y%m%d",os.time())
	local key_umsgid = string.format('umsgid:%s:%s:%s',date,GID,UID)
	ok, umsgid = redis_api.cmd('weibo', '', 'get', key_umsgid)

	local key_gmsgid = string.format('gmsgid:%s:%s',date,GID)
	ok, gmsgid = redis_api.cmd('weibo', "", 'get', key_gmsgid)
	only.log('S','xxxxxxxxxx GID=%s,UID=%s,umsgid=%s,gmsgid=%s',GID,UID,umsgid,gmsgid)
	if gmsgid then
		if umsgid then
			--umsgid >gmsgid have no message
			if tonumber(umsgid) > tonumber(gmsgid) then
				only.log('S','no message GID=%s,UID=%s,umsgid=%s,gmsgid=%s',GID,UID,umsgid,gmsgid)
			elseif umsgid == gmsgid then
				key_msg = string.format('msg:%s:%s:%s',date,GID,umsgid)
				ok, message = redis_api.hash_cmd('weibo_hash', key_msg, 'get', key_msg)
				only.log('S','get next messages GID=%s,UID=%s,key=%s,vaule=%s',GID,UID,key_msg,message)
				only.log('S','incr key=%s,vaule=%s',key_umsgid,umsgid)
				redis_api.cmd('weibo', "", 'incr', key_umsgid)
			else
				--umsgid <gmsgid have too many messages
				key_msg = string.format('msg:%s:%s:%s',date,GID,umsgid)
				ok, message = redis_api.hash_cmd('weibo_hash', key_msg, 'get', key_msg)
				if  message then
					only.log('S','too many messages no expire GID=%s,UID=%s,key=%s,vaule=%s',GID,UID,key_msg,message)
					redis_api.cmd('weibo', "", 'incr', key_umsgid)
				else
					--too many messages have expire

					umsgid = gmsgid
					key_msg = string.format('msg:%s:%s:%s',date,GID,umsgid)
					ok, message = redis_api.hash_cmd('weibo_hash', key_msg, 'get', key_msg)
					only.log('S','too many messages have expire GID=%s,UID=%s,key=%s,vaule=%s',GID,UID,key_msg,message)
					umsgid = gmsgid+1
					redis_api.cmd('weibo', '','set',key_umsgid,umsgid)
				end
			end
		else
			-- not umsgid first get latest message
			umsgid = gmsgid
			key_msg = string.format('msg:%s:%s:%s',date,GID,umsgid)
			ok, message = redis_api.hash_cmd('weibo_hash', key_msg, 'get', key_msg)
			only.log('S','first get latest GID=%s,UID=%s,key=%s,vaule=%s',GID,UID,key_msg,message)
			umsgid = gmsgid+1
			redis_api.cmd('weibo', '','setex',key_umsgid,'86400',umsgid)

		end
	end
	return message
end


function handle()

	local args = supex.get_our_body_table()
	local UID = args["UID"]
	if UID then
		local message = nil
		local GID = nil

		local redis_get_gid = string.format("\
		local tab = redis.call('smembers','%s:userFollowMicroChannel')\
		local groupVoice = redis.call('get','%s:currentChannel:groupVoice')\
		if groupVoice then\
			table.insert(tab,groupVoice)\
		end\
		local voiceCommand = redis.call('get','%s:currentChannel:voiceCommand')\
		if voiceCommand then\
			table.insert(tab,voiceCommand)\
		end\
		if tab then\
			return tab\
		end\
		return nil\
		", UID, UID,UID)


		local ok, tab = redis_api.hash_cmd('read_private','', 'EVAL', redis_get_gid, 0)
		if  #tab == 0 then
			message =  get_message(default_channel,UID)
		else
			for k,v in ipairs(tab) do
				only.log('S','key=%s,value=%s',k,v)
				GID = v
				message = get_message(GID,UID)
				if message then
					break
				end
			end
		end
		if message then
			local afp = supex.rgs(200)
			supex.say(afp, message)
			return supex.over(afp, "text/plain")
		end
	end
	local afp = supex.rgs(404)
	return supex.over(afp)
end

