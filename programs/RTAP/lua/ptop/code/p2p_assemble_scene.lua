local conf	= require('CURRENT_SCENE_LIST')	
local WORK_FUNC_LIST = require('WORK_FUNC_LIST')
module('p2p_assemble_scene',package.seeall)

--触发逻辑：根据星期几、日期、时间段、天气，四种组合而成生成下发url
function handle( data, app_name )

	--> 从配置中选出星期几所对应的字符
        local week = os.date("%w",os.time())
        local assemble_week = conf.OWN_LIST["week"][week]

	--> 根据当前时间从配置中选出对应的时间段从而产生对应的字符
	local idx = 0
        local time = os.date("%X", os.time())
        for i = 1 , #conf.OWN_LIST["time"] do
                if time > conf.OWN_LIST["time"][i]['min'] and time < conf.OWN_LIST["time"][i]['max']  then
                        idx = i
                        break
                end
        end
	local assemble_time = "0" .. idx
	
	-->>根据当前日期从配置中选出对应的字符
        local date = os.date("%x", os.time())
	local assemble_date = conf.OWN_LIST["date"][date]
	if not assemble_date then
		assemble_date = "00"
	end

	-->>根据当前天气状况从配置中选出对于的字符
        local assemble_weather = conf.OWN_LIST["weather"][data] 
	if not assemble_weather then
		assemble_weather ="00"
	end
	
	-->> 拼接url
	local assemble_url = assemble_weather .. assemble_date .. assemble_week .. assemble_time

	-->>调用微薄接口
	WORK_FUNC_LIST["half_url_unknow_str_send_weibo"]("p2p_assemble_scene",assemble_url)
end
