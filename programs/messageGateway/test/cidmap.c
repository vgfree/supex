#include "cidmap.h"

#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

static struct cidmap g_cid_map;

static int g_index = 0;

void init_cidmap()
{
  g_cid_map.cid_number = 0;
  g_cid_map.current_offset = 0;
  g_cid_map.buf = (char *)malloc(MAX_SIZE * CID_MAX_LENTH * sizeof(char));
}

void destroy_cidmap()
{
  free(g_cid_map.buf);
}

char *get_first_cid()
{
  g_index = 0;
  if (g_index >= g_cid_map.cid_number ) {
    return NULL;
  }
  
  int size = g_cid_map.cid[g_index]._end - g_cid_map.cid[g_index]._front + 1;
  char *cid = malloc(size * sizeof(char));

  memset(cid, 0, size);
  memcpy(cid, g_cid_map.buf + g_cid_map.cid[g_index]._front, size - 1);
  return cid;
}

char *get_next_cid()
{
  g_index++;
  if (g_index >= g_cid_map.cid_number) {
    return NULL;
  }
  
  int size = g_cid_map.cid[g_index]._end - g_cid_map.cid[g_index]._front + 1;
  char *cid = malloc(size * sizeof(char));

  memset(cid, 0, size);
  memcpy(cid, g_cid_map.buf + g_cid_map.cid[g_index]._front, size - 1);
  return cid;
}

int append_cid(char *cid, int size)
{
  assert(cid);
  assert(g_cid_map.cid_number < MAX_SIZE);
  memcpy(g_cid_map.buf + g_cid_map.current_offset, cid, size);
  g_cid_map.cid[g_cid_map.cid_number]._front = g_cid_map.current_offset;
  g_cid_map.cid[g_cid_map.cid_number]._end = g_cid_map.current_offset + size;
  g_cid_map.current_offset = g_cid_map.cid[g_cid_map.cid_number]._end;
  g_cid_map.cid_number++;
  return 0;
}

int remove_cid(char *cid, int size)
{
  int i = 0;
  for (; i < g_cid_map.cid_number; i++) {
    if (memcmp(g_cid_map.buf + g_cid_map.cid[i]._front, cid, size) == 0) {
      memmove(g_cid_map.buf + g_cid_map.cid[i]._front,
              g_cid_map.buf + g_cid_map.cid[i]._end,
              g_cid_map.current_offset - g_cid_map.cid[i]._end);
	  break;
	}
  }
  if (i == g_cid_map.cid_number) {
    return -1;
  }
  for (; i < g_cid_map.cid_number - 1; i++) {
    g_cid_map.cid[i]._front = g_cid_map.cid[i+1]._front;
    g_cid_map.cid[i]._end = g_cid_map.cid[i+1]._end;
  }
  g_cid_map.cid_number--;
  return 0;
}

int get_numbers()
{
  return g_cid_map.cid_number;
}
