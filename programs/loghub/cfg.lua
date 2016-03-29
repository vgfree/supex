local cfg = {
	LOGLV = 0,
	LOGSV = "/data/logs/",

		--| <daokelogshub>
		LOG_HUB_APPEND = false,-->>是否保存到本地
		LOG_HUB_NOTIFY = true,-->>是否发送邮件
		LOG_HUB_PRINTF = true,-->>是否打印
		LOG_HUB_FILTER = 1,	-->> {[debug] = 1, [info] = 2, [warnning] = 3,[error] = 4, [system] = 5,[fatal] = 6,}
		LOG_HUB_MAXLEN = 200,--50,
		
		--|接收邮件地址tab
		API_OWER_EMAIL ={
	--		xx1 ="<xx1@mirrtalk.com>",
	--		xx2	= "<xx2@mirrtalk.com>",
	--		xx3 = "<xx3@mirrtalk.com>",
	--		lipengwei = "<lipengwei@mirrtalk.com>",
		},
}
return cfg
