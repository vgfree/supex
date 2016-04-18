module("link")

OWN_POOL = {
	redis = {
		public = {
			host = '192.168.1.11', 
			port = 6349,
		},

		private = {
			host = '192.168.1.11',
			port = 6319,
		},

		read_private = {
			host = '192.168.1.11',
			port = 6319,
		},


		statistic = {
			host = '192.168.1.11',
			port = 6339,
		},

		saveTokencode = {
			host = '192.168.1.11',
			port = 6339,
		},

		tsdb_hash = {
			-- hash模式consistent／customer
			-- hash = 'consistent',
			hash = 'customer',
                        {'M', 'a0', '192.168.1.12', 7776, 30},
                        {'M', 'a1', '192.168.1.12', 7777, 30},
                        {'M', 'a2', '192.168.1.12', 7778, 30},
                        {'M', 'a3', '192.168.1.12', 7779, 30},
                        {'M', 'a4', '192.168.1.12', 7776, 30},
                        {'M', 'a5', '192.168.1.12', 7777, 30},
                        {'M', 'a6', '192.168.1.12', 7778, 30},
                        {'M', 'a7', '192.168.1.12', 7779, 30},
                        {'M', 'a8', '192.168.1.12', 7776, 30},
                        {'M', 'a9', '192.168.1.12', 7777, 30},
		},

		url_hash = {
			-- hash模式consistent／customer
			-- hash = 'consistent',
			hash = 'customer',
                        {'M', 'a0', '192.168.1.12', 8000, 30},
                        {'M', 'a1', '192.168.1.12', 8001, 30},
                        {'M', 'a2', '192.168.1.12', 8002, 30},
                        {'M', 'a3', '192.168.1.12', 8003, 30},
                        {'M', 'a4', '192.168.1.12', 8004, 30},
                        {'M', 'a5', '192.168.1.12', 8000, 30},
                        {'M', 'a6', '192.168.1.12', 8001, 30},
                        {'M', 'a7', '192.168.1.12', 8002, 30},
                        {'M', 'a8', '192.168.1.12', 8003, 30},
                        {'M', 'a9', '192.168.1.12', 8004, 30},
		},
	
		message_hash = {
			-- hash模式consistent／customer
			-- hash = 'consistent',
			hash = 'customer',
                        {'M', 'b0', '192.168.1.12', 6379, 30},
                        {'M', 'b1', '192.168.1.12', 6379, 30},
                        {'M', 'b2', '192.168.1.12', 6379, 30},
                        {'M', 'b3', '192.168.1.12', 6379, 30},
		},
		---- 大坝redis协议
		damServer = {
			host = "192.168.71.56",
			--host = "192.168.71.71",
			port = 4210,
			--port = 6379,
		},
	},
	mysql = {
		app_ns___newStatus = {
			host = '192.168.1.6',
			port = 3306,
			database = 'newStatus',
			user = 'app_ns',
			password ='DKnsMSG110',
    		},

		---- adTalk 2015-06-05
		app_adtalk___readAdTalkInfo= {
			host = '192.168.1.6',
			port = 3306,
			database = 'app_adTalk',
			user = 'app_adtalk',
			password ='appaabc123',
		},

	},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {}
}
