libIMX := libappsrv.a
openIMX := IMX_openChat IMX_userInfoApi IMX_appServer IMX_appClient
IMX:$(libIMX) $(openIMX)

libappsrv.a:
	$(MAKE) -C ./IMX/libappsrv lib
	mv ./IMX/libappsrv/lib/$@ ./IMX/




IMX_openChat:
	$(MAKE) -C ./IMX/openChat MAIN_APP_SERV=openChat
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
	$(MAKE) -C ./IMX/openChat clean
	$(MAKE) -C ./IMX/UserInfoApi clean
	$(MAKE) -C ./IMX/appServer clean
	$(MAKE) -C ./IMX/appClient clean
	$(MAKE) -C ./IMX/libappsrv clean
	rm -rf ./IMX/*.a

distclean:
	$(MAKE) -C ./IMX/openChat distclean
	$(MAKE) -C ./IMX/UserInfoApi distclean
	$(MAKE) -C ./IMX/appServer distclean
	$(MAKE) -C ./IMX/appClient distclean
	$(MAKE) -C ./IMX/libappsrv distclean
	rm -rf ./IMX/*.a
