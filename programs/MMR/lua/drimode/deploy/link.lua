module("link")

OWN_POOL = {
	redis = {
	},

}

OWN_DIED = {
	http = {

		----测试服务器URI(转发多台)
		['publicentry'] = { 
			{host = '127.0.0.1', port = 8999},
		},

	},
}
