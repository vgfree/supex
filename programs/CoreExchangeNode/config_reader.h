#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <stdlib.h>

#define MAX_STRING_SIZE 50

struct string {
  char buf[MAX_STRING_SIZE];
  int size;
};

struct string_pair {
  struct string key;
  struct string value;
};

struct string_map {
  struct string_pair *pair;
  int cap;
  int max_valid;
};

struct config_reader
{
  int m_load_ok;
  struct string_map m_config_map;
  char m_config_file[MAX_STRING_SIZE];
};

int get_ip(const char *src, char *dest);

struct config_reader* init_config_reader(const char *filename);
void destroy_config_reader(struct config_reader *config);

char* get_config_name(struct config_reader *config, const char* name);
int set_config_value(struct config_reader *config,
                     const char *name, const char*  value);
static void _load_file(struct config_reader *config, const char* filename);
static int _write_file(struct config_reader *config, const char* filename);
static void _parse_line(struct config_reader *config, char* line);
static char* _trim_space(char* name);

#endif /* _CONFIG_READER_H_ */
