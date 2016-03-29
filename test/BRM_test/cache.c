#include <stdio.h>
#include <string.h>

#include "cache.h"

#include "libmini.h"

static bool _resize_cache(struct netdata_cache *netdata);

bool netdata_cache_init(struct netdata_cache *netdata)
{
	return_val_if_fail(netdata, false);

	memset(netdata, 0, sizeof(*netdata));
	netdata->data_buf = netdata->bufbase;
	netdata->size = CACHE_DEF_SIZE;

	return true;
}

void netdata_cache_free(struct netdata_cache *netdata)
{
	if (likely(netdata && (netdata->data_buf != netdata->bufbase))) {
		_free(netdata->data_buf);
	}
}

bool netdata_cache_add(struct netdata_cache *netdata, const char *buf, int size)
{
	return_val_if_fail(netdata && buf, false);

	if (unlikely((netdata->offset + size) > netdata->size)) {
		if (unlikely(!_resize_cache(netdata))) {
			return false;
		}
	}

	memcpy(netdata->data_buf + netdata->offset, buf, size);
	netdata->offset += size;

	return true;
}

void netdata_cache_reset(struct netdata_cache *netdata)
{
	return_if_fail(netdata);

	if (netdata->data_buf != netdata->bufbase) {
		_free(netdata->data_buf);
	}

	memset(netdata, 0, sizeof(*netdata));
	netdata->data_buf = netdata->bufbase;
	netdata->size = CACHE_DEF_SIZE;
}

static bool _resize_cache(struct netdata_cache *netdata)
{
	char    *old = NULL;
	char    *outsize = NULL;

	return_val_if_fail(netdata, false);
	old = netdata->data_buf;

	if (unlikely((netdata->size + netdata->size) > CACHE_MAX_SIZE)) {
		x_printf(I, "no more memery for net data cache,begin release some cache for new data");

		if (netdata->outsize != netdata->offset) {
			// there have some massage we didn't send out yet
			outsize = netdata->data_buf + netdata->outsize;
		}
	}

	_new_array((netdata->size + netdata->size), netdata->data_buf);

	if (likely(netdata->data_buf)) {
		netdata->size += netdata->size;

		if (unlikely(outsize)) {
			memcpy(netdata->data_buf, outsize, netdata->offset - netdata->outsize);
			netdata->offset = netdata->offset - netdata->outsize;
		} else {
			memcpy(netdata->data_buf, old, netdata->offset);
		}

		if (old != netdata->bufbase) {
			_free(old);
		}

		return true;
	} else {
		netdata->data_buf = old;
		return false;
	}
}

