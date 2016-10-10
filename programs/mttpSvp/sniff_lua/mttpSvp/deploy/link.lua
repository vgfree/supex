module("link")

OWN_POOL = {
	redis = {
		newstatusRedis= {
                        host = '192.168.1.12', 
                        port = 9002
             	},
                damS = {
                        host = '127.0.0.1',
                        port = 4210
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



