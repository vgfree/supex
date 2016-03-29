local supex	= require('supex')
local only	= require('only')
local scan	= require('scan')
local redis_api	= require('redis_pool_api')

module('cachekv', package.seeall)

--[[
运行存储类REDIS & KV 命令
1.run('cmd', 'keyvalue1', 'keyvalue2', ...) 返回string/error
2.run({ 'cmd1', 'keyvaluse1', 'keyvaluse2', ... }, { 'cmd2', 'keyvaluse1', 'keyvaluse2', ... }, ...)
返回表 { {result, ...}, { result, ... }, ... , {result...}/error}
@return false/true and string/table
]]
function cmdset( server, hash, ... )
	local ok, result = redis_api.cmd(server, hash, ...)
	
	if ok then
		ok, result = pcall(supex.luakvcmd, ...)
	end

	if not ok then
		only.log("E", string.format(
			'cachekv.cmdset() failed ; ' ..
			'server : %s, command : %s, case : %s', 
			server, scan.dump({...}), scan.dump(result)))
	end
	return ok, result
end

--[[
运行获取类REDIS & KV 命令
]]
function cmdget( server, hash, ... )
	local ok, result = pcall(supex.luakvcmd, ...)

	if not ok or (not result or #result == 0) then
		ok, result = redis_api.cmd(server, hash, ...)
		if ok then
			if type(...) == 'table' then
				ok, result = false, 'not supported this operation.'
			else
				local cmd, key = ...
				local value = nil
				cmd = string.lower(cmd or ' ')
				if string.find(cmd, 'get') then
					ok, value = pcall(supex.luakvcmd, 'set', key or ' ', result)
				elseif string.find(cmd, 'smembers') then
					ok, value = pcall(supex.luakvcmd, 'sadd', key or ' ', unpack(result))
				else
					-- ok, value = false, 'not supported this operation.'
				end

				if not ok then
					result = value
				end
			end
		end
	end

	if not ok then
		only.log("E", string.format( 'cachekv.cmdget() failed ; ' .. 
			'server : %s, command : %s, case : %s', 
			server, scan.dump({...}), scan.dump(result)))
	end
	return ok, result
end

--[[
运行删除或运算类REDIS & KV 命令
]]
function cmd( server, hash, ... )
	
	local ok, result = pcall(supex.luakvcmd, ...)

	if not ok then
		only.log("E", string.format(
				'cachekv.cmd() failed ; ' ..
				'server : %s, command : %s, case : %s', 
				server, scan.dump({...}), scan.dump(result)))
		return ok, result
	end

	local ok1, result1 = redis_api.cmd(server, hash, ...)

	if not ok1 then
		only.log("E", string.format(
				'cachekv.cmd() failed ; ' ..
				'server : %s, command : %s, case : %s', 
				server, scan.dump({...}), scan.dump(result1)))
		-- return ok, result
	end

	return ok, result
end

