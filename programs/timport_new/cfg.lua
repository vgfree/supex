module("cfg")

USER_PART_KEY = "ACTIVEUSER"
time_interval = 10
migration_interval = 10
expire_time = 60

zk_servers = "192.168.1.14:2181"

timport =
{
	{
		zk_disabled = 0,
		zk_rnode = "/gps",
		key = "GPS:",
		data_type = "SET",
		hash_filter = "imei",
		tsdb = 
		{
			{
				key_set = 
				{
					0,
					2048,
				},
				name = "tsdb0",
			},
			{
				key_set = 
                                {
                                        2048,
                                        4096,
                                },
                                name = "tsdb1",
			},
			{
                                key_set =
                                {
                                        4096,
					6144,
                                },
                                name = "tsdb2",
                        },
			{
                                key_set =
                                {
                                        6144,
					8192,
                                },
                                name = "tsdb3",
                        },
		},
	},
	{
		zk_disabled = 0,
		zk_rnode = "/url",
		key = "URL:",
		data_type = "SET",
		hash_filter = "imei",
		tsdb =
                {
                        {
                                key_set = 
                                {
                                        0,
                                        2048,
                                },
                                name = "tsdb0",
                        },
                        {
                                key_set =
                                {
                                        2048,
                                        4096,
                                },
                                name = "tsdb1",
                        },
                        {
                                key_set =
                                {
                                        4096,
                                        6144,
                                },
                                name = "tsdb2",
                        },
                        {
                                key_set =
                                {
                                        6144,
                                        8192,
                                },
                                name = "tsdb3",
                        },
                },
	},
}
