
		配置加载[v]
		   |
		   |							   /--->超时踢除[x]
事件控制组件:	   v	    /---->主线程事件挂载[v] ---->监听客户端连接[v]/		      |---->mfptp协议解析[v]
		流程框架[v]/						  \		      |---->记录用户数据和连接映射表[v]
			   \						   \--->认证阶段[?]---|---->mfptp协议封包[x]
			    \---->子线程事件挂载[v]					      |---->发送缓冲区添加认证成功数据[v]
					|								|
					|								|
				-----------------							|
				|		|							|
				v		v							|
			------>io写[v]		io读[v] <----管道注入/转接[v] <----哈希用户[x]<---------|
			|			|							|
		提取task任务,添加发送缓冲区[?]	v							|
			^		mfptp协议解析[v]----------------------------------------->异常终止[v]
			|			|
		worker任务队列注入[v]		v
			^		转化/封装zmq消息[x]
		   	|			|
		   哈希用户[x]			v
			^		使用worker的zmq套接字push消息[x]
			|			|
消息控制组件:		|			v
			|	     	       |导出|
			| zmq组件的初始化[x]---|--->zmq代理线程的创建[x]
			|	     	       |--->zmq ipc套接字pull创建及bind[x]
			|	      	       |--->zmq tcp套接字push创建及bind[x]--------------------------------->暂时丢弃消息
			|	     	       |导入|
			|	     	       |--->zmq tcp套接字pull创建及bind[x]<----------------------------------外部系统情景消息导入
			|				           |
			|	                                   -------------
			|							|
			|	   			   		 批量消息线程
			|							|
			|							|
			|	转化成mfptp协议数据流[x]<------------ 		|
			|		|		      \	    |		|
			-----------------		       \    |		|
								\   |		|
								 \  |		|
								  \ |		|
信息控制组件:							   \|		|
								    |	   zmq_poll轮询--------zmq tcp套接字sub创建及bind---------用户信息增/删/改/查
								    |		|
					    sqlite存储		    |		|
						|		    |		v
		用户信息初始化导入[x]------>组织数据结构-----|--->提供情景快速查询接口[x]
						|	     |--->提供个人订阅查询接口[x]
						|
				-----------------------------------------
				|					|
		rbtree建立user_node: |appKey号|head记录地址|     rbtree建立scene_node: |情景号|head记录地址|
		
	一条记录: |user2地址|user2的上一条记录地址|user2的下一条记录地址|scene1地址|scene1的上一条记录地址|scene1的下一条记录地址|
	一条记录: |user1地址|user1的上一条记录地址|user1的下一条记录地址|scene1地址|scene1的上一条记录地址|scene1的下一条记录地址|
	一条记录: |user1地址|user1的上一条记录地址|user1的下一条记录地址|scene2地址|scene2的上一条记录地址|scene2的下一条记录地址|

