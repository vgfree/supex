module("link")

OWN_POOL = {
	redis = {},
	mysql = {},
	zmq = {
		damR = {
			host = "127.0.0.1",
			port = 8989,
			mold = "PUSH",
		},
	},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},
}



