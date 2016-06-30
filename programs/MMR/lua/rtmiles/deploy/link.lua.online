module("link")

OWN_POOL = {
	redis = {
		mapRTMileage = {
			hash='consistent',
			{'M', 'mile1','rtmiles1.redis.daoke.com', 5091, 30},
			{'M', 'mile2','rtmiles1.redis.daoke.com', 5092, 30},
			{'M', 'mile3','rtmiles1.redis.daoke.com', 5093, 30},
			{'M', 'mile4','rtmiles1.redis.daoke.com', 5094, 30},
			{'M', 'mile5','rtmiles2.redis.daoke.com', 5091, 30},
			{'M', 'mile6','rtmiles2.redis.daoke.com', 5092, 30},
			{'M', 'mile7','rtmiles2.redis.daoke.com', 5093, 30},
			{'M', 'mile8','rtmiles2.redis.daoke.com', 5094, 30},
		},
		owner = {
                        host = '127.0.0.1',
                        port = 6400,
                },
		public = {
                        host = 'public.redis.daoke.com',
                        port = 6349,
                },
	        mapOnlineUserM = {
			host = 'rtmiles1.redis.daoke.com',
			port =  5095,
                },
	        mapOnlineUserAccountID = {
			host = 'rtmiles2.redis.daoke.com', 
			port = 5095,
                },

	},

}

OWN_DIED = {
	http = {
		["p2p_continuous_driving_mileage_remind"]                                    = { host = "127.0.0.1",         port = 4080 },
		["p2p_fatigue_driving"]                                    = { host = "127.0.0.1",         port = 4080 },

                --测试服务器URI(转发多台)
                                ['publicentry'] = {
                                                        --{host = '172.16.21.102', port = 4140},
                  					},
	},
}
