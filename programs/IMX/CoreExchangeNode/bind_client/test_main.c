#include "core_exchange_node_test.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define MAX_LINE_LEN 1024

/*char g_cmd_string[10][20] = {};
 *   char g_buf[MAX_LINE_LEN] = {};
 *   int g_cmd_num = 0;
 *
 *   static void split_cmd(char *buf)
 *   {
 *   int len = strlen(buf);
 *   char element[20] = {};
 *
 *   g_cmd_num = 0;
 *   int j = 0;
 *   for (int i = 0; i < len; i++) {
 *    if (buf[i] == ' ' || buf[i] == '\t') {
 *      if (element[0] != '\0') {
 *        strcpy(g_cmd_string[g_cmd_num++], element);
 *                memset(element, 0, 20);
 *      }
 *    }
 *        else {
 *      element[j] = buf[i];
 *          j++;
 *    }
 *   }
 *
 *   if (element[0] != '\0') {
 *    strcpy(g_cmd_string[g_cmd_num++], element);
 *   }
 *   }
 *
 *   static void print_help()
 *   {
 *   printf("Usage:\n");
 *   printf("Please typed fellow:\n");
 *
 *   printf("function select: uidmap|gidmap|gateway|client|quit.\n");
 *   }
 *
 *   static int exec_cmd()
 *   {
 *   if (g_cmd_num == 0) {
 *    printf("\n ------------please cin -------------\n");
 *    return 0;
 *   }
 *   if (strcmp(g_cmd_string[0], "uidmap\0") == 0) {
 *    if (test_uid_map() == 0) {
 *      printf("\n -------tested uidmap OK------------\n");
 *    }
 *        else {
 *      printf("\n -----------tested uidmap failed-------------\n");
 *        }
 *   }
 *   else if (strcmp(g_cmd_string[0], "gidmap\0") == 0) {
 *    if (test_gid_map() == 0) {
 *      printf("\n ---------tested gidmap  OK-----------\n");
 *    }
 *        else {
 *      printf("\n ---------tested gidmap failed ---------------\n");
 *        }
 *   }
 *   else if (strcmp(g_cmd_string[0], "gateway\0") == 0) {
 *    if (test_simulate_gateway() == 0) {
 *      printf("\n-----------simulate gateway function OK -------\n");
 *    }
 *        else {
 *      printf("\n---------simulate gateway function failed -------------\n");
 *        }
 *   }
 *   else if (strcmp(g_cmd_string[0], "client\0") == 0) {
 *    if (test_simulate_client() == 0) {
 *      printf("\n -------------simulate client function OK -----------\n");
 *        }
 *        else {
 *      printf("\n ------------simulate client function failed ----------\n");
 *        }
 *   }
 *   else if (strcmp(g_cmd_string[0], "quit\0") == 0) {
 *    printf ("\n----------------quit--------------\n");
 *        return -1;
 *   }
 *   else {
 *    print_help();
 *   }
 *   return 0;
 *   }*/

// struct CSLog *g_imlog = NULL;

void *multi_client(void *usr)
{
	test_simulate_client((char *)usr);
}

int main(int argc, char *argv[])
{
	//  g_imlog = CSLog_create("test", WATCH_DELAY_TIME);

	/*for (int i = 0; i < argc; i++) {
	 *   printf("%s\n", argv[i]);
	 *   }
	 *   printf("\n---------------------------------tested start:----------------------------\n");
	 *   while (1) {
	 *   if (fgets(g_buf, MAX_LINE_LEN - 1, stdin) == NULL) {
	 *    printf("error cin, again\n");
	 *        continue;
	 *   }
	 *      g_buf[strlen(g_buf) - 1] = '\0';
	 *
	 *      split_cmd(g_buf);
	 *
	 *      if (exec_cmd() == -1) {
	 *    break;
	 *      }
	 *   }*/
	//  CSLog_destroy(g_imlog);
	if (argc != 3) {
		printf("arg not right\n");
		return -1;
	}

	int i;

	for (i = 0; i < atoi(argv[1]); i++) {
		pthread_t tid;
		assert(pthread_create(&tid, NULL, multi_client, argv[2]) == 0);
	}

	while (1) {}

	return 0;
}

