#include "core_exchange_node_test.h"
#include "loger.h"

#include <string.h>
#include <stdio.h>

#define MAX_LINE_LEN 1024
char g_cmd_string[10][20] = {};
char g_buf[MAX_LINE_LEN] = {};
int g_cmd_num = 0;

static void split_cmd(char *buf)
{
  int len = strlen(buf);
  char element[20] = {};

  g_cmd_num = 0;
  int j = 0;
  for (int i = 0; i < len; i++) {
    if (buf[i] == ' ' || buf[i] == '\t') {
      if (element[0] != '\0') {
        strcpy(g_cmd_string[g_cmd_num++], element);
		memset(element, 0, 20);
      }
    }
	else {
      element[j] = buf[i];
	  j++;
    }
  }

  if (element[0] != '\0') {
    strcpy(g_cmd_string[g_cmd_num++], element);
  }
}

static void print_help()
{
  printf("Usage:\n");
  printf("Please typed fellow:\n");

  printf("function select: parse|pack|gateway|client|quit.\n");
}

static int exec_cmd()
{
  if (g_cmd_num == 0) {
    printf("\n ------------please cin -------------\n");
    return 0;	
  }
/*
  if (strcmp(g_cmd_string[0], "parse\0") == 0) {
    if (test_parse_function() == 0) {
      printf("\n -------parse function OK------------\n");	  
    }
	else {
      printf("\n -----------parse function failed-------------\n");
	}
  }
  else if (strcmp(g_cmd_string[0], "pack\0") == 0) {
    if (test_pack_function() == 0) {
      printf("\n ---------pack fucntion OK-----------\n");
    }
	else {
      printf("\n ---------pack function failed ---------------\n");
	}
  }*/
  else if (strcmp(g_cmd_string[0], "gateway\0") == 0) {   
    if (test_simulate_gateway() == 0) {
      printf("\n-----------simulate gateway function OK -------\n");
    }
	else {
      printf("\n---------simulate gateway function failed -------------\n");
	}
  }
  else if (strcmp(g_cmd_string[0], "client\0") == 0) {
    if (test_simulate_client() == 0) {
      printf("\n -------------simulate client function OK -----------\n");
	}
	else {
      printf("\n ------------simulate client function failed ----------\n");
	}
  }
  else if (strcmp(g_cmd_string[0], "quit\0") == 0) {
    printf ("\n----------------quit--------------\n");
	return -1;
  }
  else {
    print_help();
  }
  return 0;
}

struct CSLog *g_imlog = NULL;

int main(int argc, char *argv[])
{
  g_imlog = CSLog_create("test", WATCH_DELAY_TIME);
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }
  printf("\n---------------------------------tested start:----------------------------\n");
  while (1) {
    if (fgets(g_buf, MAX_LINE_LEN - 1, stdin) == NULL) {
      printf("error cin, again\n");
	  continue;
    }
	g_buf[strlen(g_buf) - 1] = '\0';

	split_cmd(g_buf);

	if (exec_cmd() == -1) {
      break;
	}		
  }
  CSLog_destroy(g_imlog);
  return 0;
}
