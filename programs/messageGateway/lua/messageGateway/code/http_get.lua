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
    local message = {
      fd = 0,
      dsize = 0,
      frames = 0,
      frames_offset = {},
      encryption = 0,
      compression = 0,
      content = nil,
    };
    comm_io_send(message);
end

