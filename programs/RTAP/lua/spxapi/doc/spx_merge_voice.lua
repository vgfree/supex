
拼接多媒体文件
==================================

###API 编号
  *

###功能介绍
  * 用于拼接不同的语音文件

###参数格式

  * API以 **POST** 方式请求，且使用json表单方式提交,注意boundary



###输入参数

 参数           | 参数说明       | 类型        |   示例             | 是否允许为空   | 限制条件
----------------|----------------|-------------|--------------------|----------------|---------------------------
  appkey        | 应用标识       | string      | 1111111111         | 否             | 长度不大于10
  sign          | 安全签名       | string      | 无                 | 否             | 长度为40 ( 计算签名只需要lenght,appKey)
  parameter     | 语音文件路径   | json        | 无                 | 否             | 不允许为空


###parameter使用json数组提交,模板如下：

 参数           | 参数说明       | 类型        |   示例                 | 是否允许为空   | 参数含义
----------------|----------------|-------------|------------------------|----------------|---------------------------
 index          | 拼装的顺序     | string      |  1                     | 否             | 定义语音拼接的顺序
 msgType        | 拼接的类型     | string      |  link                  | 否             | 参数只限为：text,link,redis_variable,redis_fixation,mp3_binary,wav_binary,amr_binary和示例中的参数
 value          | 拼接的值       | string      |  http://xxx.amr        | 否             | 具体见下表中的参数
 default        | 默认拼接值     | string      |  空,仅限http开头链接    | 是             | value获取失败,使用default的值
 isAllowFailed  | 是否允许失败   | string      |  0，1                  | 否             | 0:表示当前元素如果提取失败.则直接返回提示api调用失败; 1:允许当前失败

###value分为不同的类型，具体含义如下：

 参数               | 参数说明                                   | 示例
--------------------|-----------------  -- ----------------------|-----------------------------------------------------------------------------------
 link               | 直接音频链接                               | http://test.abc.com/test.amr
 redis_variable     | 直接缓存类型/操作方式/已经提取的redis-key  | private/get/zHnlgrNpiq:nicknameURL(使用竖杠分割 | )
 redis_fixation     | 固定缓存字段,代码已经预设的redis-key       | 代码固定取accountID,nicknameURL, 是private的accountID:nicknameURL链接/private的groupID:channelNameUrl
 amr_binary         | form表单,包含二进制流                      | amr二进制流
 mp3_binary         | form表单,包含二进制流                      | mp3二进制流
 wav_binary         | form表单,包含二进制流                      | wav的二进制流

### 备注
只能出现一种类型的***_binary, ( amr_binary / mp3_binary / wav_binary  )

### 示例 
	[
	    {
	        "index": "1",
	        "msgType": "link",
	        "value": "http://192.168.71.84:2222/dfsapi/v2/gainSound?isStorage=false%26group=redis_1%26file=1452852482%3Adce68f5ebb6f11e5876500505697e0db.amr",
	        "default": "",
	        "filesize":"0",
	        "isAllowFailed": "0"
	    },
	    {
	        "index": "2",
	        "msgType": "redis_variable",
	        "value": "private|get|KbmwmGYbel:nicknameURL",
	        "default": "",
	        "filesize":"0",
	        "isAllowFailed": "1"
	    },
	    {
	        "index": "5",
	        "msgType": "redis_fixation",
	        "value": "qMkGm2sJue|nicknameURL",
	        "default": "",
	        "filesize":"0",
	        "isAllowFailed": "1"
	    }
	]


###示例代码/spx_merge_voice


	POST /spx_merge_voice HTTP/1.0
	Host:172.16.51.58:4090
	Content-Length:649
	Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryDcO1SS14275259593184720MTDfpuu1427525959kkU

	------WebKitFormBoundaryDcO1SS14275259593184720MTDfpuu1427525959kkU
	Content-Disposition: form-data; name="sign"

	B185CB492592CE90E58B11BCDF372C2618B8DA2
	-----WebKitFormBoundaryDcO1SS14275259593184720MTDfpuu1427525959kkU
	Content-Disposition: form-data; name="appKey"

	1111111111
	-----WebKitFormBoundaryDcO1SS14275259593184720MTDfpuu1427525959kkU
	Content-Disposition: form-data; name="parameter"

	[{"index":"1","msgType":"link","value":"http://192.168.71.84:2222/dfsapi/v2/gainSound?isStorage=false%26group=redis_1%26file=1452852482%3Adce68f5ebb6f11e5876500505697e0db.amr","default":"","filesize":"512","isAllowFailed":"1"}]
	-----WebKitFormBoundaryDcO1SS14275259593184720MTDfpuu1427525959kkU--


### 返回body示例

* 失败: `{"ERRORCODE":"ME01002", "RESULT":"appKey error"}`
* 成功: `{"ERRORCODE":"0", "RESULT":{"url":"http://v1.mmfile.daoke.me/dfsapi/v2/gainSound?group=group_1&file=1449918331:42549fcca0c011e5a58900505697e0db.amr","fileID":"b64f7158da8b11e3ae3b000c29b90439","fileSize":"143305"}}`


### 结果返回参数

  参数       | 参数说明
  url        | 新生成的文件表地址
  fileID     | 文件ID
  fileSize   | 文件大小


### 错误编码

 参数                 | 错误描述               | 解决办法
----------------------|------------------------|---------------------------------------
 0                    | 调用成功               |
 ME01002              | appKey error           | appKey需使用从语镜公司申请得到的appKey
 ME01019              | sign is not match      | 请阅读语镜公司提供的签名算法
 ME01023              | parameters is error    | 参数错误 请检查输入参数


### 测试地址: 172.16.51.58/spx_merge_voice



