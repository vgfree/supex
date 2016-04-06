#include <stdlib.h>
#include <stdint.h>

enum router_object {
	CLIENT,
	MESSAGE_GATEWAY
};

struct CID {
        uint32_t IP;
	int fd;
};

struct router_head {
        uint8_t head_size;
	enum router_object message_from;
	enum router_object message_to;
        uint16_t CID_number;
	struct CID** cid;
        uint32_t body_size;
};
