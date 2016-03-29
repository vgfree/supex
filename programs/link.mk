#[=========>MARK<=========]#
ifeq ($(HANDLE_MODEL), SCCO)
LIBA += -lsupex_scco
endif

ifeq ($(HANDLE_MODEL), LINE)
LIBA += -lsupex_line
endif

ifeq ($(HANDLE_MODEL), EVUV)
LIBA += -lsupex_evuv
endif

ifeq ($(HANDLE_MODEL), EVCORO)
LIBA += -lsupex_evcoro
endif

LIBA += -lev -lkv -lscco -levcoro -lcoro
LIBS += -lcrypto

#CFLAGS += -fprofile-arcs

INC_DIR += -I../../lib/libmini/include -I../../lib/libevcoro/include
