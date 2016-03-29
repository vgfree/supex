module("cfg")


keywords = {
	powerOn		= {{true},{},{},{}},
	powerOff	= {{true},{},{},{}},
	collect		= {{true},{},{},{}},

	accountID	= {{},{},{},{}},
	IMEI		= {{},{},{},{}},
	model		= {{},{},{},{}},
	tokenCode	= {{},{},{},{}},

	GSENSORTime	= {{},{},{},{}},
	gx		= {{},{},{},{}},
	gy		= {{},{},{},{}},
	gz		= {{},{},{},{}},

	GPSTime		= {{},{},{},{}},
	speed		= {{},{},{},{}},
	altitude	= {{},{},{},{}},
	direction	= {{},{},{},{}},
	position	= {{},{},{},{"is_4_miles_ahead_have_poi"}},
	T_FRONT_POI_SET	= {{},{},{},{}},
	T_FRONT_ROAD_INFO = {{},{},{},{}},
}

workfunc = {
	["exact"] = {"app_task_forward"},
	["local"] = {"app_task_forward"},
	["whole"] = {"app_task_forward"},
	["alone"] = {},
}

ranklist = {-->other default is 0,the higher rank function will be run the first.
	["is_4_miles_ahead_have_poi"]	= 44,
}

slotlist = {
	powerOn		= 1,
	powerOff	= 2,
	collect		= 3,

	accountID	= 20,
	IMEI		= 21,
	model		= 22,
	tokenCode	= 23,

	GSENSORTime	= 31,
	gx		= 32,
	gy		= 33,
	gz		= 34,

	GPSTime		= 35,
	speed		= 36,
	altitude	= 37,
	direction	= 38,
	longitude	= 39,
	latitude	= 40,
	extragps 	= 50,

	T_FRONT_ROAD_INFO = 60,
	T_FRONT_POI_SET	  = 61,	
}

EXP_KEYS = {
}

--定义补充模块中文名称
MOD_NAME = {
}

OWN_INFO = {
	LOGLV = 0,
        SYSLOGLV = false,
	DEFAULT_LOG_PATH = "./logs/",
	--OPEN_LOGS_CLASSIFY = false,
	OPEN_LOGS_CLASSIFY = true,
	
	--if current system is for real customer set "true" or "false"
	CUSTOM_ORIENTED_SYSTEM = false,


	key_len = 20,
	bit_step = 5, --> must <= 6
	--> 20 * 5 kinds
}

-- testing users accounts
TEST_ACCOUNT = {
}
