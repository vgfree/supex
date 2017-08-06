
openIMX := IMX_coreExchangeNode IMX_messageGateway IMX_loginServer IMX_settingServer IMX_userInfoApi IMX_appServer


IMX_coreExchangeNode:
	$(MAKE) -C ./IMX/CoreExchangeNode MAIN_APP_SERV=coreExchangeNode
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_messageGateway:
	$(MAKE) -C ./IMX/messageGateway MAIN_APP_SERV=messageGateway
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_loginServer:
	$(MAKE) -C ./IMX/loginServer MAIN_APP_SERV=loginServer
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_settingServer:
	$(MAKE) -C ./IMX/settingServer MAIN_APP_SERV=settingServer
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_userInfoApi:
	$(MAKE) -C ./IMX/UserInfoApi MAIN_APP_SERV=UserInfoApi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_appServer:
	$(MAKE) -C ./IMX/appServer MAIN_APP_SERV=appServer
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

clean:
	$(MAKE) -C ./IMX/CoreExchangeNode clean
	$(MAKE) -C ./IMX/messageGateway clean
	$(MAKE) -C ./IMX/settingServer clean
	$(MAKE) -C ./IMX/loginServer clean
	$(MAKE) -C ./IMX/UserInfoApi clean
	$(MAKE) -C ./IMX/appServer clean

distclean:
	$(MAKE) -C ./IMX/CoreExchangeNode distclean
	$(MAKE) -C ./IMX/messageGateway distclean
	$(MAKE) -C ./IMX/settingServer distclean
	$(MAKE) -C ./IMX/loginServer distclean
	$(MAKE) -C ./IMX/UserInfoApi distclean
	$(MAKE) -C ./IMX/appServer distclean
