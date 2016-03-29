#!/usr/bin/env lua

-- Make it easier to test
local src_dir, build_dir = ...
if ( src_dir ) then
    package.path  = src_dir .. "?.lua;" .. package.path
    package.cpath = build_dir .. "?.so;" .. package.cpath
end

local lhp = require 'http.parser'


function ok(assert_true, desc)
    local msg = ( assert_true and "ok " or "not ok " )
    if ( desc ) then
        msg = msg .. " - " .. desc
    end
    print(msg)
    assert(assert_true, msg)
end

local connection_close = [[
HTTP/1.1 200 OK
Date: Wed, 02 Feb 2011 00:50:50 GMT
Connection: close

0123456789]]
connection_close = connection_close:gsub('\n', '\r\n')

function connection_close_test()
    local cbs = {}
    local complete_count = 0
    local body = ''
    function cbs.on_body(chunk)
        if chunk then body = body .. chunk end
    end
    function cbs.on_message_complete()
        complete_count = complete_count + 1
    end

    local parser = lhp.response(cbs)
    parser:execute(connection_close)
    parser:execute('')

    ok(parser:should_keep_alive() == false)
    ok(parser:status_code() == 200)
    ok(complete_count == 1)
    ok(#body == 10)
end





connection_close_test()

