#[=========>MARK<=========]#
ifeq ($(HANDLE_MODEL), EVCORO)
LIBA += -levcs
endif

LIBA += -lkv -lscco -levcoro -lcoro -lev
LIBS += -lcrypto


INC_DIR += -I../../lib/libmini/include -I../../lib/libevcoro/include
