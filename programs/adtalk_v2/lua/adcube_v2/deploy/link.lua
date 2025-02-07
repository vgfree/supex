module("link")

OWN_POOL = {
    redis = {
        public = {
            host = '192.168.1.11',
            port = 6349,
        },
        private1 = {
            host = '192.168.71.71',
            port = 6379,
        },
    },
    mysql = {},
}


OWN_DIED = {
    redis = {
        public = {
            host = '192.168.1.11',
            port = 6349,
        },
    },
    mysql = {},
    http = {
        ["weiboapi/v2/sendMultimediaPersonalWeibo"] = {
            host = "192.168.1.3",
            port = 8088,
        },
        ["dfsapi/v2/txt2voice"] = {
            host = "api.daoke.io",
            port = 80,
        },  
       IFQueryDistrictInfo = {
            host = 'api.daoke.io',
            port = 80,
      },
    },
}



