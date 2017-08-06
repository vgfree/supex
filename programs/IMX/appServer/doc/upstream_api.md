客户端开机上传json
0 upstream
1 CID
2 "bind"
3 {"uid":"UID"}

0 upstream
1 CID
2 {json}

bind消息上行
```json
{
	"opt": "bind", // *
	"accountID": "1234567890", //用户的accountID *
	"msgToken":"1234567890",  //登陆后获取的token
	"timestamp":"1234567890",  //当前时间戳 *
	"lat": 31.01,  //登陆时纬度
	"lon": 121.01  //登陆时经度
}
```
普通消息上行
```json
{
	"opt":"single_msg",   //普通消息类型 *
	"fromUser":"",  //发送用户 *
	"toUser":"",  //接收用户 *
	"holdTime":0,// 服务器留存时间，默认0为即时，否则为保存的时间秒数。
	"msgType":""//消息主体类型，由业务决定
	picture/text/html/voice/action *
	"msgObj":{
		"msg":"~~~~~~~~~~~"//消息体，由业务决定*
	}

}
```
群组消息上行
```json
{
	"opt":"group_msg",   //普通消息类型 *
	"fromUser":"",  //发送用户 *
	"toGroup":"",  //接收群组 groupID *
	"holdTime":0, // 服务器留存时间，默认0为即时，否则为保存的时间秒数。
	"msgType":"" //消息主体类型，由业务决定 *
	"msgObj":{
		"msg":"~~~~~~~~~~~"//消息体，由业务决定	*
	}
}
```

downstream
0 downstream
1 cid/uid/gid
2 CID/UID/GID
3 {json}

消息下行
```json
{
	"opt":"msg",   //消息类型 *
	"fromUser": "HJJHS55SD", //发送用户的accountID *
	"groupID":"HLKIPPOO",//消息来自的频道，如果是频道消息，不允许为空。*
	"msgTime":"",//消息发送时间 *
	"msgID":"",//最长64位
	"msgType":""//消息主体类型，由业务决定*
	"msgObj":{
		"msg":"~~~~~~~~~~~"//消息体，由业务决定	*
	}
}
```
