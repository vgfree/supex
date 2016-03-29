module("link")

OWN_POOL = {
	redis = {

		public = {
			host = '192.168.71.46',
			port = 6410,
		},
		match_road = {
            		host = '192.168.1.10',
           		 port = 5555
		},
		owner = {
			host = '192.168.71.47',
			port = 6400,
		},
		private_hash={
			hash = 'customer',
			{'M',"g1" ,'192.168.71.47', 6400, 30},
			{'M',"g2" ,'192.168.71.47', 6401, 30},
			{'M',"g3" ,'192.168.71.47', 6402, 30},
			{'M',"g4" ,'192.168.71.47', 6403, 30},
		},
		roadRelation = {
			host = '192.168.71.73',
			port =  4060,
		},
		mapRoadLine = {
			host = '192.168.71.65',
			port =  5602,
		},
		mapLineNode = {
			host = '192.168.71.65',
			port =  5601,
		},
		mapRoadInfo = {
			host = '192.168.71.65',
			port =  5603,
		},
		mapLandMark = {
			host = '192.168.71.65',
			port = 5531,
		},
		MapLineInRoad = {
			host = '192.168.71.65',
			port = 5605,
		},

		mapGPSData = {
			host = '192.168.71.61',
			port = 5301,
		},


	},
}


OWN_DIED = {
	http = {
		["p2p_fetch_4_miles_ahead_poi"]          = { 
			host = "192.168.71.58", 
			port = 7777,
		},
	},
}
