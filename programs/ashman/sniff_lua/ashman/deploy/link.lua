module("link")

OWN_POOL = {
	redis = {
		private = {
			host = '192.168.1.14',
			port = 6379,
		},
		statistic = {
		    	host = '192.168.1.11',
			port = 6379,
		},
		mapGridOnePercent = {
			host = '192.168.71.61',
			port = 5071,
		},

	},
}

OWN_DIED = {
	http = {
	    	dataCore= {
			host = "192.168.1.102",
			port = 8080,
		}
	},
}
