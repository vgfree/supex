module("link")

OWN_POOL = {
	lhttp = {},
	redis = {
		damServer = {
			host = "127.0.0.1",
			port = 4210,
		},
		tsdb = {
			host = "192.168.1.16",
			port = 7501,
		}
	},
	mysql = {},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {
		["weiboapi/v2/sendMultimediaPersonalWeibo"] = {
			host = "192.168.1.3",
			port = 8088,
		},
		["spx_txt_to_voice"] = {
			host = "api.daoke.io",
			port = 80,
		},
	},
}



