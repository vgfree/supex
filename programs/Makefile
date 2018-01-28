libIMX := libappsrv.a
openIMX := IMX_openChat IMX_usrApi IMX_appSrv IMX_appCli
IMX:$(openIMX)

libappsrv.a:
	$(MAKE) -C ./IMX/libappsrv lib
	mv ./IMX/libappsrv/lib/$@ ./IMX/




IMX_openChat:
	$(MAKE) -C ./IMX/openChat MAIN_APP_SERV=openChat
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_usrApi:
	$(MAKE) -C ./IMX/usrApi MAIN_APP_SERV=usrApi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_appSrv:
	$(MAKE) -C ./IMX/appSrv MAIN_APP_SERV=appSrv
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

IMX_appCli:
	$(MAKE) -C ./IMX/appCli MAIN_APP_SERV=appCli
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


clean:
	$(MAKE) -C ./IMX/openChat clean
	$(MAKE) -C ./IMX/usrApi clean
	$(MAKE) -C ./IMX/appSrv clean
	$(MAKE) -C ./IMX/appCli clean

distclean:
	$(MAKE) -C ./IMX/openChat distclean
	$(MAKE) -C ./IMX/usrApi distclean
	$(MAKE) -C ./IMX/appSrv distclean
	$(MAKE) -C ./IMX/appCli distclean
