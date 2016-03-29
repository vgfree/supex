module("link")

OWN_POOL = {
	redis = {
	},
	mysql = {
		transit_flow = {
		host = '127.0.0.1',
		port = 3306,
		database = 'transit_flow',
		user = 'root',
		password ='root',
		}
		}
}


OWN_DIED = {
		--> mysql
		mysql = {
		transit_flow = {
		host = '127.0.0.1',
		port = 3306,
		database = 'transit_flow',
		user = 'root',
		password ='root',
		},
		},
		--[[
		app_mirrtalk___mirrtalkCall = {
		host = '192.168.1.3',
		port = 3306,
		database = 'mirrtalkCall',
		user = 'app_mirrtalk',
		password ='appmabc123',
		},
		app_tweet___tweet = {
		host = '192.168.1.3',
		port = 3306,
		database = 'tweet',
		user = 'app_tweet',
		password = 'apptabc123',
		},
		app_weibo___app_weibo = {
		host = '192.168.1.3',
		port = 3306,
		database = 'app_weibo',
		user = 'app_weibo',
		password ='appwabc123',
		},
		]]--
	
	redis = {
		--> redis
		--[[
		logs = {
		host = "192.168.1.3",
		port = 6379,
		},
		]]--
		private = {
			host = "192.168.1.11",
			port = 6379,
		},
	},
	http = {
		["mapapi/v2/getLocation"] = {
			host = "221.228.231.83",
			port = 80,
		},
	},
}



