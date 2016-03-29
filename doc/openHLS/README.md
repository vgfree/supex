#### 需要EasyDSS模块提供以下类似接口
	void *set_stream_to_chunk(char *name, int split_offset, char *chunk_buf, size_t chunk_size, function user_set_call_back);
	void *get_stream_by_chunk(char *name, int split_offset, char *chunk_buf, size_t &chunk_size, function user_get_call_back);





## user_get_call_back大致逻辑:
* 1.先从内存查找对应的chunk，找到直接返回给调用者。
* 2.找不到的话,用api接口从存储系统中查找出来再加载到本地cache中(设置过期时间)，同时返回给调用者。
## user_set_call_back大致逻辑:
* 1.加载到本地cache中(设置过期时间)
* 2.通过api接口写入存储系统中

## user_set_call_back和user_get_call_back函数，我这边会自行开发实现。




关于为什么要经过dfsapiServer，而不直接从dfsldbServer上拉数据，原因有四：
1.dfsldbServer上以磁盘存储为主，dfsapiServer会有内存缓存功能。
2.dfsldbServer可以接收数据直接入库，直接对外不安全，防止脏数据写入。
3.dfsapiServer可以通过常见的负载均衡组件前置来平行扩展，而不需要通过复制磁盘数据来扩容dfsldbServer抗压。
4.dfsapiServer上可以添加业务代码来实现特殊需求。




https://github.com/johnf/m3u8-segmenter.git
依赖:sudo yum install ffmpeg-devel.x86_64
