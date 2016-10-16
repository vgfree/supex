local mysql_api = require('mysql_pool_api')
local redis_api = require('redis_pool_api')
local socket = require('socket')
local utils = require('utils')

INIT = false
MAX = 10000000

MM = 4023289
local list = {
	[1] = utils.random_among(1, MM),
	[2] = utils.random_among(1, MM),
	[3] = utils.random_among(1, MM),
	[4] = utils.random_among(1, MM),
	[5] = utils.random_among(1, MM),
	[6] = utils.random_among(1, MM),
	[7] = utils.random_among(1, MM),
}

ngx.say( "======================================" )
ngx.say( string.format( '%f', socket.gettime() ) )

if INIT then
	for i=1,MAX do
		local ok,data = mysql_api.cmd("root___map", "insert", string.format("insert into test set id=%d, road=%d, grid=%d, poi=%d, att='Hello world!'",i, i, MAX - i, i))
	end
else
	for i,j in pairs( list ) do
		local ok,data = mysql_api.cmd("root___map", "select", string.format("select poi from test where (road=%d or road=%d) and (grid=%d or grid=%d)", j, ( 534 + i )% 2888, MAX - j, MAX - ( 534 + i )% 2888))
		ngx.say( data[1]["poi"] )
		ngx.say( data[2]["poi"] )
		ngx.say( #data )
		ngx.say( string.format( '%f', socket.gettime() ) )
	end
end

ngx.say( string.format( '%f', socket.gettime() ) )
ngx.say( "======================================" )
ngx.say( string.format( '%f', socket.gettime() ) )

if INIT then
	for i=1,MAX do
		local ok,data = redis_api.cmd("test", "set", tostring(i) .. ":grid", tostring( MAX - i ))
		local ok,data = redis_api.cmd("test", "set", tostring(i) .. ":road", tostring( i ))
	end
else
	for i,j in pairs( list ) do
		local ok,data1 = redis_api.cmd("test", "get", tostring(j) .. ":grid")
		local ok,data2 = redis_api.cmd("test", "get", tostring(j) .. ":road")
		ngx.say( data1 )
		ngx.say( data2 )
		ngx.say( string.format( '%f', socket.gettime() ) )
	end
end

ngx.say( string.format( '%f', socket.gettime() ) )
ngx.say( "======================================" )
