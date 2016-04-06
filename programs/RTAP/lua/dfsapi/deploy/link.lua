module("link")

OWN_POOL = {
	redis = {
		-----------------------------------------------------------------------------------------------------
		--dfsdb amr write port
		dfsdb_amr_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '192.168.1.12', 7501, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7503, 30},
		},
		--dfsdb amr read port
		dfsdb_amr_read = {
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '192.168.1.12', 7502, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7504, 30},
		},

		--dfsdb mp4 write port 
		dfsdb_mp4_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '192.168.1.12', 7501, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7503, 30},
		},

		--dfsdb mp4 read port
		dfsdb_mp4_read = {	
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '192.168.1.12', 7502, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7504, 30},
		},
		
		
		--dfsdb jpg write port 
		dfsdb_jpg_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '192.168.1.12', 7501, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7503, 30},
		},

		--dfsdb jpg read port
		dfsdb_jpg_read = {	
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '192.168.1.12', 7501, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7503, 30},
		},
		
		--dfsdb file write port
		dfsdb_file_write = {
			 hash = 'customer',
                        {'M', 'dfsdb_1', '192.168.1.12', 7501, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7503, 30},
		},
		--dfsdb file read port
		dfsdb_file_read = {
			 hash = 'appoint',
                        {'M', 'dfsdb_1', '192.168.1.12', 7502, 30},
                        {'M', 'dfsdb_2', '192.168.1.12', 7504, 30},
		},

		--dfsdb write spare port
		dfsdb_write_spare = {
			 hash = 'customer',
                        {'M', 'dfsdb_bak', '192.168.1.12', 7501, 30},
		},
		--dfsdb read spare port
		dfsdb_read_spare = {
			hash = 'appoint',
                        {'M', 'dfsdb_bak', '192.168.1.12', 7502, 30}, 
		},
		
		-- ssdb file spare port
		ssdb_file_spare = {
			 hash = 'customer',
                        {'M', 'ssdb_bak', '192.168.71.84', 8890, 30},
		},
 		-- ssdb mp4 spare port
		ssdb_mp4_spare = {
			 hash = 'customer',
                        {'M', 'ssdb_bak', '192.168.71.84', 8891, 30},
		},
		-- ssdb amr spare port
		ssdb_amr_spare = {
			 hash = 'customer',
                        {'M', 'ssdb_bak', '192.168.71.84', 8892, 30},
		},
		-- ssdb img spare port
		ssdb_img_spare = {
			 hash = 'customer',
                        {'M', 'ssdb_bak', '192.168.71.84', 8893, 30},
		},
	
		--redis amr write port
		redis_amr_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},
		--redis amr read port
		redis_amr_read = {
			 hash = 'appoint',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},

		--redis mp4 write port 
		redis_mp4_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},

		--redis mp4 read port
		redis_mp4_read = {	
			 hash = 'appoint',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},
		
		--dfsdb jpg write port 
		redis_jpg_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},

		--dfsdb jpg read port
		redis_jpg_read = {	
			 hash = 'appoint',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},
		
		--dfsdb file write port
		redis_file_write = {
			 hash = 'customer',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},
		--dfsdb file read port
		redis_file_read = {
			 hash = 'appoint',
                        {'M', 'redis_1', '192.168.1.12', 7505, 30},
                        {'M', 'redis_2', '192.168.1.12', 7507, 30},
		},

		--dfsdb write spare port
		redis_write_spare = {
			 hash = 'customer',
                        {'M', 'redis_bak', '192.168.1.12', 7505, 30},
		},
		--dfsdb read spare port
		redis_read_spare = {
			 hash = 'appoint',
                        {'M', 'redis_bak', '192.168.1.12', 7505, 30},
		},

                -- redis spare file,amr,mp4,jpg
		redis_file_spare = {
			hash = 'customer',
	       		{'M', 'redis_bak', '192.168.1.12', 7505, 30},
      		},
		redis_amr_spare = {
			 hash = 'customer',
                        {'M', 'redis_bak', '192.168.1.12', 7505, 30},
		},
		redis_mp4_spare = {
			 hash = 'customer',
                        {'M', 'redis_bak', '192.168.1.12', 7505, 30},
		},
		redis_img_spare = {
			 hash = 'customer',
                        {'M', 'redis_bak', '192.168.1.12', 7505, 30},
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
			host = '192.168.1.12',
			port = 6379,
		},
		public = {
			host = '192.168.1.12',
			port = 6379,
		},
		tmpamr = {
			host = '192.168.1.12',
			port = 6379,
		},
		tmpvideo = {
			host = '192.168.1.12',
			port = 6379,
		},
		tmpimage = {
			host = '192.168.1.12',
			port = 6379,
		},
		redis_bak = {
			host = '192.168.1.12',
			port = 6379,
		},
	},
	mysql = {},
	http = {
		jTTSServer = {
			host = "192.168.1.9",
			port = 3000,
		},
		["dfsapi/v2/mp32voice"] = {
			host = "192.168.1.207",
			--host = "192.168.1.6",
			port = 80,
		},
	},
}
