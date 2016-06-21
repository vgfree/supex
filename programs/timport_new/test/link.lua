module("link")

OWN_POOL = {
	redis = {
                redis0 = {
                        host = '192.168.1.12',
                        port = 9001,
                },
		redis1 = {
			host = '192.168.1.12',
			port = 9002,
		},
	},
}

OWN_DIED = {
	mysql = {},
	http = {                                                              
        },
}
