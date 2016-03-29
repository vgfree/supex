module("link")

OWN_POOL = {
	lhttp = {},
	redis = {
                private = {
			host = '192.168.1.11',
			port = 6379,
		},
		public = {
			host = '192.168.1.11',
			port = 6379,
		},
		mapGridOnePercent = {
			host = '192.168.71.61',
			port = 5071,
		},
		mapGridOnePercent01 = {
			host = '192.168.71.61',
			port = 5061,
		},
        },
	mysql = {},
}

OWN_DIED = {
	mysql = {},
	http = {
		["weiboapi/v2/sendMultimediaPersonalWeibo"] = {
			host = "192.168.1.207",
			port = 80,
		},
		["spx_txt_to_voice"] = {
			host = "192.168.1.194",
			port = 2222,
		},
		["dianping_server"] = {
			host = 'api.dianping.com',
			port = 80,
		},
		["4milesAMR"] = {
			host = "127.0.0.1",
			port = 80,
		},
		["offsiteRemind"] = {
			host = "127.0.0.1",
			port = 80,
		},
		["overSpeedAMR"] = {
			host = "127.0.0.1",
			port = 80,
		},
		["roadRankapi/v2/getFrontTrafficInfoByLBS"] = {
			host = "api.daoke.io",
			port = 80,
		},
		["customizationapp/poweronDiaryStart"] = { 
			host = "192.168.71.98",
			port = 80
		},
		["/customizationapp/v2/callbackFetch4MilesAheadPoi"] = {
			host = "192.168.71.98",
			port = 80
		},
		["/customizationapp/v2/callbackDrivingPatternRemind"] = {
			host = "192.168.71.98",
			port = 80
		},
		["crzptY_weather_forcast"]  = { 
			host = "127.0.0.1",  port = 3333 
		},
	},
}
