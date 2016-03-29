local utils     = require('utils')
local cutils    = require('cutils')
local only      = require('only')
local msg       = require('msg')
local gosay     = require('gosay')
local spx_utils	= require('spx_utils')
local safe      = require('safe')
local dfs_cfg 	= require('dfs_cfg')
local cjson     = require('cjson')
local supex     = require('supex')
local lua_imageMagick = require("lua_imageMagick")

module('api_pic_append_watermark', package.seeall)

local url_tab = {
	type_name = 'system',
	app_key  = '',
	client_host = '',
	client_body = '',
}

--logo默认X坐标
local DEFAULT_OFFSET_X = 20

--logo默认Y坐标
local DEFAULT_OFFSET_Y = 10

--logo 
local logo_names = {
	'logo.png',
	'logo-01.png',
	'logo-02.png',
}

--logo的宽和高
local logo_attributes = {
	[1] = {24,30},
	[2] = {16,20},
	[3] = {149,112},
}

--字体
local font_names = {
	'PingFang-SC-Thin.ttf',
	'PingFang.ttc',
	'simhei.ttf',
	'SIMKAI.TTF',
}

local logo_path = "/data/dfsapi/lua/dfsapi/conf/"


local function save_file_dfs(binary,args,image_type,cacheTime)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainImage", dfs_cfg["image_domain"])		
	local url_args = 'isStorage=true&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local jpgkey = basekey .. "." .. image_type
	----------------------------------------------------------------------------------------
	-->以uuid.image_type存放在本地
	local tmp_jpgkey = uuid .. '.' .. image_type
	local fd = io.open(tmp_jpgkey, "w+")
	fd:write(binary)
	fd:close()
	--获取所传图片的宽和高
	local ok, width, height = lua_imageMagick.readImageSize(tmp_jpgkey)
	-- locationX或locationY不传或为空则默认坐标
	-- 默认原点为图片左上角
	local locationX = tonumber(args['locationX']) or tonumber(DEFAULT_OFFSET_X)
	local locationY = tonumber(args['locationY']) or tonumber(DEFAULT_OFFSET_Y)		
	if  locationX > width or locationY > height then 
		only.log('E', "locationX or locationY error!")
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "locationX or locationY")
	end 
	
	local fontsize, text_x, text_y
	--logo的宽和高
	local logo_width = logo_attributes[args['logoName']][1]
	local logo_height = logo_attributes[args['logoName']][2]
	
	--设置字体大小与文字位置
	if string.find(args['annotation'],"\n") then 
		text_x = locationX +logo_width+8
		text_y = locationY +logo_height/3
	    fontsize = 12
	else 
		text_x = locationX +logo_width+8
		text_y = locationY +logo_height/1.4
	    fontsize = 14
	end
	--设置字体颜色，默认为黑色
	local font_color = args['color'] 
	if not font_color or font_color == '' then
		font_color = 'black'
	end

	-->加logo
	os.execute(string.format("convert %s %s -geometry +%s+%s -composite %s", tmp_jpgkey, logo_path .. logo_names[args['logoName']], locationX, locationY,tmp_jpgkey))
	only.log('D', string.format("convert %s %s -geometry +%s+%s -composite %s", tmp_jpgkey, logo_path .. logo_names[args['logoName']], locationX, locationY,tmp_jpgkey))
	--加文字
	os.execute(string.format("convert -fill %s -font %s -pointsize %s -draw 'text %s,%s \"%s\"' %s %s", font_color, logo_path .. font_names[args['font']], fontsize, text_x, text_y, args['annotation'], tmp_jpgkey, tmp_jpgkey))
	only.log('D', string.format("convert -font %s -pointsize %s -draw 'text %s,%s \"%s\"' %s %s", logo_path .. font_names[args['font']], fontsize, text_x, text_y, args['annotation'], tmp_jpgkey, tmp_jpgkey))
	local fd = io.open(tmp_jpgkey, "r")
	local jpg_binary = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -f %s", tmp_jpgkey))
	----------------------------------------------------------------------------------------
	
	local info_tab = {
		url		= '',
		fileID		= uuid,
		fileSize	= #jpg_binary,
	}
	-->jpg
	cacheTime = tonumber(cacheTime)
	local ok, result, memb = spx_utils.set_to_dfsdb(jpgkey, jpg_binary, cacheTime)
	if not ok then 
		ok, result, memb = spx_utils.set_to_dfsdb_spare(jpgkey, jpg_binary, cacheTime)
		if not ok then return false, "save to tsdb failed" end
	end

	local group = memb['mark']
	local uri = string.format(url_args, group, jpgkey)
	info_tab['url'] = url_path .. "?" .. cutils.url_encode(uri)
	return true, info_tab
end

local function save_file_redis(binary,args,image_type,cacheTime)
	local uuid = utils.create_uuid()
	local url_path = string.format("http://%s/dfsapi/v2/gainImage", dfs_cfg["image_domain"])		
	local url_args = 'isStorage=false&group=%s&file=%s'
	local basekey = os.time() .. ":" .. uuid
	local jpgkey = basekey .. "." .. image_type
	----------------------------------------------------------------------------------------
	-->以uuid.image_type存放在本地
	local tmp_jpgkey = uuid .. '.' .. image_type
	local fd = io.open(tmp_jpgkey, "w+")
	fd:write(binary)
	fd:close()
	--获取所传图片的宽和高
	local ok, width, height = lua_imageMagick.readImageSize(tmp_jpgkey)
	-- locationX或locationY不传或为空则默认坐标
	-- 默认原点为图片左上角
	local locationX = tonumber(args['locationX']) or tonumber(DEFAULT_OFFSET_X)
	local locationY = tonumber(args['locationY']) or tonumber(DEFAULT_OFFSET_Y)		
	if  locationX > width or locationY > height then 
		only.log('E', "locationX or locationY error!")
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "locationX or locationY")
	end 

	local fontsize, text_x, text_y
	--logo的宽和高
	local logo_width = logo_attributes[args['logoName']][1]
	local logo_height = logo_attributes[args['logoName']][2]
	
	--设置字体大小与文字位置
	if string.find(args['annotation'],"\n") then 
		text_x = locationX +logo_width+8
		text_y = locationY +logo_height/3
	    fontsize = 12
	else 
		text_x = locationX  +logo_width+8
		text_y = locationY  +logo_height/1.4
	    fontsize = 14
	end
	--设置字体颜色，默认为黑色
	local font_color = args['color'] 
	if not font_color or font_color == '' then
		font_color = 'black'
	end

	-->加logo
	os.execute(string.format("convert %s %s -geometry +%s+%s -composite %s", tmp_jpgkey, logo_path .. logo_names[args['logoName']], locationX, locationY,tmp_jpgkey))
	only.log('D', string.format("convert %s %s -geometry +%s+%s -composite %s", tmp_jpgkey, logo_path .. logo_names[args['logoName']], locationX, locationY,tmp_jpgkey))
	--加文字
	os.execute(string.format("convert -fill %s -font %s -pointsize %s -draw 'text %s,%s \"%s\"' %s %s", font_color, logo_path .. font_names[args['font']], fontsize, text_x, text_y, args['annotation'], tmp_jpgkey, tmp_jpgkey))
	only.log('D', string.format("convert -font %s -pointsize %s -draw 'text %s,%s \"%s\"' %s %s", logo_path .. font_names[args['font']], fontsize, text_x, text_y, args['annotation'], tmp_jpgkey, tmp_jpgkey))
	local fd = io.open(tmp_jpgkey, "r")
	local jpg_binary = fd:read("*a")
	fd:close()

	os.execute(string.format("rm -f %s", tmp_jpgkey))
	----------------------------------------------------------------------------------------
	
	local info_tab = {
		url		= '',
		fileID		= uuid,
		fileSize	= #jpg_binary,
	}
	-->jpg
	cacheTime = tonumber(cacheTime)
	local ok, result, memb = spx_utils.set_to_redis(jpgkey, jpg_binary, cacheTime)
	if not ok then 
		ok, result, memb = spx_utils.set_to_redis_spare(jpgkey, jpg_binary, cacheTime)
		if not ok then return false, "save to redis failed" end
	end

	local group = memb['mark']
	local uri = string.format(url_args, group, jpgkey)
	info_tab['url'] = url_path .. "?" .. cutils.url_encode(uri)
	return true, info_tab
end

function handle()
	local file_binary, file_name = nil, nil
	local args = supex.get_our_body_table()
	for _,v in pairs(args or {}) do
		if type(v) == "table" then	
			file_binary = v['data']
			file_name = v['file_name']
			break
		end
	end
	if not args or not file_binary then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "arg is nil")
	end

	local st, ed, image_type = string.find(file_name, "%.(%a+.)$")
	--图片支持格式有bmp,jpg,jpeg,gif和png
	if image_type ~= 'bmp' and image_type ~= 'jpg' and image_type ~= 'jpeg' and image_type ~= 'gif' and image_type ~= 'png' then 
		only.log('E', "image format error")
		return gosay.resp_msg(msg["MSG_ERROR_IMAGE_TYPE"], "image type error")
	end	
	--check
	url_tab['app_key'] = args['appKey']	
	if tonumber(args['length'] or 0) ~= tonumber(#file_binary) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "file length")
	end
--	check_parameter(args)

	args['logoName'] = tonumber(args['logoName']) or 1
	if not logo_names[args['logoName']] then 
		args['logoName'] = 1
	end
	args['font'] = tonumber(args['font']) or 1
	if not font_names[args['font']] then 
		args['font'] = 1
	end
	
	local isStorage = args['isStorage'] 
	if not isStorage or isStorage == '' then
		isStorage = 'false'
	end
	local cacheTime = args['cacheTime'] or ''
	local ok, ret_val = nil, nil
	if isStorage == 'true' then
		ok, ret_val = save_file_dfs(file_binary, args, image_type, cacheTime)	
		if not ok or not ret_val then
			only.log('W','file save redis failed!')
			return gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed")
		end
	elseif isStorage == 'false' then
		ok, ret_val  = save_file_redis(file_binary, args, image_type, cacheTime)
		if not ok or not ret_val then
			only.log('W','file save dfs failed!')
			return gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed")
		end	
	else
		only.log('E', 'isStorage is error')
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], "isStorage")
	end

	--加水印并存入dfs
--	local ok, ret_val  = save_file_dfs(file_binary,args,image_type)
--	if not ok or not ret_val then
--		only.log('W','file save dfs failed!')
--		return gosay.resp_msg(msg["MSG_ERROR_DFSSAVE_FAILED"], "file save failed")
--	end

	local ok, dfs_url_info = pcall(cjson.encode, ret_val)
	dfs_url_info = string.gsub(dfs_url_info, "\\", "")
	return gosay.resp_msg(msg["MSG_SUCCESS_WITH_RESULT"], dfs_url_info)
end
