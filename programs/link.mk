#[=========>MARK<=========]#
ifeq ($(HANDLE_MODEL), EVCORO)
LIBA += -levcs
endif

LIBA += -lev -lkv -lscco -levcoro -lcoro
LIBS += -lcrypto


INC_DIR += -I../../lib/libmini/include -I../../lib/libevcoro/include
