#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include "cJSON.h"

int main(int argc, char **argv)
{
        // char text[] = "{\"aa\":22,\"bb\":\"\"}";
        // char text[] = "{\"srchost\" : [ \
        //         { \
        //                 \"name\" : \"a\",\
        //                 \"ip\" : \"127.0.0.1\",\
        //                 \"port\" : \"10001\",\
        //                 \"proto\" : \"http\"\
        //         },\
        //         {\
        //                 \"name\" : \"b\",\
        //                 \"ip\" : \"127.0 .0 .1\",\
        //                 \"port\" : \"10002\",\
        //                 \"proto\" : \"http\"\
        //         }\
        // ]}";
	char *text = NULL;
	char *out = NULL;
        
	assert(argc == 2);

        int fd = open(argv[1], O_RDONLY, 0);
        assert(fd > -1);
        
        off_t length = lseek(fd, 0, SEEK_END);
        assert(length > -1);
        
        lseek(fd, 0, SEEK_SET);

        text = calloc(length, sizeof(char));
        assert(text);
        
        ssize_t bytes = read(fd, text, length);
        assert(bytes > 0);
        
        cJSON *root = cJSON_Parse(text);

        if (!root) {
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        } else {
                out = cJSON_Print(root);
                printf("%s\n", out);
                free(out);

                // if (!cJSON_GetObjectItem(root, "cc")) {
                //         printf("NULL\n");
                // }

                // int aa = cJSON_GetObjectItem(root, "aa")->valueint;
                // printf("%d\n", aa);
                // char *bb = cJSON_GetObjectItem(root, "bb")->valuestring;
                // printf("%s\n", bb);
                // cJSON_Delete(root);
        }

        return EXIT_SUCCESS;
}