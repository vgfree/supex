#pragma once

#include <libmini.h>


void *cpinit(void);

#define cpcall(fcb, ...)			\
	({	 \
	 	bool ok = true;				\
		SKIP_TRY				\
		{				\
			fcb(##__VA_ARGS__);	\
		}				\
		SKIP_CATCH				\
		{				\
			printf("CATCH ...\n");	\
	 		ok = false;		\
		}				\
		SKIP_FINALLY				\
		{				\
			printf("FINALLY ...\n");\
		}				\
		SKIP_END;				\
	 	ok;				\
	})
