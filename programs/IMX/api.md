##appSrv:
#登入接口
	请求==>'{"action":"login","accountID":"??????"}'
	返回==>'{"action":"loginRsp","ERRORCODE":"0","RESULT":"success"}'

#心跳接口
	请求==>'{"action":"heart"}'


#加为好友
	请求==>'{"action":"amigoJoin","accountID":"??????","chatAmigoID":"??????"}'
	返回==>'{"action":"amigoJoinRsp","ERRORCODE":"0","RESULT":"success"}'

#获取好友
	请求==>'{"action":"amigos","accountID":"??????"}'
	返回==>'{"action":"amigosRsp","amigos":["??????","??????"]}'

#私聊接口
	请求==>'{"action":"chatAmigo","message":{"chatID":"??????","fromAccountID":"??????","chatAmigoID":"??????","content":"??????"}}'
	返回==>'{"action":"chatAmigoRsp","message":{"chatID":"??????","fromAccountID":"??????","chatAmigoID":"??????"}}'


#绑定群组
	请求==>'{"action":"groupJoin","accountID":"??????","chatGroupID":"??????"}'
	返回==>'{"action":"groupJoinRsp","ERRORCODE":"0","RESULT":"success"}'

#获取群组
	请求==>'{"action":"groups","accountID":"??????"}'
	返回==>'{"action":"groupsRsp","groups":["??????","??????"]}'

#群聊接口
	请求==>'{"action":"chatGroup","message":{"chatID":"??????","fromAccountID":"??????","chatGroupID":"??????","content":"??????"}}'
	返回==>'{"action":"chatGroupRsp","message":{"chatID":"??????","fromAccountID":"??????","chatGroupID":"??????"}}'

