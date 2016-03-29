package.cpath = '../?.so;' .. package.cpath

log = require('lualog')

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

function testlog( ... )
        --支持调用路径记录，并输出日志
        log.writefull('D', "full path ...")
end

function testlevel( level )
        --设置日志级别，如果日志级别不足，则不输出信息
        --日志级别从低到高为：['D', 'I', 'W', 'E', 'F']
        log.setlevel(level);
        log.write('D', 'debug ...');
        log.write('I', 'info  ...');
        log.write('W', 'warn  ...');
        log.write('E', 'error ...');
        log.write('F', 'fatal ...')
end


pool = {}


function test( ... )
        local account = 'zhouqiuping';
        local age = 27;
        local sex = "girl";

        log.write('D', 'Start ...');

        --支持打印nil的变量，且在没用打开句柄的情况下，输出到终端
        local msg = nil;
        log.write('D', msg);
        

        --支持附件信息
        log.addinfo("account", account, "age", age, "sex", sex);

        --使用指定句柄进行输出
        local obj = log.open('test1.log');
        obj:write('I', 'write by object...');
        --对指定句柄刷新，如果当前日志文件创建日期与当前日期不同，则创建新的日志文件
        obj:flush();
        --在日志test1.log中你将看不到E以下的日志信息
        testlevel(4);


        --关掉所有的日志句柄
        -- log.close();
        -- collectgarbage();

        -------------------- 新一轮的测试 -----------------
        local account = 'zhoukai';
        local age = 30;
        local sex = "boy";

        --支持附件信息
        log.addinfo("account", account, "age", age);
        --设置日志路径
        log.setpath('./log');
        --打开一个句柄，并设置为默认句柄
        log.open('test2.log')
        testlevel(0);

        --全路径打印
        testlog();

        --关掉所有的日志句柄
        log.close();
        collectgarbage();

        --清除附加信息
        log.addinfo(nil);

        --支持lua的string.format()格式化,以下信息将输出到终端
        log.write('I', 'run %s script at %s.', 'lua', os.date('%x',os.time()));
        log.write('D', "%s", "End ...");
end

do
        
        test();
        log.setlevel(50);
        -- print(obj);
        log.write('E', '%s, %s, %d', 'zhoukai', obj, obj);
--         print(dump(log));
end
