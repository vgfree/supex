### path的前半部分是有请求是传入的

### 获取接口:

#path		/driviewFetch.json

1.获取所有模块列表
#body		{"operate":"get_all_was"}

#http result	{"l_position_remind":"xxxxx","w_test":"文本测试"}


#body		{"operate":"get_all_app","mode":"all"}

#http result	[{"name":"精准模式","value":{}},{"name":"部分模式","value":{"l_reply_dispatch":"open","l_first_point_dispatch":"null","l_driview_power_off":"close","l_first_mirrtalk_call":"open","l_driview_power_on":"open"}},{"name":"广播模式","value":{"w_test":"open"}},{"name":"点播模式","value":{}}]

*	先获取所有模块的别名，用别名显示.
*	把返回结果分解成3个列表"精确模式"，"部分模式","广播模式"
*	把每个模式下的模块名称显示出来（“value”的值）
*	每个模块名称对应一组开关,分别是"打开","关闭","加载","卸载"
*	当模块状态为"null":enable "加载",disable "打开" "关闭" "卸载"
*	当模块状态为"open":enable "卸载" "关闭",disable "打开" "加载"
*	当模块状态为"close":enable "卸载" "打开",disable "关闭" "加载"

*	"打开" 触发控制请求: status = "open"
*	"关闭" 触发控制请求: status = "close"
*	"加载" 触发控制请求: status = "insmod"
*	"卸载" 触发控制请求: sttaus = "rmmod"

2.获取某个模块详细信息
#body		{"operate":"get_app_cfg","appname":"w_test"}

#gttp result	{"appname":"w_test","config":{"bool":{"method1":{"speed":60,"time":"2014"}},"work":{"send_weibo":{"speed":60,"time":"2014"}}}}

*	appname 为某个具体的模块,根据实际点击或者搜索请求导入
*	把返回结果展示成新的页面,并且最底层key对应的value是可修改的
*	页面上要有个"保存"的按钮,当用户修改了参数,把所有修改后的数据（包括未修改的）,按json格式打包，并调用保存修改配置接口


3.获取某个模块说明信息
#body		{"operate":"get_app_exp","appname":"l_drive_status"}

#gttp result	{"appname":"l_drive_status","explain":{"bool":{"drive_online_hours":{}},"work":{"send_weibo":{"text":"[name],[road]","cause":"once,one_day"}}}}

*	该请求为调用["get_app_cfg"]同时触发的,获取到直接显示，不用可修改设置,是对"get_app_cfg"中的变量的说明，提示。


4.获取所有模板列表
		{"operate":"get_all_tmp"}

#http result	[{"name":"精准模式","value":{"movie":"哈哈哈"}},{"name":"部分模式","value":{"movie":"哈哈哈"}},{"name":"广播模式","value":{}}]

*	在主页面上有一个"新建"的按钮，点击后触发这个请求，并跳转生成新页面
*	把返回结果分解成3个列表"精确模板"，"部分模板","广播模板"
*	把每个模式下的模版名称显示出来（“value”的值）

5.获取所有参数列表
#body		'{"operate":"get_all_arg"}'

#http result	["passby","active","readBizid","mirrtalkNumber","speed","no","tokenCode","powerOn","yes","gz","gy","gx","GSENSORTime","powerOff","height","direction","IMEI","latitude","model","collect","longitude","news","call","randomCode","guest","GPSTime","subjoin","userid","sos"]

*	在模板列表界面上3种模式各有一个"新建模板"的按钮，点击后触发这个请求，并跳转生成新页面
*	设置一个标题框,方便用户输入模板名
*	把返回结果显示成列,没一项对应一个勾选框
*	在页面上有一个"创建"的按钮
*	当点击"创建",把用户所有勾选的项,打包成json格式,并调用创建模板接口
*	打包时mode为上一层进入的3种模式之一


6.获取模板参数列表信息
#body		'{"operate":"get_tmp_arg","mode":"local","tmpname":"movie"}'

#http result	{"args":[{"model":[{},{},["MT801","801C"],{}]},{"speed":[{},[0,180],{},["eq","ne","gt","lt","le","ge"]]},{"height":[{},[0,200],{},["eq","ne","gt","lt","le","ge"]]},{"collect":[[true],{},{},{}]},{"longitude":[{},{},{},["method1","method2"]]}],"format":["boolean","number","string","function"]}

*	在模板列表界面上每个模板各有一个"新建模块"的按钮，点击后触发这个请求，并跳转生成新页面
*	mode对应3中模式
*	设置一个标题框,方便用户输入模块名
*	把返回结果的每一项（如：{"height":[{},[0,200],{},["eq","ne","gt","lt","le","ge"]]}）展示成一行，每一行对应4列显示列,分别为"布尔值","数字","字符","函数"
*	默认显示的4列为该参数的4种属性，每种也有不同可选属性,把空的disable掉.
*	最下面还有一个执行函数列表勾选框
*	在页面上有一个"创建"的按钮
*	当点击"创建",把用户选择的配置,打包成json格式,并调用创建模块接口
*	打包"args"时,value以数组格式，数组第一个元素代表设置的列,"布尔值","数字","字符","函数"分别代表"boolean","number","string","function";数组第二个元素代表勾选项的内容.如果有填充值，填充的值放在数组的第三个元素.


7.获取执行任务列表
#body		'{"operate":"get_all_job","mode":"alone"}'

#http result	{"func":["send_weibo"]}

*	该请求为调用["get_tmp_arg"]同时触发的,获取后显示成一列多选的下拉框
*	当"创建"的时候，同["get_tmp_arg"]用户勾选的数据一起返回.


8.根据模板名搜索app
#body		'{"operate":"get_tmp_app","mode":"local","tmpname":"fatigue_driving_remind"}'

#http result	{"status":{"l_position_remind":"null","l_poit_remind":"null"},"alias":{"l_position_remind":"xxxxx","l_poit_remind":"xxxxx"}}

*	在主页面开一个搜索框







### 同步接口:

#path		/driviewMerge.json

9.模块控制
#body		{"operate":"ctl_one_app","status":"open","mode":"local","appname":"e_adtalk2"}

*	status对应4个值"open","close","insmod","rmmod"
*	appname为对应要控制的模块名

10.保存修改配置
#body		{"operate":"fix_app_cfg","appname":"w_test","config":{"bool":{"method1":{"speed":60,"time":"2014"}},"work":{"send_weibo":{"speed":60,"time":"2015"}}}}

11.创建模板
#body		{"operate":"new_one_tmp","mode":"whole","tmpname":"mode1","remarks":"测试","args":["model","speed","height","collect","longitude"]}

*	mode为模式，对应的值为从主页面的哪个入口进入，"whole" = "广播模式" , "local" = "部分模式" , "exact" = "精准模式"
*	tmpname为标题框里的内容,做下保护，标题框为空时报错

12.创建模块
#body		{"operate":"new_one_app","mode":"whole","appname":"w_test","tmpname":"fatigue_driving_remind","nickname":"文本测试","args":{"model":["string","801C"],"speed":["number",160],"height":["function","eq"],"collect":["boolean",true],"longitude":["function","method1"]},"func":["send_weibo"]}

*	mode为模式，对应的值为从主页面的哪个入口进入，"whole" = "广播模式" , "local" = "部分模式" , "exact" = "精准模式"
*	appname为标题框里的内容,做下保护，标题框为空时报错
