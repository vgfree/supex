module("link")

OWN_POOL = {
	redis = {
		newstatusRedis= {
                        host = '127.0.0.1', 
                        port = 6379
             	},
                dcRedis = {
                        host = '172.16.71.43',
                        port = 6379
                } 


	},
	mysql = {},
	zmq = {},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},
}



