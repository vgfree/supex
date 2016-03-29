module("link")

OWN_POOL = {
	lhttp = {},
	redis = {
	},
	mysql = {
	},
}

OWN_DIED = {
	redis = {
		public = {
			host = '192.168.1.11',
			port = 6349,
		},
		tmpvoice = {
			host = '192.168.1.11',
			port = 6349,
		},
--		tmpvoice = {
--			host = '172.16.31.186',
--			port = 9023,
--		},
	},
	mysql = {},
	tcp = {
	},
	http = {
--		wsapiServer = {
--			host = "192.168.1.21",
--			port = 8000
--		},
		jTTSServer = {
			host = "192.168.1.9",
			port = 3000,
		},   
		dfsSaveSound = {
			host = "192.168.71.84",
			port = 2222,
		},
		
	},
}
