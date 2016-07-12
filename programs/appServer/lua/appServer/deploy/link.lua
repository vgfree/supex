module("link")

OWN_POOL = {
	lhttp = {},
	redis = {},
	mysql = {},
	zmq   = {
		downstream = {
			host = "192.168.71.141",
			port = 8090,
			mold = "PUSH",
		},
		setting = {
			host = "192.168.71.143",
			port = 8102,
			mold = "PUSH",
		},
	},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},
}



