module("link")

OWN_POOL = {
	lhttp = {},
	redis = {
		-----------------------------------------------------------------------------------------------------
		--dfsdb amr write port
		dfsdb_amr_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
		},
		--dfsdb amr read port
		dfsdb_amr_read = {
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
                        {'M', 'dfsdb_3', '127.0.0.1', 7506, 30},
                        {'M', 'dfsdb_4', '127.0.0.1', 7508, 30},
		},

		--dfsdb mp4 write port 
		dfsdb_mp4_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
		},

		--dfsdb mp4 read port
		dfsdb_mp4_read = {	
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
                        {'M', 'dfsdb_3', '127.0.0.1', 7506, 30},
                        {'M', 'dfsdb_4', '127.0.0.1', 7508, 30},
		},
		
		--dfsdb jpg write port 
		dfsdb_jpg_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
		},

		--dfsdb jpg read port
		dfsdb_jpg_read = {	
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
                        {'M', 'dfsdb_3', '127.0.0.1', 7506, 30},
                        {'M', 'dfsdb_4', '127.0.0.1', 7508, 30},
		},
		
		--dfsdb file write port
		dfsdb_file_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
		},
		--dfsdb file read port
		dfsdb_file_read = {
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '127.0.0.1', 7501, 30},
                        {'M', 'dfsdb_2', '127.0.0.1', 7503, 30},
                        {'M', 'dfsdb_3', '127.0.0.1', 7506, 30},
                        {'M', 'dfsdb_4', '127.0.0.1', 7508, 30},
		},

		--dfsdb write spare port
		dfsdb_write_spare = {
			hash = 'customer',
			{'M', 'dfsdb_bak', '127.0.0.1', 7501, 30},
		},
		--dfsdb read spare port
		dfsdb_read_spare = {
			hash = 'appoint',
			{'M', 'dfsdb_bak', '127.0.0.1', 7501, 30},
		},
	

		--redis amr write port
		redis_amr_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},
		--redis amr read port
		redis_amr_read = {
			 hash = 'appoint',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},

		--redis mp4 write port 
		redis_mp4_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},

		--redis mp4 read port
		redis_mp4_read = {	
			 hash = 'appoint',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},
		
		--redis jpg write port 
		redis_jpg_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},

		--redis jpg read port
		redis_jpg_read = {	
			 hash = 'appoint',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},
		
		--redis file write port
		redis_file_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},
		--redis file read port
		redis_file_read = {
			 hash = 'appoint',
                        {'M', 'redis_1', '127.0.0.1', 7502, 30},
                        {'M', 'redis_2', '127.0.0.1', 7504, 30},
		},

		--dfsdb write spare port
		redis_write_spare = {
			hash = 'customer',
			{'M', 'redis_bak', '127.0.0.1', 7502, 30},
		},
		--dfsdb read spare port
		redis_read_spare = {
			hash = 'appoint',
			{'M', 'redis_bak', '127.0.0.1', 7502, 30},
		},
		----------------------------------------------------------------------------------------------------------
--		tmpamr = {
--			host = '172.16.31.186',
--			port = 9023,
--		},
	},
	mysql = {
		--[[
		app_dfs___fastDFS = {
			host = '192.168.1.6',
			port = 3306,
			database = 'app_fastDFS',
			user = 'app_dfs',
			password ='dk110FastDFS',
		},
		]]--
	},
}

OWN_DIED = {
	redis = {
		private = {
			host = '127.0.0.1',
			port = 6379,
		},
		public = {
			host = '127.0.0.1',
			port = 6379,
		},
	},
	mysql = {},
	http = {
		["dfsapi/v2/mp32voice"] = {
			host = "192.168.1.207",
			--host = "192.168.1.6",
			port = 80,
		},
	},
}
