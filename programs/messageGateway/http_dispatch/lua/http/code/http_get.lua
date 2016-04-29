local only       =   require('only')
local supex      =   require('supex')
local cjson      =   require('cjson')
local socket     =   require('socket')
local safe       =   require('safe')
local http_api   =   require('http_short_api')
local utils      =   require('utils')

module('http_get', package.seeall)

function handle()
    only.log("D","get interface start ...")
    local data = supex.get_our_body_table()
end

