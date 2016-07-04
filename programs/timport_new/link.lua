module("link")

OWN_POOL = {
	redis = 
	{
                redis0 = {
                        host = '192.168.1.12',
                        port = 9001,
                },
		redis1 = {
			host = '192.168.1.12',
			port = 9002,
		},
		tsdb0 = {
			host = '127.0.0.1',
			port = 7501,
		},
		tsdb1 = {
                        host = '127.0.0.1',
                        port = 7503,
                },
		tsdb2 = {
                        host = '127.0.0.1',
                        port = 7505,
                },
		tsdb3 = {
                        host = '127.0.0.1',
                        port = 7507,
                },
	},
}

OWN_ZOOKEEPER = {
    zkhost = "192.168.1.14:2181,192.168.1.14:2181",
}

OWN_DIED = {
	mysql = {},
	http = {                                                              
        },
}
