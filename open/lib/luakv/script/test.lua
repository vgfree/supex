package.cpath = "../lib/?.so;./lib/?.so;../?.so;" .. package.cpath
local luakv 		= require('luakv')

function dump(obj)
	local getIndent, quoteStr, wrapKey, wrapVal, dumpObj
	getIndent = function(level)
		return string.rep("\t", level)
	end
	quoteStr = function(str)
		return '"' .. string.gsub(str, '"', '\\"') .. '"'
	end
	wrapKey = function(val)
		if type(val) == "number" then
			return "[" .. val .. "]"
		elseif type(val) == "string" then
			return "[" .. quoteStr(val) .. "]"
		else
			return "[" .. tostring(val) .. "]"
		end
	end
	wrapVal = function(val, level)
		if type(val) == "table" then
			return dumpObj(val, level)
		elseif type(val) == "number" then
			return val
		elseif type(val) == "string" then
			return quoteStr(val)
		else
			return tostring(val)
		end
	end
	dumpObj = function(obj, level)
		if type(obj) ~= "table" then
			return wrapVal(obj)
		end
		level = level + 1
		local tokens = {}
		tokens[#tokens + 1] = "{"
		for k, v in pairs(obj) do
			tokens[#tokens + 1] = getIndent(level) .. wrapKey(k) .. " = " .. wrapVal(v, level) .. ","
		end
		tokens[#tokens + 1] = getIndent(level - 1) .. "}"
		return table.concat(tokens, "\n")
	end
	return dumpObj(obj, 0)
end

local log = {
	writefull = print
}
local scan = {
	dump = dump
}

local function entry_cmd( kvhdl, ... )
        local transresult = {
                _tonumber_ = function ( result )
                        return tonumber(result[1])
                end,

                _boolean_ = function ( result )
                        return result and true or false
                end,

                _ontrans_ = function ( result )
                        return result
                end,

                _first_ = function ( result )
                        return result[1]
                end,
        };

        local transresult = setmetatable({
                -- 以下命令结果被转换成数字
                sadd            = transresult._tonumber_,
                del             = transresult._tonumber_,
                dbsize          = transresult._tonumber_,
                incr            = transresult._tonumber_,
                incrby          = transresult._tonumber_,
                decr            = transresult._tonumber_,
                decrby          = transresult._tonumber_,
                lpush           = transresult._tonumber_,
                expire          = transresult._tonumber_,
                expireat        = transresult._tonumber_,
                pexpire         = transresult._tonumber_,
                pexpireat       = transresult._tonumber_,
                sismember       = transresult._tonumber_,
                len             = transresult._tonumber_,
                rpush           = transresult._tonumber_,
                hset            = transresult._tonumber_,
                scard           = transresult._tonumber_,

                -- 以下命令结果被转换成真假值
                set             = transresult._boolean_,
                flushdb         = transresult._boolean_,
                exists          = transresult._boolean_,
                mset            = transresult._boolean_,
                ["select"]      = transresult._boolean_,
                flushall        = transresult._boolean_,
                hmset           = transresult._boolean_,

                -- 以下命令结果被取第一个值
                get             = transresult._first_,
                echo            = transresult._first_,
                rpop            = transresult._first_,
                lpop            = transresult._first_,
                ["type"]        = transresult._first_,
                hget            = transresult._first_,

                -- 以下命令结果不被转换
                smembers        = transresult._ontrans_,
                lrange          = transresult._ontrans_,
                hmget           = transresult._ontrans_,
                srandmember     = transresult._ontrans_,
                
                --其他命令不转换

        }, { __index = transresult });

        --[[runcmd('set', 'key', 'value');]]--
        local runcmd = function ( oper, ... )
                oper = string.lower(oper)
                
                local ok, result = luakv.run(kvhdl, oper, ...)
		if ok then
			if type(result) == "table" then
                		result = setmetatable(result, { __mode = 'kv' });
			end
        		--[[根据命令进行数据转换]]
            		result = (transresult[oper] or transresult._ontrans_)(result);
		else
			assert(false, result)
		end
                return result;
        end

        
        local ok, result = nil, nil;
        if type(...) == 'table' then
                local results = setmetatable({}, { __mode = 'kv' });
                for i, subtab in ipairs(...) do
                        if type(subtab) ~= 'table' then
                                log.writefull("E", "error args %s", scan.dump(...));
                                break;
                        end
                        ok, result = pcall(runcmd, unpack(subtab))
                        if not ok then
                                log.writefull("E", "run command %s fail : %s", 
                                        scan.dump(table.concat( subtab , " ")), result);
                                return ok, result
                        end

                        results[i] = result;
                end

                return ok, results;
        else
                ok, result = pcall(runcmd, ...);
                if not ok then
                        log.writefull("E", "run command %s fail : %s", 
                                scan.dump(table.concat( { ... } , " ")), result);
                end
                return ok, result;
        end
end



do
	luakv.init()
	local hdl = luakv.new("own");

        local ok, result = pcall(luakv.run, hdl)
	print("==>", dump(result));

	local list = {
		{ 'set', 'key', 8},
		{ 'get', 'key'},
	};
	for _, cmd in pairs(list) do
		local ok, result = entry_cmd( hdl, unpack(cmd) )
		print("==>", dump(result));
	end

	local list = {
		{ 'del', 'name'},
		{ 'smembers', 'name'},
		{ 'sadd', 'name', 'zhoukai', 'zhouqiuping' },
		{ 'smembers', 'name'},
		{ 'sismember', 'name', 'zhoukai' },
		{ 'sismember', 'name', 'zhouxiaolong' },
	};
	for _, cmd in pairs(list) do
		local ok, result = entry_cmd( hdl, unpack(cmd) )
		print("==>", dump(result));
	end

	local list = {
		{ 'del', 'name' },
		{ 'get', 'name' },
		{ 'set', 'name', 'zhoukai' },
		{ 'get', 'name' }
	};
	for _, cmd in pairs(list) do
		local ok, result = entry_cmd( hdl, unpack(cmd) )
		print("==>", dump(result));
	end

	local list = {
                {'set', 'age', '24'},
                {'incr', 'age'},
                {'get', 'age'},
	};
	for _, cmd in pairs(list) do
		local ok, result = entry_cmd( hdl, unpack(cmd) )
		print("==>", dump(result));
	end

	local list = {
                {'sadd', 'name', 'zhoukai', 'louis', 'zhouqiuping'},
                {'sadd', 'male', 'zhoukai', 'louis'},
                {'sinter', 'name', 'male'},
                {'sdiffstore', 'name', 'male'},
	};
	for _, cmd in pairs(list) do
		local ok, result = entry_cmd( hdl, unpack(cmd) )
		print("==>", dump(result));
	end

	local list = {
                {'hset', 'people', 'louis', 'louistin'},
                {'hset', 'people', 'zhou', 'zhoukai'},
                {'hgetall', 'people'},
	};
	for _, cmd in pairs(list) do
		local ok, result = entry_cmd( hdl, unpack(cmd) )
		print("==>", dump(result));
	end
end
