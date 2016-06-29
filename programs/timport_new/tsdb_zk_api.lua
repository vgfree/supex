local zklua = require ('zklua')
local only  = require ('only')

module("tsdb_zk_api", package.seeall)

local function zk_watcher(zh, type, state, path, watcherctx)
	if type == zklua.ZOO_SESSION_EVENT then
		if state == zklua.ZOO_CONNECTED_STATE then
			only.log('E', 'Connected to zookeeper service successfully!');
		 elseif (state == zklua.ZOO_EXPIRED_SESSION_STATE) then
			only.log('E', 'Zookeeper session expired!');
		end
	end
end

local function tsdb_watcher(zh, type, state, path, watcherctx)
	if state == zklua.ZOO_CONNECTED_STATE then
		if type == zklua.ZOO_CHILD_EVENT then
			unregister_zkcache(watcherctx)
			register_zkcache(watcherctx)
		end
	end
end

local function expansion_watcher(zh, type, state, path, watcherctx)
	if state == zklua.ZOO_CONNECTED_STATE then
		if type == zklua.ZOO_CHILD_EVENT then
			unregister_expansion_zkcache(watcherctx)
			register_expansion_zkcache(zh, watcherctx)
		end
	end
end

local function keyset_watcher(zh, type, state, path, watcherctx)
	if state == zklua.ZOO_CONNECTED_STATE then
		if type == zklua.ZOO_CHILD_EVENT then
			unregister_keyset_zkcache(watcherctx)
			register_keyset_zkcache(zh, path, watcherctx)
		end
	end
end

function open_zkhandler(zkhost, alloc_dn_hook, free_dn_hook, ctx)
	local zc = {}
	
	zklua.set_debug_level(zklua.ZOO_LOG_LEVEL_ERROR)
	zc.zkhandle = zklua.init(zkhost, zk_watcher, 30000, nil, zc)
	if not zc.zkhandle then
		return nil
	end

	zc.name = name
	zc.alloc_dn_hook = alloc_dn_hook
	zc.free_dn_hook = free_dn_hook
	zc.ctx = ctx
	zc.expansion_set = {}

	return zc
end


function register_zkcache(zc)
	if #zc.expansion_set ~= 0 then
		only.log("E", "register_zkcache before true!!!")
		return true
	end

	ret,sv = zklua.wget_children(zc.zkhandle, "/tsdb", tsdb_watcher, zc)
	if ret ~= zklua.ZOK or not sv then
		only.log("E", "zklua.wget_children failed !!!")
		return false
	end

	for index, value in ipairs(sv) do
		zc.expansion_set[index] = {}
		zc.expansion_set[index].path = "/tsdb/" .. value
		zc.expansion_set[index].key_set = {}
		ret = parse_expansion_node(value, zc.expansion_set[index])
		if not ret then
			return false
		end
		
		zc.expansion_set[index].parent = zc
		ret = register_expansion_zkcache(zc.zkhandle, zc.expansion_set[index])
		if not ret then
			return false
		end

		if zc.expansion_set[index].mode == "RW" and index > 1 then
			tmp = zc.expansion_set[index]
			zc.expansion_set[index] = zc.expansion_set[1]
			zc.expansion_set[1] = tmp
		end
	end

	only.log("E", "register_zkcache before true!!!")
	return true
end

function unregister_zkcache(zc)
	if zc.free_dn_hook then
		for index, es in ipairs(zc.expansion_set) do
			unregister_expansion_zkcache(es)
		end
	end
	
	zc.expansion_set = {};
end

function register_expansion_zkcache(zkhandle, expansion_set)
	if #expansion_set.key_set ~= 0 then
		return true
	end
	
	ret, sv = zklua.wget_children(zkhandle, expansion_set.path, expansion_watcher, expansion_set)
	if ret ~= zklua.ZOK or not sv then
		return false
	end
 
	for index, value in ipairs(sv) do
		expansion_set.key_set[index] = {}
		expansion_set.key_set[index].data_set = {};
		expansion_set.key_set[index].path = expansion_set.path .. "/" .. value
		
		ret = parse_keyset_node(value, expansion_set.key_set[index])
		if not ret then
			return false
		end
		
		expansion_set.key_set[index].parent = expansion_set

		ret = register_keyset_zkcache(zkhandle, expansion_set.key_set[index].path, expansion_set.key_set[index])
		if not ret then
			return false
		end
		expansion_set.key_set[index].is_synced = true

	end

	return true
end

function unregister_expansion_zkcache(expansion_set)
	local zc = expansion_set.parent
	if zc.free_dn_hook then
		for index, ks in ipairs(expansion_set.key_set) do
			unregister_keyset_zkcache(ks)
		end
	end
	
	expansion_set.key_set = {}
end

function register_keyset_zkcache(zkhandle, path, key_set)
	if key_set.is_synced then
		return true
	end
	
	key_set.data_set = {}
	
	ret, sv = zklua.wget_children(zkhandle, path, keyset_watcher, key_set)
	if ret ~= zklua.ZOK or not sv then
		return false
	end

	local zc = key_set.parent.parent
	
	for index, value in ipairs(sv) do
		ret, tmp_ds = parse_datanode(value)
		if not ret then
			return false
		end
		
		local is_hited = false
		
		for index, data_set in ipairs(key_set.data_set) do
			if tmp_ds.id == data_set.id then
				offset = #data_set.data_node + 1
				data_set.data_node[offset] = {}
				data_set.data_node[offset].ip = tmp_ds.ip
				data_set.data_node[offset].w_port = tmp_ds.w_port
				data_set.data_node[offset].r_port = tmp_ds.r_port
				data_set.data_node[offset].role = tmp_ds.role
				data_set.data_node[offset].parent = data_set
				
				if zc.alloc_dn_hook then
					zc.alloc_dn_hook(zc.ctx, data_set.data_node[offset])
				end
				
				is_hited = true
				
				break
			end
		end
		
		if not is_hited then
			offset = #key_set.data_set + 1
			key_set.data_set[offset] = {}
			key_set.data_set[offset].data_node = {}
			key_set.data_set[offset].data_node[1] = {}
			key_set.data_set[offset].data_node[1].ip = tmp_ds.ip
			key_set.data_set[offset].data_node[1].w_port = tmp_ds.w_port
			key_set.data_set[offset].data_node[1].r_port = tmp_ds.r_port
			key_set.data_set[offset].data_node[1].role = tmp_ds.role
			key_set.data_set[offset].data_node[1].parent = key_set.data_set[offset]
			key_set.data_set[offset].id = tmp_ds.id
			key_set.data_set[offset].mode = tmp_ds.mode
			key_set.data_set[offset].s_time = tmp_ds.s_time
			key_set.data_set[offset].e_time = tmp_ds.e_time
			key_set.data_set[offset].dn_idx = 1
			if zc.alloc_dn_hook then
				zc.alloc_dn_hook(zc.ctx, key_set.data_set[offset].data_node[1])
			end
		end
	end
	
	return true
end

function unregister_keyset_zkcache(key_set)
	local zc = key_set.parent.parent
	if zc.free_dn_hook then
		for index, data_set in ipairs(key_set.data_set) do
			for index, dn in ipairs(data_set.data_node) do
				zc.free_dn_hook(zc.ctx, dn)
			end
		end
	end
	
	key_set.is_synced = false;
end

function parse_datanode(node)
	local dn = {}
	
	mode, id, role, ip, w_port, r_port, s_time, e_time = string.match(node, "(R[OW]):(%d+):(%a+):(%d+%.%d+%.%d+%.%d+):(%d+):(%d+):(%d+):(%-?%d+)")
	if not mode or not id or not role or not ip or not w_port or not r_port or not s_time or not e_time then
		return false, nil
	end

	dn.mode = mode
	dn.id = tonumber(id)
	dn.role = role
	dn.ip = ip
	dn.w_port = w_port
	dn.r_port = r_port
	dn.s_time = tonumber(s_time)
	dn.e_time = e_time == "-1" and 100000000000000 or tonumber(e_time)
	
	return true, dn
end

function parse_keyset_node(node, key_set)
	s_key, e_key = string.match(node, "(%d+):(%d+)")
	if not s_key or not e_key then
		return false
	end

	key_set.s_key = tonumber(s_key)
	key_set.e_key = tonumber(e_key)
	
	return true
end

function parse_expansion_node(node, expansion_set)
	mode, s_time, e_time = string.match(node, "(%a+):(%d+):(%-?%d+)")
	if not mode or not s_time or not e_time then
		return false
	end
	expansion_set.mode = mode
	expansion_set.s_time = tonumber(s_time)
	expansion_set.e_time = e_time == "-1" and 100000000000000 or tonumber(e_time)
	return true
end

function get_write_dataset(zc, key_field)
	if not zc or #zc.expansion_set == 0 then
		return false
	end
	
	local es = zc.expansion_set[1]
	if not es or es.mode ~= "RW" or #es.key_set == 0 then
		return false
	end
	
	for index, ks in ipairs(es.key_set) do
		if not ks.data_set then
			return false
		end
		if key_field >= ks.s_key and key_field < ks.e_key then
			if ks.data_set[0] and ks.data_set[0].mode ~= "RW" then
				return false
			end
			
			return true, ks.data_set[0]
		end
	end
	
	return false
end

function get_read_dataset(zc, key_field, time_field)
	if not zc or #zc.expansion_set == 0 then
		only.log("E", "get_read_dataset failed !!!, #zc.expansion_set = %d", #zc.expansion_set)
		return false
	end
	
	local expansion_set = nil
	for index, es in ipairs(zc.expansion_set) do
		if time_field >= es.s_time and time_field < es.e_time then
			expansion_set = es
			break
		end
	end
	
	if not expansion_set or #expansion_set.key_set == 0 then
		only.log("E", "get_read_dataset failed !!!")
		return false
	end
	

	for index, ks in ipairs(expansion_set.key_set) do
		repeat
			if not ks.data_set or #ks.data_set == 0 then
				break
			end
			
			if key_field >= ks.s_key and key_field < ks.e_key then
				return true, ks.data_set[1]
			end
			
			break
		until true
	end
	only.log("E", "get_read_dataset failed !!!")
	return false
end
