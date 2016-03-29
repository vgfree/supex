#include <ev.h>
#include "libmini.h"

int main(int argc, char const *argv[])
{
	struct ev_loop  *defloop = NULL;
	struct ev_loop  *loop = NULL;

	defloop = ev_default_loop(EVFLAG_AUTO | EVBACKEND_POLL);

	loop = ev_loop_new(EVFLAG_AUTO | EVBACKEND_POLL);

	ev_loop_destroy(defloop);
	ev_loop_destroy(loop);
	/* code */
	return 0;
}

