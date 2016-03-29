module("link")

OWN_POOL = {
	redis = {
		clubCustomPoint = {
			host = '192.168.71.66',
			port = 6005,
		},
		match_road = {
			host = '192.168.1.10',
			port = 5555
		},
		mapSGInfo = {
			host = '192.168.71.66',
			port = 6002
		},
		roadRank_v3_2 = {
			host = '192.168.1.12',
			port = 6000
		--	host = '192.168.71.66',
		--	port = 6379
		--	host = '127.0.0.1',
		--	port = 5404
		},
		mapGridOnePercent_v2 = {
			host = '192.168.71.66',
			port = 6007
		},
		roadname_search = { 
			host = '192.168.71.66',
			port = 6006
		},
	},
}


OWN_DIED = {
	redis = {
	},
	mysql = {},
	http ={

		["club/appointmentPhoto/getNearDeviceOfPoint"] 		= { host = "test.wemeapp.mirrtalk.com",		port = 80 },
		----测试服务器URI(转发多台)
		['publicentry'] = { 
			--{host = '127.0.0.1', port = 8999},
		},
	},
}



