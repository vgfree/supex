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
	zmq   = {},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},
}



