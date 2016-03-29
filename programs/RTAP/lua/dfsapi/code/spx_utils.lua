local utils     = require('utils')
local only      = require('only')
local redis_api = require("redis_pool_api")

module("spx_utils", package.seeall)

function set_to_dfsdb(key, value, cacheTime)
	local st, ed, type_val = string.find(key, "%.(%a%a.)$")
	if type_val == 'amr' then
		return redis_api.hash_cmd("dfsdb_amr_write", key, "set",key,value)
--		if cacheTime then
--			return redis_api.hash_cmd("dfsdb_amr_write","expire",key,cacheTime)
--		end
	elseif type_val == 'mp4' then
		return redis_api.hash_cmd("dfsdb_mp4_write", key, "set",key,value)
--		if cacheTime then
--			return redis_api.hash_cmd("dfsdb_mp4_write","expire",key,cacheTime)
--		end
	elseif type_val == 'jpg' then
		return redis_api.hash_cmd("dfsdb_jpg_write", key, "set",key,value)
--		if cacheTime then
--			return redis_api.hash_cmd("dfsdb_jpg_write","expire",key,cacheTime)
--		end
	else
		return redis_api.hash_cmd("dfsdb_file_write", key, "set",key,value)
	end
	
end 

function set_to_redis(key, value, cacheTime)
	local st, ed, type_val = string.find(key, "%.(%a%a.)$")
	if type_val == 'amr' then
		if cacheTime then
			return redis_api.hash_cmd("redis_amr_write", key, "setex",key,cacheTime,value)	
		else
			return redis_api.hash_cmd("redis_amr_write", key, "set",key,value)	
		end
	elseif type_val == 'mp4' then
		if cacheTime then
			return redis_api.hash_cmd("redis_mp4_write", key, "setex",key,cacheTime,value)	
		else
			return redis_api.hash_cmd("redis_mp4_write", key, "set",key,value)	
		end
	elseif type_val == 'jpg' then
		if cacheTime then
			return redis_api.hash_cmd("redis_jpg_write", key, "setex",key,cacheTime,value)	
		else
			return redis_api.hash_cmd("redis_jpg_write", key, "set",key,value)	
		end
	else
		if cacheTime then
			return redis_api.hash_cmd("redis_file_write", key, "setex",key,cacheTime,value)	
		else
			return redis_api.hash_cmd("redis_file_write", key, "set",key,value)	
		end
	end
	
end 

function get_from_dfsdb(key, group)
	local st, ed, type_val = string.find(key, "%.(%a%a.)$")
	if type_val == 'amr' then
		return redis_api.just_cmd("dfsdb_amr_read", group, "get", key)
	elseif type_val == 'mp4' then
		return redis_api.just_cmd("dfsdb_mp4_read", group, "get", key)
	elseif type_val == 'jpg' then
		return redis_api.just_cmd("dfsdb_jpg_read", group, "get", key)
	else
		return redis_api.just_cmd("dfsdb_file_read", group, "get", key)
	end
end 

function get_from_redis(key, group)
	local st, ed, type_val = string.find(key, "%.(%a%a.)$")
	if type_val == 'amr' then
		return redis_api.just_cmd("redis_amr_read", group, "get", key)
	elseif type_val == 'mp4' then
		return redis_api.just_cmd("redis_mp4_read", group, "get", key)
	elseif type_val == 'jpg' then
		return redis_api.just_cmd("redis_jpg_read", group, "get", key)
	else
		return redis_api.just_cmd("redis_file_read", group, "get", key)
	end
end 
function set_to_dfsdb_spare(key,value,cacheTime)
	return redis_api.hash_cmd("dfsdb_write_spare", key, "set",key,value)		
--	if cacheTime then
--		return redis_api.hash_cmd("dfsdb_write_spare","expire",key,cacheTime)
--	end
end 

function set_to_redis_spare(key,value,cacheTime)
	if cacheTime then
		return redis_api.hash_cmd("redis_write_spare", key, "setex",key,cacheTime,value)	
	else
		return redis_api.hash_cmd("redis_write_spare", key, "set",key,value)	
	end
end
 
function get_from_dfsdb_spare(key, group)
	return redis_api.just_cmd("dfsdb_read_spare", group, "get", key)
end 

function get_from_redis_spare(key, group)
	return redis_api.just_cmd("redis_read_spare", group, "get", key)
end 
