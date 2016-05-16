module("link")

OWN_POOL = {
}

OWN_DIED = {
	redis = {},
	mysql = {},
	http = {                                                              
        	["dfsapi/v2/saveImage"] = {                 
                	host = "192.168.71.84",                                 
                	port = 2222,                                              
                },
		["dfsapi/v2/saveSound"] = {
		        host = "192.168.71.84",
                        port = 2222,		
		},
		["saveRtrPicBySgid"] = {
			host = "mapapi.daoke.me",
			port = 80,
		},
        }
}
