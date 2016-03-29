timport 设计文档
======

## 配置说明

* timport_conf.json文件配置项说明

```json
{
    "max_req_size": 33554432,
    "log_path": "./var/logs", 
    "log_file": "timport", 
    "log_level": 1,  
    "delay_time": 7200,                                 /* 导入延迟的时间 */
    "zk_disabled": 0,                                   /* 0：禁用zookeeper, 禁用后强制使用本配置中的`tsdb`服务器列表; 1：启动zookeeper */
    "zk_servers": "192.168.1.14:2181",                  /* zookeeper服务器 */
    "zk_rnode": "/tsdb",                                /* zookeeper中tsdb服务器配置信息所在的根节点 */
    "backup_path": "./var/backup",
    "start_time_file": "./var/start_time.txt", 
    "redis": [
        {    
            "host": "192.168.1.12",
            "port": 9001
        },    
        {    
            "host": "192.168.1.12",
            "port": 9002
        },    
        {    
            "host": "192.168.1.12",
            "port": 9003
        },    
        {    
            "host": "192.168.1.12",
            "port": 9004
        },    
        {    
            "host": "192.168.1.12",
            "port": 9005
        },    
        {    
            "host": "192.168.1.12",
            "port": 9006
        },    
        {    
            "host": "192.168.1.12",
            "port": 9007
        },    
        {    
            "host": "192.168.1.12",
            "port": 9008
        },    
        {    
            "host": "192.168.1.12",
            "port": 9009
        },    
        {    
            "host": "192.168.1.12",
            "port": 9010
        }
    ],  
    "tsdb": [                                           /* 当`zk_disabled`为0时，强制使用该配置下的tsdb；当`zk_disabled`为1时，需在`timport`配置项下指定`tsdb_in_cfg`才能使用该配置中的连接 */
        {
            "key_set": [0, 4096],                       /* key_set范围,左开右闭原则，即[0, 4096) */
            "data_set": [                               /* data_set 最多有2台互为主备（双master）的tsdb */
                {
                    "host": "192.168.1.15",
                    "port": 7020
                },
                {   "host": "192.168.1.15",
                    "port": 7022
                }
            ]
        },
        {
            "key_set": [4096, 8192],
            "data_set": [
                {
                    "host": "192.168.1.15",
                    "port": 7030
                }
            ]
        }
    ],
    "statistics": {                                     /* 目前只有gps/url数据有统计需求 */
        "host": "127.0.0.1",
        "port": 6329,
        "keys": [
            {
                "key": "gps",                           /* 统计key为`gps:20160121`，统计一天量的需INCR */
                "mode": "INCR",
            },
            {
                "key": "url",
                "mode": "INCR",
            },
            {
                "key": "activeuserCount",               /* 统计key未`activeuserCount:20160121144`,统计单十分钟，所以是直接SET */
                "mode": "SET",
            }
        ]
    },
    "timport": [
        {
            "key": "ACTIVEUSER",                        /* 总索引，与`interval`中的`TEN_MIN`共同拼成key, 如"ACTIVEUSER:20160121144" */
            "type": "WHOLE_INDEX",                      /* 类型，`WHOLE_INDEX`表示总索引，数据类型为SET，分布在不同redis中的key需要合并；`ALONE_INDEX`表示单索引(msgimport_conf.json),数据类型为SET，只分布在当前redis，无需合并结果；“KEYS_INDEX”表示通过`keys PREFIX:2016012114`来获取索引(simimport_conf.json)，无需固化至tsdb */
            "interval": "TEN_MIN",                      /* `TEN_MIN`: 表示时间以十分钟为单位，如“20160121144”；`ONE_HOUR`:表示时间以小时为单位，如"2016012114" */
            "expire_time": 7200,                        /* 当成功固化至tsdb后，设置该key在redis中的expire时间 */
            "statistics": "activeuserCount",            /* 该key `ACTIVEUSER` 下需要统计`activeuserCount`，必须保证已在`statistics`配置中有定义 */
            "child": [                                  /* 表示索引下需要固化的key */
                {
                    "key": "GPS",
                    "type": "SET_VAL",                  /* "SET_VAL": 表示这个是SET类型数据，需要用`SMEMBERS`来获取；“STRING_VAL”: 表示这个是STRING类型数据，需要用`GET`来获取数据 */
                    "param_cnt": 2,                     /* “2”：表示需要从索引获取的imei以及时间来拼成key; "1": 表示只需要从索引获取的imei来拼成key(暂时未出现该种情况，保留) */
                    "param_tm_pos": 1,                  /* "1": 表示将时间放在第二个冒号后拼成key，如“GPS:000000000000000:20160121150”；“0”：表示将时间放在第一个冒号后，如“GPS:20160121150:000000000000000” */
                    "hash_filter": "imei_hash_filter",  /* 所有filter函数都与`timport_filter.c`定义函数对应，毕竟业务不都是相同的，此处`imei_hash_filter` 表示从索引取到imei后根据8192取模决定将该key固化至相应的tsdb */
                    "statistics": "gps"                 /* 该key `GPS` 下需要统计`gps`,必须保证已在`statistics`配置中有定义 */
                },
                {
                    "key": "URL",
                    "type": "SET_VAL",
                    "param_cnt": 2,
                    "param_tm_pos": 1,
                    "hash_filter": "imei_hash_filter",
                    "ignore_error": 1,                  /* 兼容原有timport中的处理情况,在URL数据列数不一致时选择忽略错误，而不是直接退出进程 */
                    "tsdb_in_cfg": 1,                   /* 表示使用本配置中`tsdb`的服务器列表，而不用zookeeper中的tsdb列表 */
                    "statistics": "url"
                }
            ]
        },
        {
            "key": "RACTIVEUSER",
            "type": "WHOLE_INDEX",
            "interval": "TEN_MIN",
            "expire_time": 7200,
            "child": [
                {
                    "key": "RGPS",
                    "type": "SET_VAL",
                    "param_cnt": 2,
                    "param_tm_pos": 1,
                    "result_filter": "unique_result_filter",  
                    "hash_filter": "imei_hash_filter",
                    "statistics": "gps"
                },
                {
                    "key": "URL",
                    "type": "SET_VAL",
                    "param_cnt": 2,
                    "param_tm_pos": 1,
                    "hash_filter": "imei_hash_filter",
                    "ignore_error": 1,
                    "tsdb_in_cfg": 1,
                    "statistics": "url"
                }
            ]
        }
    ]
}
```

* msgimport_conf.json文件配置项说明
```
...
    "timport": [
        {
            "key": "sendMessage", 
            "type": "WHOLE_INDEX", 
            "interval": "TEN_MIN", 
            "expire_time": 3600, 
            "child": [
                {
                    "key": "sendMessage",                               /* 第二级索引, 所以`type`为“ALONE_INDEX”，key只存在单redis中，结果无需合并 */
                    "type": "ALONE_INDEX", 
                    "param_cnt": 2,  
                    "param_tm_pos": 1,
                    "hash_filter": "account_hash_filter",               /* “account_hash_filter” 表示用account_id来进行custom_hash运算后的值进行选择不同tsdb进行固化 */
                    "child": [
                        {
                            "key": "sendInfo", 
                            "type": "STRING_VAL",
                            "param_cnt": 1,
                            "key_filter": "timestamp_key_filter",       /* `key_filter`表示如何从上级索引中获取的结果来提取关键字，关键字最终用来决定从哪个redis取key和固化到哪个tsdb中去 */
                            "redis_filter": "key_redis_filter",         /* `redis_filter`表示根据关键字的hash值去哪个redis取数据，“key_redis_filter”表示用`key_filter`的结果来算hash值 */
                            "hash_filter": "key_hash_filter"            /* `hash_filter`表示根据关键字的hash值将该key固化至哪个tsdb, “key_hash_filter”表示用`key_filter`的结果来算hash值 */
                        },
                        {
                            "key": "fileLocation", 
                            "type": "STRING_VAL", 
                            "param_cnt": 1,
                            "key_filter": "timestamp_key_filter",
                            "redis_filter": "key_redis_filter",
                            "hash_filter": "key_hash_filter"
                        }
                    ]
                }
            ]
        },
...
```
