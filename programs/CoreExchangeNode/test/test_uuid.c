#include <uuid/uuid.h>


void gen_uuid(char *dst)
{
	uuid_t  uuid;
	uuid_generate_time(uuid);
	uuid_unparse(uuid, dst);
}

void main(void)
{
	char    str[37] = { 0 };
	gen_uuid(str);
	printf("%s\n", str);
}
