module("link")

OWN_POOL = {
	redis = {

		public = {
			host = '192.168.1.11',
			port = 6349,
		},
		owner = {
			host = '192.168.71.46',
			port = 6410,
		},
		private = {
			host = '192.168.1.11',
			port = 6319,
		},
                private_hash = {
                	-- hash模式consistent／customer
                	-- hash = 'consistent',
                	hash = 'customer',
			{'M', 'n1', '192.168.71.47', 6400, 30},
                        {'M', 'n2', '192.168.71.47', 6401, 30},
                        {'M', 'n3', '192.168.71.47', 6402, 30},
                        {'M', 'n4', '192.168.71.47', 6403, 30},
                },
		weibo = {
			host = '192.168.1.11',
			port = 6329,
		},
		roadRelation = {
			host = '192.168.71.73',
			port =  4060,
		},  
		mapRoadLine = { 
			host = '192.168.1.10',
			port =  5602,
		},  
		mapLineNode = { 
			host = '192.168.71.65',
			port =  5601,
		},
		mapRoadInfo = {
			host = '192.168.1.9',
			port = 5603,
		},
		mapdata  = {
			host = '192.168.1.9',
			port = 5084,
		},

	},
	tcp = {
		tcp1 ={
			host = "127.0.0.1",
			port = 4140,
		}

	},

}

OWN_DIED = {
	http = {
		["p2p_newbie_guide"]                                    = { host = "192.168.71.151",		port = 7777 },
		["p2p_road_traffic"]                                    = { host = "192.168.71.151",		port = 7777 },
		["p2p_offsite_remind"]                                  = { host = "192.168.71.151",		port = 7777 },
		["p2p_fatigue_driving"]                                 = { host = "192.168.71.151",		port = 7777 },
		["p2p_over_speed"]                                      = { host = "192.168.71.151",		port = 7777 },
		["p2p_power_off"]					= { host = "192.168.71.151", 	port = 7777 },
		["p2p_power_on"]					= { host = "192.168.71.58", 	port = 7777 },
		["p2p_solarcalendar"]					= { host = "192.168.71.58", 	port = 7777 },
        
		----测试服务器URI(转发多台)
		['publicentry'] = { 
			--{host = '127.0.0.1', port = 8999},
		},

	},
}
