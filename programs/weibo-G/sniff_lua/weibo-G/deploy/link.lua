module("link")

OWN_POOL = {
	redis = {
                weibo = {
			host = '127.0.0.1',
			port = 6329,
		},
                statistic = {
                        host = '127.0.0.1',
                        port = 6339,
                },
	},
	lhttp = {
                lhttp1 = {
			host = 'www.renren.com',
			port = 80,
		},
                lhttp2 = {
			host = 'www.youku.cn',
			port = 80,
		},
                lhttp3 = {
			host = 'www.zhihu.com',
			port = 80,
		},
	},
	mysql = {},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {
	},
}



