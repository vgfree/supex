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
			host = '192.168.1.12',
			port = 7101,
		},
		tsdb1 = {
                        host = '192.168.1.12',
                        port = 7103,
                },
		tsdb2 = {
                        host = '192.168.1.12',
                        port = 7105,
                },
		tsdb3 = {
                        host = '192.168.1.12',
                        port = 7107,
                },
	},
}

OWN_DIED = {
	mysql = {},
	http = {                                                              
        },
}
