#include <assert.h>
#include <string.h>

int remove_first_nframe(int nframe, struct comm_message *msg)
{
	if (nframe > msg->package.frames) {
		//    error("nframe:%d > msg->package.frames:%d.", nframe, msg->package.frames);
	} else if (nframe == msg->package.frames) {
		memset(&msg->package, 0, sizeof(msg->package));
		msg->package.packages = 1;
		return 0;
	}

	int rmsz = msg->package.frame_offset[nframe];
	msg->package.raw_data.len -= rmsz;
	memmove(msg->package.raw_data.str, msg->package.raw_data.str + rmsz,
		msg->package.raw_data.len);

	int i = 0;

	for (i = 0; (i + nframe) < msg->package.frames; i++) {
		msg->package.frame_size[i] = msg->package.frame_size[nframe + i];
		msg->package.frame_offset[i] = msg->package.frame_offset[nframe + i] - rmsz;
	}

	msg->package.frames -= nframe;
	msg->package.frames_of_package[0] -= nframe;
	return rmsz;
}
