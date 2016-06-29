local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../open/lib/?.lua;",
        "../../open/apply/?.lua;",
        "../../open/spxonly/?.lua;",
        "../../open/linkup/?.lua;",
        "../../open/public/?.lua;",
        "open/?.lua;",
}

local CFG_LIST          = require('cfg')

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

local utils     = require('utils')
local only      = require('only')
local cjson     = require('cjson')
local redis_api = require('redis_pool_api')
local http_api  = require('http_short_api')
local libhttps  = require("libhttps")
local math	= require("math")
--local zk	= require("zklua")
local zk_api 	= require ('tsdb_zk_api')
local zkhost 	= link["OWN_ZOOKEEPER"]["zkhost"]

local zkhandle = nil

local function add_dn(ctx, dn)
        only.log('E', string.format('[ADD] ip:%s wport:%d rport:%d role:%s', dn["ip"], dn["w_port"], dn["r_port"], dn["role"]))
        redis_api.add_to_pool(dn["ip"] .. ":" .. dn["r_port"], dn["ip"], dn["r_port"])
end

local function del_dn(ctx, dn)
	only.log('E', string.format('[DEL] ip:%s wport:%d rport:%d role:%s', dn["ip"], dn["w_port"], dn["r_port"], dn["role"]))
	--redis_api.del_from_pool(dn["ip"] .. ":" .. dn["r_port"])
end

function init()
	only.log("E", "enter init")
	zkhandle = zk_api.open_zkhandler(zkhost, add_dn, del_dn)
	zk_api.register_zkcache(zkhandle)
end

function handle()
	only.log("E", "timport conf = %s", scan.dump(CFG_LIST['timport']))
	init()
	local time_field = tonumber(os.date("%Y%m%d%H%M%S", os.time()))
        local key = 6145

	local ok, ds = zk_api.get_read_dataset(zkhandle, key, time_field)
        only.log("E", "time_field = %d, key = %d", time_field, key)
        if not ok then
                only.log("E", "get_read_dataset failed")
                return "nil"
        end

	only.log('E', 'ok = %s', scan.dump(ok))
	only.log('E', 'ip = %s', ds.data_node[ds.dn_idx].ip)
	only.log('E', 'wport = %s', ds.data_node[ds.dn_idx].w_port)
end

handle = handle()
