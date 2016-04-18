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

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

require("zmq")

local redis_api = require('redis_pool_api')

redis_api.init( )

local api_dk_newstatus = require("api_dk_newstatus")


local data = 'keyvalue\r\nnt=0&mt=21776&Accountid=111111111111111&gps=221215102957,11444.98295E,4045.92141N,262,18,746;221215102958,11444.97898E,4045.92095N,262,19,746;221215102959,11444.97479E,4045.92069N,263,20,746;221215103000,11444.97046E,4045.92029N,262,20,746;221215103001,11444.96605E,4045.91999N,262,21,746&tokencode=0al15UMqpo&imsi=460022211427272&imei=752795632561713&mod=SG900'
api_dk_newstatus.handle( data )
