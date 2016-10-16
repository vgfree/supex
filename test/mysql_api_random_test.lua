local mysql_api = require('mysql_pool_api')
local socket = require('socket')
local utils = require('utils')

local list = {
	[1] = utils.random_among(1, 1000),
	[2] = utils.random_among(1, 1000),
	[3] = utils.random_among(1, 1000),
	[4] = utils.random_among(1, 1000),
	[5] = utils.random_among(1, 1000),
	[6] = utils.random_among(1, 1000),
	[7] = utils.random_among(1, 1000),
}

ngx.say( string.format( '%f', socket.gettime() ) )

--[[
for i=7,1000 do
	local ok,data = mysql_api.cmd("root___map", "insert", string.format("insert into test set id=%d, road=%d, grid=%d, poi=%d",i, i + 10000, i + 20000, i + 30000))
end
]]--
for i,j in pairs( list ) do
	local ok,data = mysql_api.cmd("root___map", "select", string.format("select grid from test where poi=%d", 30000 + j))
	ngx.say( data[1]["grid"] )
	ngx.say( string.format( '%f', socket.gettime() ) )
end

ngx.say( string.format( '%f', socket.gettime() ) )
ngx.say( "======================================" )
