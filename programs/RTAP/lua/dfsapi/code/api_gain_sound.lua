-- voice file from fastdfs to tsdb 

local utils     = require('utils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local gosay     = require('gosay')
local spx_utils	= require('spx_utils')
local redis_api = require("redis_short_api")
local ffmpeg 	= require("lua_ffmpeg")

module('api_gain_sound', package.seeall)
	
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
	if type_val ~= 'amr' and type_val ~= 'mp3' and type_val ~= 'wav' then 
		only.log("E","file is not know the type")
		local afp = supex.rgs(403)
		return supex.over(afp)
	end
	--> get amr
	file = string.gsub(file, type_val .. "$", "amr")
	local index = string.find(file, ':')
	local tmp_amrkey = string.sub(file, index+1, -1)
	local tmp_voicekey = string.gsub(tmp_amrkey, "amr", type_val)
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
	--> conversion
	----------------------------------------------------------------------------------------
	local return_type
	local voice_binary = binary
        if tostring(type_val) == "mp3" then
		return_type = 'audio/mp3'
                local fd = io.open(tmp_amrkey, "w+")
                fd:write(binary)
                fd:close()
                os.execute(string.format("./ffmpeg -i %s -y -ab 5.15k -ar 8000 -ac 1 %s", tmp_amrkey, tmp_voicekey))
                local fd = io.open(tmp_voicekey, "r")
                voice_binary = fd:read("*a")
                fd:close()

                os.execute(string.format("rm -f %s %s", tmp_amrkey, tmp_voicekey))
        elseif tostring(type_val) == "wav" then
		return_type = 'audio/wav'
                local fd = io.open(tmp_amrkey, "w+")
                fd:write(binary)
                fd:close()
                os.execute(string.format("./ffmpeg -i %s -y -ab 5.15k -ar 8000 -ac 1 %s", tmp_amrkey, tmp_voicekey))
                local fd = io.open(tmp_voicekey, "r")
                voice_binary = fd:read("*a")
                fd:close()

                os.execute(string.format("rm -f %s %s", tmp_amrkey, tmp_voicekey))
        elseif tostring(type_val) == "tom" then
		return_type = 'application/octet-stream'
        	local tomcat_len,tom_err = nil,nil
		voice_binary ,tomcat_len, tom_err= ffmpeg.amr2tom(binary,#binary)
	
	else
		return_type = 'audio/amr'
                tmp_voicekey = tmp_amrkey
        end
	----------------------------------------------------------------------------------------
	if not voice_binary then
		only.log('E', 'get from dfsdb is ok but binary is failed')
		local afp = supex.rgs(404)
		return supex.over(afp)
	end
	
	local afp = supex.rgs(200)
	supex.say(afp, voice_binary)
	return supex.over(afp, return_type)
end
