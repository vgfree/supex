local APP_CFG = require('cfg')
local APP_POOL = require('pool')
local APP_LOGS = require('logs')

module('only', package.seeall)

--[[=================================LOG FUNCTION=======================================]]--
OWN_LOGLV = {
	D = {1, "LOG_ON-OFF_DEBUG",		"DEBUG"		},
	I = {2, "LOG_ON-OFF_INFO",		"INFO"		},
	W = {3, "LOG_ON-OFF_WARNING",		"WARN"		},
	F = {4, "LOG_ON-OFF_FAIL",		"FAIL"		},
	E = {5, "LOG_ON-OFF_ERROR",		"ERROR"		},
	S = {9, "LOG_ON_SYSTEM",		"SYSTEM"	},

	verbose = APP_CFG["OWN_INFO"]["LOGLV"],
}

local function save_log(lv, fmt, ...)
	local name = APP_POOL["OWN_APP_NAME"]
	local mode = APP_POOL["OWN_APP_MODE"]
	local user = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local lg = string.format("%s [%s](%s)|%s|-->" .. tostring(fmt) .. "\n",
		os.date('%Y%m%d_%H%M%S'),
		OWN_LOGLV[ lv ][3],
		(user or ""),
		(name or ""),
		...)
	local file
	if APP_CFG["OWN_INFO"]["OPEN_LOGS_CLASSIFY"] then
		file = name
	else
		file = mode
	end
	if not file then
		io.write(lg)
	else
		if not APP_LOGS["OWN_LOG_POOL"][ file ] then
			APP_LOGS.open_log( file )
		end
		APP_LOGS["OWN_LOG_POOL"][ file ]:write( lg )
		APP_LOGS["OWN_LOG_POOL"][ file ]:flush()
	end
end


function log(lv, fmt, ...)
	if lv ~= 'S' and OWN_LOGLV[ lv ][1] < OWN_LOGLV["verbose"] then return end
	local ok, err = pcall(save_log, lv, fmt, ...)
	if not ok then
		print(err)
		print(debug.traceback())
		assert(false, err .. "\n" .. debug.traceback())
	end
end
