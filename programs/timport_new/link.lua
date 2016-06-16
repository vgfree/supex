module("link")

OWN_POOL = {
	redis = {
                IdKey = {
                        host = '192.168.1.12',
                        port = 9001,
                },
		tsdb01 = {
			host = '192.168.1.12',
			port = 7101,
		},
	},
}

OWN_DIED = {
	mysql = {},
	http = {                                                              
        },
	key = {
		KEY1 = 'ACTIVEUSER:',
                KEY2 = 'GPS:',
        },

}
