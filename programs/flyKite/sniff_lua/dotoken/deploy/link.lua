module(..., package.seeall)

OWN_POOL = {
    redis = {
        --地标POI
        mapLandMark = {
            host = '192.168.1.11',
            port = 6349,
            --host = '192.168.71.65',
            --port = 5531,        
        },
        private = {
            host = '192.168.1.11',
            port = 6349,
        },
        mapGridOnePercent = {
            host = '192.168.1.11',
            port = 6349,
            --host = '192.168.71.61',
            --port = 5071,
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
	owner = {
            host = '192.168.1.11',
            port = 6349,
	--	host = '192.168.71.66',
	--	port = 5091,
	},
	road = {
	    host = '192.168.1.10',
	    port = 5555,
	},
    },
    mysql = {
	dotoken = {
            host = '192.168.1.10',
            database = 'onlineComputation',
            user = 'gengxuanxuan',
            password = 'abc123',
	    table = 'mileageInfo201604'
	    
        },

    },
}

OWN_DIED = {
    mysql = {
    },

    redis = {

    },

    http = {
        addTravelInfo = {
            host = "192.168.1.13",
            port = 9098
        },
        getgps ={
            host = "123.159.205.72",
            --host = "tsdb.mirrtalk.com",
            port = 80, 
        }, 
    },
}


setmetatable(_M, { __index = _M })
