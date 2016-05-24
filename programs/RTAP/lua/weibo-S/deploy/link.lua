module("link")

OWN_POOL = {
	redis = {
		weibo = {
			host = '192.168.1.11',
			port = 6329,
		},
		weibo_hash = {
			hash = 'customer',
			{'M', 'a0', '192.168.1.11', 6991, 30},
			{'M', 'a1', '192.168.1.11', 6992, 30},
			{'M', 'a2', '192.168.1.11', 6993, 30},
			{'M', 'a3', '192.168.1.11', 6994, 30},
		},

		statistic_hash = {
			hash = 'customer',
			{'M', 'b0', '192.168.1.11', 6339, 30},
			{'M', 'b1', '192.168.1.11', 6329, 30},
		},


		statistic = {
			host = '192.168.1.11',
			port = 6339,
		},
		read_private = {
			host = '192.168.1.11',
			port = 6319,
		},
	},
	lhttp = {
	},
	mysql = {},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {
	},
}
