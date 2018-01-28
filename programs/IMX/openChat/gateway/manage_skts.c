#include <string.h>
#include <assert.h>

#include "comm_utils.h"
#include "manage_skts.h"

void manage_skts_init(struct manage_skts *mng)
{
	memset(mng, 0, sizeof(*mng));
	pthread_rwlock_init(&mng->rwlock, NULL);
}

void manage_skts_free(struct manage_skts *mng)
{
	pthread_rwlock_destroy(&mng->rwlock);
}

void manage_skts_add(struct manage_skts *mng, int sockfd)
{
	pthread_rwlock_wrlock(&mng->rwlock);

	if (mng->used > MAX_MANAGE_SKTS) {
		loger("core exchange node:%d > max size:%d.", mng->used, MAX_MANAGE_SKTS);
		abort();
	}

	mng->skts[mng->used++] = sockfd;
	pthread_rwlock_unlock(&mng->rwlock);
}

void manage_skts_del(struct manage_skts *mng, int sockfd)
{
	pthread_rwlock_wrlock(&mng->rwlock);

	for (int i = 0; i < mng->used; i++) {
		if (mng->skts[i] == sockfd) {
			memcpy(&mng->skts[i], &mng->skts[i + 1], mng->used - i - 1);
			mng->used--;
			break;
		}
	}

	pthread_rwlock_unlock(&mng->rwlock);
}

int manage_skts_travel(struct manage_skts *mng, MANAGE_TRAVEL_FOR_LOOKUP_FCB lfcb, MANAGE_TRAVEL_FOR_DELETE_FCB dfcb, void *usr)
{
	if (dfcb) {
		pthread_rwlock_wrlock(&mng->rwlock);
	} else {
		pthread_rwlock_rdlock(&mng->rwlock);
	}

	int i = 0;
	int j = 0;
	for (; i < mng->used; i++) {
		j++;
		if (lfcb) {
			bool inherit = lfcb(mng->skts[i], i, usr);

			if (!inherit) {
				break;
			}
		}

		if (dfcb) {
			bool discard = dfcb(mng->skts[i], i, usr);

			if (discard) {
				memcpy(&mng->skts[i], &mng->skts[i + 1], mng->used - i - 1);
				mng->used--;
				i--;
			}
		}
	}

	pthread_rwlock_unlock(&mng->rwlock);
	return j;
}

