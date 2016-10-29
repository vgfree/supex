local utils     = require('utils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local gosay     = require('gosay')
local spx_utils	= require('spx_utils')
local redis_api = require("redis_short_api")
local magick = require("magick.init")
module('api_gain_image', package.seeall)


function handle()
	local args = supex.get_our_uri_table()
	if not args then
		only.log('E', 'args is nil')
		local afp = supex.rgs(400)
		return supex.over(afp)
	end
	
	local group = args["group"]
	local file  = args["file"]
	local isStorage = args['isStorage']
	local scaling = args['scaling']
	
	if not file or (not group and not isStorage)then
		only.log('E', 'args is nil')
		local afp = supex.rgs(400)
		return supex.over(afp)
	end
	--judge isStorage exist
	if not isStorage then
		group = string.gsub(group, 'group_', 'dfsdb_')
		isStorage = 'true'
	end
	--> fiter
	local st, ed, type_val = string.find(file, "%.(%a%a.)$")
	if (not type_val) then
		only.log("E","file is not know the type")
		local afp = supex.rgs(403)
		return supex.over(afp)
	end
	
	local index = string.find(file, ':')
	local tmp_jpgkey = string.sub(file, index+1, -1)
	--> get image
	local ok,binary = nil ,nil
	if isStorage == 'true' then
		ok, binary = spx_utils.get_from_dfsdb(file, group)
		if not ok then
			ok,binary = spx_utils.get_from_dfsdb_spare(file, group)
			if not ok then
				only.log('E', 'get from dfsdb is failed')
				local afp = supex.rgs(500)
				return supex.over(afp)
			end
		end
	else
		ok, binary = spx_utils.get_from_redis(file, group)
		if not ok then
			ok,binary = spx_utils.get_from_redis_spare(file, group)
			if not ok then
				only.log('E', 'get from redis is failed')
				local afp = supex.rgs(500)
				return supex.over(afp)
			end
		end
	end

	if not binary then
		only.log('E', 'get from dfsdb is ok but binary is failed')
		local afp = supex.rgs(404)
		return supex.over(afp)
	end

	local img = assert(magick.load_image_from_blob(binary))
	local local_width = img:get_width()
	local local_height = img:get_height()
	only.log('D',string.format( "image is %d X %d,type is %s",local_width,local_height, img:get_format()))
	img:set_format("jpeg")	
	
	
	local jpg_binary = nil 
	if scaling then
		local index = string.find(scaling, 'x') 
		width = tonumber(string.sub(scaling, 1, index-1))
		local res = string.sub(scaling, index+1, -1) 
		index = string.find(res, 'x')
		height = tonumber(string.sub(res, 1, index-1))
		local mode = tonumber(string.sub(res, index+1, -1))
		if not width or not height or not mode then
		   	only.log('E', "scaling is error")
			local afp = supex.rgs(403)
			img:destroy()
			return supex.over(afp)
		end
		if not (width >= 10 and width <= 5000) or not (height >= 10 and height <= 5000) then
		   	only.log('E', "width or height is error")
			local afp = supex.rgs(403)
			img:destroy()
			return supex.over(afp)
		end
		if width == local_width and height == local_height then
			local afp = supex.rgs(200)
			jpg_binary = img:get_blob()
			img:destroy()
			supex.say(afp, jpg_binary)
			return supex.over(afp, "image/jpeg")
		end
		----------------------------------------------------------------------------------------
		-->图片缩放
		if mode == 0 then		-- 按参数无条件拉伸
			img:resize(width,height)
		elseif mode == 1 then		-- 取宽等比例缩放
			img:resize(width)
		elseif mode == 2 then		-- 取高等比例缩放
			img:resize(nil,height)
		elseif mode == 3 then		-- 根据大的值缩放
			img:resize(width)
		elseif mode == 4 then		-- 根据小的值缩放
			img:resize(nil,height)
		else
			only.log('E', "mode is error")
			img:destroy()
			local afp = supex.rgs(403)
			return supex.over(afp)
		end
	
		
		----------------------------------------------------------------------------------------
	end
	
	jpg_binary = img:get_blob()
	img:destroy()
	local afp = supex.rgs(200)
	if jpg_binary then
		supex.say(afp, jpg_binary)
	else
		supex.say(afp, binary)
	end
	return supex.over(afp, "image/jpeg")
end
