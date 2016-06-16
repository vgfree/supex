#include "proto.h"

int main()
{
	int a = 10;

	struct list *list = proto_init();

	proto_append(list, (char *)&a, sizeof(int));
	proto_append(list, "hewke", strlen("hewke"));
	proto_append(list, "s&^%fsej", strlen("s&^%fsej"));
	proto_append(list, "hello", strlen("hello"));
	proto_append(list, "shanghainihao", strlen("shanghainihao"));
	proto_append(list, "010100100101010010100100101010010100101000100", strlen("010100100101010010100100101010010100101000100"));

	char *proto_str = proto_pack(list);
	proto_destroy(list);

	struct list *lst = proto_init();
	proto_parse(lst, proto_str);
	proto_get(lst);
	proto_destroy(lst);

	strfree(proto_str);

	return 0;
}

