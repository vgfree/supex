package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
local	MAX = 1
for i=1, MAX do

	local tcp = socket.tcp()
	tcp:setoption('keepalive', true)
	tcp:settimeout(1, 'b')  -- five second timeout

	local ret = tcp:connect("127.0.0.1", 8222)
	if ret == nil then
		return false
	end

	body = '{"powerOn":true,	"accountID":"850212090004492",	"IMEI":"850212090004492", "tokenCode":"1adada912939",	"model":"SG900"}'
	body = '{"powerOff":true,	"accountID":"850212090004492",	"IMEI":"850212090004492", "tokenCode":"1adada912939",	"model":"SG900"}'
	body = '{"collect":true,	"accountID":"850212090004492",	"tokenCode":"1adada912939"}'
	body = '{"collect":true,	"accountID":"850212090004492",	"GPSTime":["1231221"]}'
	body = '{"collect":true,	"accountID":"850212090004492",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	body = '{"collect":true,	"accountID":"850212090004492",	"longitude":["121.358717"], "latitude":["31.222029"], "speed":[150]}'
	body = '{"collect":true, "accountID":"850212090004492", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}'
	body = '{"powerOn":true,	"accountID":"zdfeqE74Vi",	"IMEI":"850212090004492", "tokenCode":"1adada912939",	"model":"SG900"}'

	body = '{"collect":true,	"accountID":"zdfeqE74Vi",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	body = '{"powerOn":true,	"accountID":"zdfeqE74Vi",	"IMEI":"850212090004492", "tokenCode":"1adada912939",	"model":"SG900"}'
	body = '{"collect":true,	"accountID":"zdfeqE74Vi",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	--body = '{"collect":true,	"accountID":"SelJNQsF7B",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[1,20,300], "direction":[23,50,100]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718,121.858718,121.858718], "latitude":[31.222029,31.322029,31.342029,31.342029,31.342029], "speed":[100,90,100,110,120], "direction":[23,50,100,23,23]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.358717,121.358717], "latitude":[31.222029,31.222029,31.222029], "altitude":[31,31,31],"speed":[10,20,300], "direction":[23,50,100]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "longitude":[121.358717,121.758718,121.858718,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029,31.322029,31.342029], "direction":[23,50,100,50,100],"speed":[80,80,80,80,80],"altitude":[31,31,31,31,31],"GPSTime":[1231221,1231221,1231221,1231221,1231221]}'
	body = '{"collect":true, "accountID":"a5ytlr93ap", ' .. 
	'"longitude":[121.3636108,121.3636108,121.3636108,121.3636108,121.3636108],'..
	'"latitude":[31.2245105,31.2245105,31.2245105,31.2245105,31.2245105],'.. 
	'"direction":[123,123,123,123,123],'..
	'"speed":[180,60,50,40,30],'..
	'"altitude":[31,31,31,31,31],'..
	'"GPSTime":[1231221,1231221,1231221,1231221,1231221]}'
	
	body = '{"collect":true, "IMEI":"850212090004492", "tokenCode":"1adada912939", "accountID":"zdfeqE74Vi", "longitude":[115.968644,115.968644,115.968644,115.968644,115.968644], "latitude":[37.630145,37.630145,37.630145,37.630145,37.630145], "direction":[123,123,123,123,123],"speed":[50,80,80,80,80],"altitude":[31,31,31,31,31],"GPSTime":[1231221,1231221,1231221,1231221,1231221]}'
-- rt 12
	body = '{"longitude":[121.360386,121.3603863,121.3603903,121.3604065,121.3604235],"latitude":[31.2236995,31.2237472,31.2238053,31.223866,31.2239192],"IMEI":"409601328282142","model":"SG900","collect":true,"speed":[16,18,19,20,20],"tokenCode":"ybIFSglLSA","accountID":"yFqrK7JeyA","GPSTime":[1456886019,1456886018,1456886017,1456886016,1456886015],"altitude":[-14,-15,-15,-16,-16],"direction":[171,182,189,192,194]}'

        body = '{"longitude":[121.3614377,121.3613337,121.361233,121.3611313,121.361036],"latitude":[31.2234517,31.2234598,31.2234702,31.2234837,31.2234968],"IMEI":"409601328282142","model":"SG900","collect":true,"speed":[35,34,33,32,32],"tokenCode":"ybIFSglLSA","accountID":"yFqrK7JeyA","GPSTime":[1456886034,1456886033,1456886032,1456886031,1456886030],"altitude":[-13,-14,-14,-13,-13],"direction":[95,96,97,96,96]}'
        --body = '{"longitude":[121.3619875,121.3618698,121.361758,121.3616472,121.3615432],"latitude":[31.2234098,31.2234177,31.2234247,31.2234355,31.2234417],"IMEI":"409601328282142","model":"SG900","collect":true,"speed":[41,39,38,37,35],"tokenCode":"ybIFSglLSA","accountID":"yFqrK7JeyA","GPSTime":[1456886039,1456886038,1456886037,1456886036,1456886035],"altitude":[-13,-13,-13,-13,-13],"direction":[94,95,95,96,95]}'
-- rt 
        body = '{"longitude":[121.3636712,121.3636587,121.3636298,121.3635805,121.3635205],"latitude":[31.2234047,31.2233522,31.2233123,31.2232905,31.2232852],"IMEI":"598420613826906","model":"YJDK2","collect":true,"speed":[18,17,16,15,12],"tokenCode":"JoEywH3pRd","accountID":"pQvEPywNzY","GPSTime":[1456886210,1456886209,1456886208,1456886207,1456886206],"altitude":[15,16,16,17,17],"direction":[9,29,60,82,93]}'
        body = '{"longitude":[121.3636767,121.3636753,121.3636757,121.3636778,121.3636772],"latitude":[31.2237758,31.2237007,31.2236227,31.2235428,31.2234657],"IMEI":"598420613826906","model":"YJDK2","collect":true,"speed":[31,0,32,19,18],"tokenCode":"JoEywH3pRd","accountID":"pQvEPywNzY","GPSTime":[1456886215,1456886214,1456886213,1456886212,1456886211],"altitude":[12,13,14,14,15],"direction":[3,2,1,3,6]}'
        body = '{"longitude":[121.3606572,121.3606572,121.3606572,121.3606572,121.3606572],"latitude":[31.2242413,31.2242413,31.2242413,31.2242413,31.2242413],"IMEI":"598420613826906","model":"YJDK2","collect":true,"speed":[0,0,0,0,0],"tokenCode":"JoEywH3pRd","accountID":"pQvEPywNzY","GPSTime":[1456890062,1456890061,1456890060,1456890059,1456890058],"altitude":[18,18,18,18,18],"direction":[73,73,73,73,73]}'

        --body = '{"longitude":[121.332396,121.3322278,121.3320562,121.3318777,121.3317037],"latitude":[31.220089,31.2200553,31.220021,31.2199888,31.2199582],"IMEI":"598420613826906","model":"YJDK2","collect":true,"speed":[59,60,61,62,61],"tokenCode":"JoEywH3pRd","accountID":"pQvEPywNzY","GPSTime":[1456889413,1456889412,1456889411,1456889410,1456889409],"altitude":[16,16,17,17,17],"direction":[77,77,78,78,79]}'

	local data = "POST /publicentry HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	"Host: 127.0.0.1:8222\r\n" ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body


	tcp:send(data)
	local result = tcp:receive("*a")
	print(result)
	tcp:close()
end
--os.execute("sleep 10")

