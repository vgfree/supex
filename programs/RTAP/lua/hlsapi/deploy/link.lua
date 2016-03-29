module("link")

OWN_POOL = {
	lhttp = {},
	redis = {
                private = {
			host = '127.0.0.1',
			port = 6379,
		},
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



