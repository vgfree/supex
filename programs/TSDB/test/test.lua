local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../../open/lib/?.lua;",
        "../../../open/apply/?.lua;",
        "../../../open/spxonly/?.lua;",
        "../../../open/linkup/?.lua;",
        "../../../open/public/?.lua;",
	"../../../open/lib/lua-coro/?.lua;",
        "open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local Coro 	= require("coro")

redis_api.init()

local function setData(key, data)
	print(key)
	local ok_status, ok_ret = redis_api.cmd('IdKey', '', 'SET', key, data)
        if not (ok_status and ok_ret) then
        	print('Get key failed')
		return nil
	end
end

local function getData(key)
	local ok_status, ok_ret = redis_api.cmd('IdKey', '', 'GET', key)
        if not (ok_status and ok_ret) then
        	print('Get key failed')
		return nil        
	end
	return ok_ret
end

local function forward_task_redis(coro, usr)
	assert(usr)
	local midKey = math.random(888888)
	local idle = coro.fastswitch
	redis_api.reg( idle, coro )
	for i = 0, 100000 do
		setData('hamster:' .. midKey .. ':'  .. tostring(i) .. ':' .. usr, 'GPS' .. tostring(i))
	end
end

local function self_cycle_idle( coro, idleable )
        if not idleable then
                only.log('E',"IDLE~~~~")
        else
                if coro:isactive() then
                        lua_default_switch(supex["__TASKER_SCHEME__"])
                else
                        only.log('D',"coro:stop()")
                        coro:stop()
                        return
                end
        end
end

function handle()
	print('begin to set data to tsdb!')
	local coro = Coro:open(true)
	coro:addtask(forward_task_redis, coro, 'user')
	if coro:startup(self_cycle_idle, coro, true) then
        	only.log('D',"Tasks execute success.")
        else
        	only.log('E',"Tasks execute failure.")
        end
        coro:close()
end

handle = handle()
