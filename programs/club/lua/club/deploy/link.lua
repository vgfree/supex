module("link")

OWN_POOL = {
	redis = {
		clubCustomPoint = {
			host = '192.168.71.66',
			port = 6005,
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



