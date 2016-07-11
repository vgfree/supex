module("link")

OWN_POOL = {
	redis = {
		newstatusRedis= {
                        host = '192.168.1.12', 
                        port = 9002
             	},
                dcRedis = {
                        host = '192.168.1.12',
                        port = 9002
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



