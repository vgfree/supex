local only              = require("only")
local json		= require("cjson")
local supex		= require('supex')
local msg		= require('msg')
local utils		= require('utils')

local MAX		= 5

module('PUBLIC_FUNC_LIST', package.seeall)

--功	能：将json格式的数据进行转码，并判断数据个数是否超过限制数量
--参	数：pointList
--返 回 值：result_data---解析后的数据	  false ---解析失败
function parse_data(pointList)

	local ok, result_data = pcall(json.decode,pointList)
        if not ok or not result_data then
		only.log('E', "json decode result error!")
		return false
        end

	if #result_data > MAX then
		only.log('I',"Exceed the maximum number")
		return false
	end

	return result_data
end

--功	能：构成千分之五格网
--参	数：longitude---经度	latitude---纬度
--返 回 值：grid_longitude---构成千分之五格网经度   grid_latitude---构成千分之五格网纬度
function grid_five_thousandths(longitude,latitude)

	local longitude100    = math.floor(tonumber(longitude)*100)
	local latitude100    = math.floor(tonumber(latitude)*100)
	local longitude1000   = math.floor(tonumber(longitude)*1000)
	local latitude1000   = math.floor(tonumber(latitude)*1000)

	local grid_longitude = (longitude1000 >= (longitude100*10 + 5)) and (longitude100*10 + 5) or (longitude100*10)
	local grid_latitude = (latitude1000 >= (latitude100*10 + 5)) and (latitude100*10 + 5) or (latitude100*10)

	return grid_longitude, grid_latitude
end

--功	能：转换返回值格式为json字符串
--参	数：数组，例： {"ME01027", "parse data error"}
--返 回 值：json字符串，{"ERRORCODE":"ME01027","RESULT":"parse data error"}
function to_json(tb)
	local result = {}
	result['ERRORCODE'] = tb[1]
	result['RESULT'] = tb[2]
	local ok, str = utils.json_encode(result)
	return str
end

--功	能：发送响应，给请求方返回ERRORCODE和RESULT
--参	数：json字符串，例：{"ERRORCODE":"ME01027","RESULT":"parse data error"}
function send_respond(msg)
	local afp = supex.rgs(200)
	supex.say(afp, msg)
	supex.over(afp)
end


