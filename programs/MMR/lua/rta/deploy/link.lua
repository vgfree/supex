module("link")

OWN_POOL = {
	redis = {
		public = {
			host = 'public.redis.daoke.com',
			port = 6349,
		},
		owner = {
			host = '127.0.0.1',
			port = 5080,
		},
		RTmiles = {
			host = '127.0.0.1',
			port = 5080,
		},
		RTtrack = {
			hash='consistent',		
			{'M','rttrack1', '172.16.51.54', 5082, 30},
                        {'M','rttrack2', '172.16.51.54', 5084, 30},
                        {'M','rttrack3', '172.16.51.95', 5082, 30},
                        {'M','rttrack4', '172.16.51.95', 5084, 30},
		},
		RTA = {
			hash='consistent',		
			{'M','rta1', '172.16.51.92', 5081, 30},
                        {'M','rta2', '172.16.51.92', 5083, 30},
                        {'M','rta3', '172.16.51.93', 5081, 30},
                        {'M','rta4', '172.16.51.93', 5083, 30},
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

		----测试服务器URI(转发多台)
		['publicentry'] = { 
		},

	},
}
