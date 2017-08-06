module("link")

OWN_POOL = {
	redis = {
                IdKey = {
                        host = '127.0.0.1',
                        port = 6379,
                },

                GidKey = {
                        host = '127.0.0.1',
                        port = 6379,
                }
        },
}

OWN_DIED = {
	mysql = {},
	http = {                                                              
        }
}
