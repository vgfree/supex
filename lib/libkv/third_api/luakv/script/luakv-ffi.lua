module("luakv-ffi", package.seeall)


--error code
local libkvcode = {
        ERR_NONE           = 0,
        ERR_TYPE           = 1,
        ERR_NO_KEY         = 2,
        ERR_SYNTAX         = 3,
        ERR_SAME_OBJECT    = 4,
        ERR_OUT_OF_RANGE   = 5,
        ERR_NIL            = 6,
        ERR_NOT_INIT       = 7,
        ERR_BUSY           = 8,
        ERR_ARGUMENTS      = 9,
        ERR_PROTOCOL       = 10,
        ERR_VALUE          = 11,
        ERR_DB_INDEX       = 12,
        ERR_CMD            = 13,
}


local ffi = require('ffi')

local libkv = ffi.load('kv', true)

ffi.cdef[[
        typedef struct kv_answer {
                int errnum;
                const char* err;
                unsigned long count; 
                struct kv_answer_value* head, *tail;
        }kv_answer_t;

        typedef struct { void *hdl;} luakv_hdl;
        typedef struct { void *ans; void *iter;} luakv_ansiter;

        void*   kv_create               (const char *param1, ...);
        void    kv_destroy              (void *handler);

        void*   kv_ask                  (void *handler, const char *cmd, unsigned int cmdlen);
        void    kv_answer_release       (void *a);

        void*   kv_answer_get_iter      (void *a, int direction);
        void    kv_answer_release_iter  (void *iter);

        void*   kv_answer_next          (void *iter);
        char*   kv_answer_value_to_string(void *value);
]]



--释放函数
local libkv_hdl_gc = function ( hdl )
        if hdl.hdl ~= nil then
                print("release libkv-hdl");
                ffi.C.kv_destroy(hdl.hdl);
        end
end

local libkv_ansiter_gc = function ( ansiter )
        if ansiter.ans ~= nil then
                print("release answer.");
                ffi.C.kv_answer_release(ansiter.ans);
        end

        if ansiter.iter ~= nil then
                print("release iter.");
                ffi.C.kv_answer_release_iter(ansiter.iter);
        end
end

-- 构造方法
local libkv_ansiter_mt = {
        __index = {
                getiter = function ( ansiter )
                        local iter = ffi.C.kv_answer_get_iter(ansiter.ans, 0);

                        if iter == nil then
                                return false;
                        end
                        ansiter.iter = iter;

                        print(string.format("Get iterator [%s] by answer [%s]", iter, ansiter.ans));

                        return true;
                end,

                getnext = function ( ansiter )

                        local value = ffi.C.kv_answer_next(ansiter.iter);
                        if value == nil then
                                return nil;
                        end

                        local result = ffi.C.kv_answer_value_to_string(value);
                        if value == nil then
                                return nil;
                        end

                        return ffi.string(result);
                end,
        }
}

-- 构造器
local libkv_ansiter_t = ffi.metatype("luakv_ansiter", libkv_ansiter_mt);

-- 构造方法
local libkv_hdl_mt = {
        __index = {
                ask = function ( hdl, cmd )
                        
                        local ans = ffi.C.kv_ask(hdl.hdl, cmd, #cmd);

                        ans = ffi.cast("kv_answer_t*", ans);

                        print(string.format("Get answer , Run [%s] comand : ", cmd), ans);

                        -- 错误
                        if ans[0].errnum ~= libkvcode.ERR_NONE and ans[0].errnum ~= libkvcode.ERR_NIL then
                                local errstr = ffi.string(ans[0].err);
                                ffi.C.kv_answer_release(ans);
                                error(string.format("run comand [%s] fail : %s.", cmd, errstr));
                        end

                        -- 没有结果
                        if ans[0].errnum == libkvcode.ERR_NIL then
                                print(string.format("run comand [%s] warn : no result.", cmd));
                                ffi.C.kv_answer_release(ans);
                                return function ( ... )
                                        return nil;
                                end
                        end

                        local ansiter = ffi.gc(libkv_ansiter_t(ans, nil), libkv_ansiter_gc);

                        if not ansiter:getiter() then
                                ansiter = nil;
                                error(string.format("run comand [%s] fail : can't get iterator.", cmd));
                        end

                        -- 返回一个闭包
                        return function ( ... )
                                -- body
                                return ansiter:getnext();
                        end
                end
        }
}

--构造器
local libkv_hdl_t = ffi.metatype("luakv_hdl", libkv_hdl_mt);


function create( ... )
        -- body
        local hdl = ffi.C.kv_create(nil);

        if hdl == nil then
                error("can't create handle of libkv.");
        end

        print("create a handle of libkv : ", hdl);

        return ffi.gc(libkv_hdl_t(hdl), libkv_hdl_gc);
end

