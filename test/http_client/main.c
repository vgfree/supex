#include "http.h"
#include <stdio.h>

void main()
{
	ssize_t size;
	char    data[] = "{\"ID\":\"c8XZjYq13y,460028272343752,358688000000158,cjdBSwSTI9,1,MT501\",\"GPS\":\"092000,12087.40736E,3111.00000N,0.00,0.557,450\",\"GSENSOR\":\"092000000,9.5129,0.1340,-0.8135\"}";

	/*get*/
	char *p = http_request(GET, "http://192.168.1.201/", &size, NULL, 0);

	printf("%s\n", p);
	free(p);

	/*post*/
	p = http_request(POST, "http://192.168.1.201/collectdata", &size, data, strlen(data));
	printf("%s\n", p);
	free(p);
}

