module("link")

OWN_POOL = {
	lhttp = {},
	redis = {
		save_store = {
			host = "127.0.0.1",
			port = 6379,
		},
	},
	mysql = {},
	zmq   = {
		downstream = {
			host = "127.0.0.1",
			port = 6002,
			mold = "PUSH",
		},
		setting = {
			host = "127.0.0.1",
			port = 9000,
			mold = "PUSH",
		},
	},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},
}



