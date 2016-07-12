module("link")

OWN_POOL = {
	lhttp = {},
	redis = {},
	mysql = {},
	downstream = {
		host = '192.168.71.141',
		port = 8092,
	},
	setting = {
		host = '192.168.71.143',
		port = 8102,
	},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},
}



