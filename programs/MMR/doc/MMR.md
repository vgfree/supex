####什么是MMR?
	Multitask Match Route,多任务匹配路由框架

####基于MMR的应用程序有哪些?
	drisamp,appcenter,MDP,passby

####数据流是怎么走的?
							MMR <======> autocode
							||
			+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			+			----------				+
	    config	+			|Reaction|				+
	    newstatus	+	  ------>drisamp|	 |------			+
			+	 /		|Dispath |	\			+
			+	/		----------	 \			+
Transit-----feadback----+----------------------------------------------->appcenter	+
			+	\							+
			+	 \-------------------------------------->passby		+
	    newstatus	+	  \------------------------------------->MDP		+
			+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

####autocode是什么?
	autocode是根据MMR的接口是为MMR框架下的应用实现:自动代码生成，控制管理应用模块的图形化工具

####MMR的模板是什么?
	MMR的模板是为autocode实现自动生成代码的中间产物

####MMR的模块是什么?
	MMR的模块是基于autocode自动生成或者人工手动编写的代码

####MMR的模式是什么?
	MMR的模式是决定一条数据投放到某个或某些模块上

####MMR模式有哪些?
	精准模式:当传递参数为a,b,c时，只有绑定a,b,c的模块才会被调用。
	部分模式:当传递参数为a,b,c时，绑定a,b的模块也会被调用。
	广播模式:传递参数无关，该模式下的模块都会被调用。
	点播模式:传递参数无关，该模式下的指定模块才会被调用,如果没有明确模块，不会调用任何模块。

####MMR模块由哪几部分组成?
	bind()函数，列举（精准）或（部分）模式[键值匹配]时绑定的键,（广播）和（点播）模式无意义。
	match()函数，进行[条件过滤]。
	work()函数，当满足时，触发执行工作。

####MMR有哪些原则?
	每个模块都有一个特有属性,就是能被直接调用
	app_name,app_mode分别指被调用的模块和被调用的模式
	当app_name传入值时，MMR框架根据app_name查询对应模块，并直接调用work()
	当app_name没有传入值时，MMR框架在[键值匹配]和[条件过滤]成功之后才会调用work()
	当app_mode传入值时，MMR框架会在对应的模式下处理
	当app_mode没有传入值时，MMR框架会在每个模式下处理

	
####MMR接口有哪些?

####autocode如何安装?


http://115.231.73.11/autocode/?service=drisamp
http://115.231.73.11/autocode/?service=appcenter
