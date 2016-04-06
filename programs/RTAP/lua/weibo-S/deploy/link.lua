module("link")

OWN_POOL = {
	redis = {
		weibo = {
			host = 'weibo.redis.daoke.com',
			port = 6329,
		},
		statistic = {
			host = 'statistic.redis.daoke.com',
			port = 6339,
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



