1、协议版本 vs:0x10
	1 个字节(高 4 位大版本、低 4 位小版本)
2、压缩格式&加密格式 fm:0x00(无加密无压缩)
	1 个字节(高 4 位压缩、低 4 位加密)
	[a] 高四位压缩格式
		0x0:无压缩
		0x1:ZIP
		0x2:GZIP
	[b] 低四位加密格式
		0x0:无加密
		0x1:IDEA
		0x2:AES
	[c] 数据组织
		先加密、再压缩
3、操作类型
	1 个字节
	0x00(握手)
	0x01(心跳)
	0x02(传输)
4、数据长度
	4 个字节(不包括头部 7 个字节,采用网络字节序)
5、实体数据
