#pragma once

#include <libmini.h>

/******************************************/
#include <sys/syscall.h>/*此头必须带上*/

static inline pid_t gettid(void)
{
	return syscall(SYS_gettid);
}

/******************************************/

void *cpinit(void);

#define cpcall(fcb, ...)			 \
	({					 \
		bool volatile ok = true;	 \
		SKIP_TRY			 \
		{				 \
			fcb(##__VA_ARGS__);	 \
		}				 \
		CATCH				 \
		{				 \
			printf("SKIP_CATCH ...\n"); \
			ok = false;		 \
		}				 \
		FINALLY				 \
		{				 \
			printf("SKIP_FINALLY ...\n"); \
		}				 \
		END;				 \
		ok;				 \
	})

