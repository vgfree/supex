|  类别  | 子类别 | 中文名称                         | 文件名称                                 | 存放路径:          	| 当前状态 | 是否与线上版本一致 | 触发逻辑 |
| :----: | :-----:| :------------------------------: | :--------------------------------------: | :-------------------: | :------: | :--------: |
|        |        |超速情景		             |l_f_over_speed.lua	 	        |lua/drisamp/data/local/|可使用    |否| 	|当前速度超过限速 
|	 |	  |				     |						|			|          |  |  	|
|        |        |异地触发			     |l_f_home_offsite.lua      	   	|lua/drisamp/data/local/|可使用    |否|		|归属地到异地只触发一次，异地再到异地也只触发一次
|	 |	  |				     |						|			|          |  |		|
|        |        |开机      			     |e_f_power_on.lua				|lua/drisamp/data/exact/|可使用    |否|		|开机触发
|	 |	  |				     |						|			|          |  |		|
|        |drisamp |关机            		     |e_f_power_off.lua				|lua/drisamp/data/exact/|可使用    |否|		|关机触发
|	 |	  |				     |						|			|          |  |		|
|        |        |今日第一次开机情景                |e_f_oneday_boot.lua            	        |lua/drisamp/data/exact/|可使用    |否|		|每一天的第一次开机触发
|	 |	  |				     |						|			|          |  |		|
|情景    |        |历史第一次开机情景                |e_f_first_boot.lua       		        |lua/drisamp/data/exact/|可使用    |否|		|第一次开机触发
|	 |	  |				     |						|			|          |  |		|
|	 |	  |				     |						|			|          |  |		|
|        |	  |连续驾驶里程触发                  |l_f_continuous_driving_mileage.lua        |lua/rtmiles/data/local/|可使用    |否|		|十公里的倍数必定下发，5公里的倍数时并且下发间隔时间大于两分钟再次下发
|        |rtmiles |		                     |					        |			|          |  |		|
|        |        |疲劳驾驶触发                      |l_f_fatigue_driving.lua        	        |lua/rtmiles/data/local/|可使用    |否|		|用户连续驾驶超过4小时
|	 |	  |				     |						|			|          |  |		|
|	 |	  |				     |						|			|          |  |		|
|        |goby    |4公里poi触发                      |l_f_fetch_4_miles_ahead_poi.lua           |lua/goby/data/local/ 	|可使用    |否|		|
|	 |	  |				     |						|			|          |  |		|
|	 |	  |				     |						|			|          |  |		|
#####其他文件说明######
	monitor.lua： 情景监控
	
	freq.lua: 频率控制
	
	_fun_point_match_road.lua: 点在路上快速定位
	
	init.lua: 匹配路径
	
	judge.lua: 判断是否触发相应功能

	init_data.lua: 数据初始化

	cfg.lua: 日志级别

	link.lua: 要连接的redis、服务器的IP信息	

	EXACT_APP_LIST.lua: 控制exact项目开关

	LOCAL_APP_LIST.lua: 控制local项目开关
	
