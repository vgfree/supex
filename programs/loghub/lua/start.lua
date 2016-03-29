local only = require('only')
local loghub = require('loghub')
local luakv_api   = require('luakv_pool_api')

function app_init()
	luakv_api.init()
        
end

function app_call( tab )
	app_init()
	
	for k,v in ipairs(tab) do
		--print(k, v)
	end
	local ed,et,filename = string.find(tab[2],".*%/(.*)")
	-->>access.log统计transit log 流量
	if filename == "access.log" then
		local ok,err = pcall( loghub.statistics_request_flow,tab )
		if not ok then
			only.log('E', err)
		end
	else
		local ok,err = pcall( loghub.analysis_log, tab ,filename)
		if not ok then
			only.log('E', err)
		end
	end
end
