libIMX := libappsrv.a
openIMX := IMX_coreExchangeNode IMX_gateway IMX_userInfoApi IMX_appServer IMX_appClient
IMX:$(libIMX) $(openIMX)

libappsrv.a:
	$(MAKE) -C ./IMX/libappsrv lib
	mv ./IMX/libappsrv/lib/$@ ./IMX/




IMX_coreExchangeNode:
	$(MAKE) -C ./IMX/CoreExchangeNode MAIN_APP_SERV=coreExchangeNode
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_gateway:
	$(MAKE) -C ./IMX/gateway MAIN_APP_SERV=gateway
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_userInfoApi:
	$(MAKE) -C ./IMX/UserInfoApi MAIN_APP_SERV=UserInfoApi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_appServer:
	$(MAKE) -C ./IMX/appServer MAIN_APP_SERV=appServer
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_appClient:
	$(MAKE) -C ./IMX/appClient MAIN_APP_SERV=appClient
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


clean:
	$(MAKE) -C ./IMX/CoreExchangeNode clean
	$(MAKE) -C ./IMX/gateway clean
	$(MAKE) -C ./IMX/UserInfoApi clean
	$(MAKE) -C ./IMX/appServer clean
	$(MAKE) -C ./IMX/appClient clean
	$(MAKE) -C ./IMX/libappsrv clean
	rm -rf ./IMX/*.a

distclean:
	$(MAKE) -C ./IMX/CoreExchangeNode distclean
	$(MAKE) -C ./IMX/gateway distclean
	$(MAKE) -C ./IMX/UserInfoApi distclean
	$(MAKE) -C ./IMX/appServer distclean
	$(MAKE) -C ./IMX/appClient distclean
	$(MAKE) -C ./IMX/libappsrv distclean
	rm -rf ./IMX/*.a
