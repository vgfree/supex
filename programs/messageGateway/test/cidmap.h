#ifndef _CID_MAP_H_
#define _CID_MAP_H_

#define MAX_SIZE 1000
#define CID_MAX_LENTH 20


struct cidmap {
  int cid_number;
  int current_offset;
  struct _cid {
    int _front;
    int _end;
  } cid[MAX_SIZE];
  char *buf;
};

void init_cidmap();
int get_numbers();
char *get_first_cid();
char *get_next_cid();
int append_cid(char *cid, int size);
int remove_cid(char *cid, int size);
void destroy_cidmap();
#endif
