#/bin/sh
# generate test data for level DB

<<<<<<< HEAD
# key_len: key 长度, 目前固定为 26, 形式为 userid:timestamp:data, userid 长度为 10
# val_len/min: 单个用户每分钟的数据量, 目前为 12Kb
# min: 如果为 n, 那么将 n 分钟的数据拼接成一个 value
# user: 生成的用户个数
# day: 生成一段时间的用户数据, 30 天为一个月

#               key_len | val_len/min | min |		user 	| day
luajit gen.lua  26 			  12288 			  10 			10		  60  &
luajit gen.lua  26 			  12288 			  10 			2			  90	&
luajit gen.lua  26 			    10    			10 			2			  90	&
luajit gen.lua  26 			    11    			10 			2			  90	&
luajit gen.lua  26 			    12    			10 			2			  90	&
luajit gen.lua  26 			    13    			10 			2			  90	&
