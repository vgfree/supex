#include "config_reader.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAP_CAPCITY 50

static void _load_file(struct config_reader     *config,
	const char                              *filename);

static int _write_file(struct config_reader     *config,
	const char                              *filename);

static void _parse_line(struct config_reader *config, char *line);

static char *_trim_space(char *name);

// 解析十进制IP 为四字节, 比如: 127.0.0.1 = {0x7f, 0x00, 0x00, 0x01}
int get_ip(const char *src, char *dest)
{
	assert(src && dest);
	uint8_t value = 0;
	int     pos = 0;

	for (int i = 0; i < 3; i++) {
		while (src[pos] != '.') {
			value = (src[pos] - '0') + value * 10;
			pos++;
		}

		dest[i] = value;
		value = 0;
		pos++;
	}

	while (src[pos] != '\0') {
		value = (src[pos] - '0') + value * 10;
		pos++;
	}

	dest[3] = value;
	return 0;
}

struct config_reader *init_config_reader(const char *filename)
{
	//  log("init_config_reader.");
	struct config_reader *config = (struct config_reader *)malloc(sizeof(struct config_reader));

	config->m_config_map.pair = (struct string_pair *)
		calloc(MAP_CAPCITY, sizeof(struct string_pair));
	config->m_load_ok = 0;
	config->m_config_map.cap = MAP_CAPCITY;
	config->m_config_map.max_valid = 0;
	_load_file(config, filename);
	return config;
}

void destroy_config_reader(struct config_reader *config)
{
	free(config->m_config_map.pair);
	free(config);
}

char *get_config_name(struct config_reader *config, const char *name)
{
	//  log("config address:%p", config);
	assert(config);

	//  log("get_config_name, m_load_ok: %d.", config->m_load_ok);
	if (config->m_load_ok == 0) {
		return NULL;
	}

	char    *value = NULL;
	int     valid = config->m_config_map.max_valid;

	//  log("get_config_name, valid: %d.", valid);
	for (int i = 0; i < valid; i++) {
		if (strcmp(config->m_config_map.pair[i].key.buf, name) == 0) {
			value = config->m_config_map.pair[i].value.buf;
			break;
		}
	}

	return value;
}

int set_config_value(struct config_reader *config,
	const char *name, const char *value)
{
	if (config->m_load_ok == 1) {
		return -1;
	}

	int     valid = config->m_config_map.max_valid;
	int     i = 0;

	for (; i < valid; i++) {
		if (strcmp(config->m_config_map.pair[i].key.buf, name) == 0) {
			strcpy(config->m_config_map.pair[i].value.buf, value);
			break;
		}
	}

	strcpy(config->m_config_map.pair[i].key.buf, name);
	strcpy(config->m_config_map.pair[i].value.buf, value);
	config->m_config_map.max_valid++;
	return _write_file(config, NULL);
}

static void _load_file(struct config_reader     *config,
	const char                              *filename)
{
	memset(config->m_config_file, 0, MAX_STRING_SIZE);
	strcpy(config->m_config_file, filename);
	FILE *fp = fopen(filename, "r");

	if (!fp) {
		//    log("can not open %s", filename);
		return;
	}

	char buf[256];

	for (;; ) {
		char *p = fgets(buf, 256, fp);

		if (!p) {
			break;
		}

		size_t len = strlen(buf);

		if (buf[len - 1] == '\n') {
			buf[len - 1] = 0;	// remove \n at the end
		}

		char *ch = strchr(buf, '#');	// remove string start with #

		if (ch) {
			*ch = 0;
		}

		if (strlen(buf) == 0) {
			continue;
		}

		_parse_line(config, buf);
	}

	fclose(fp);
	config->m_load_ok = 1;
	//  log("m_load_ok:%d", config->m_load_ok);
}

static int _write_file(struct config_reader *config, const char *filename)
{
	FILE *fp = NULL;

	if (filename == NULL) {
		fp = fopen(config->m_config_file, "w");
	} else {
		fp = fopen(filename, "w");
	}

	if (fp == NULL) {
		return -1;
	}

	char    szPaire[128];
	int     valid = config->m_config_map.max_valid;

	for (int i = 0; i < valid; i++) {
		memset(szPaire, 0, sizeof(szPaire));
		snprintf(szPaire, sizeof(szPaire), "%s=%s\n",
			config->m_config_map.pair[i].key.buf,
			config->m_config_map.pair[i].value.buf);
		int ret = fwrite(szPaire, strlen(szPaire), 1, fp);

		if (ret != 1) {
			fclose(fp);
			return -1;
		}
	}

	fclose(fp);
	return 0;
}

static void _parse_line(struct config_reader *config, char *line)
{
	char *p = strchr(line, '=');

	if (p == NULL) {
		return;
	}

	*p = 0;
	char    *key = _trim_space(line);
	char    *value = _trim_space(p + 1);
	int     valid = config->m_config_map.max_valid;

	if (key && value &&
		(valid < config->m_config_map.cap)) {
		strcpy(config->m_config_map.pair[valid].key.buf, key);
		strcpy(config->m_config_map.pair[valid].value.buf, value);
		config->m_config_map.max_valid++;
	}
}

static char *_trim_space(char *name)
{
	// remove starting space or tab
	char *start_pos = name;

	while ((*start_pos == ' ') || (*start_pos == '\t')) {
		start_pos++;
	}

	if (strlen(start_pos) == 0) {
		return NULL;
	}

	// remove ending space or tab
	char *end_pos = name + strlen(name) - 1;

	while ((*end_pos == ' ') || (*end_pos == '\t')) {
		*end_pos = 0;
		end_pos--;
	}

	int len = (int)(end_pos - start_pos) + 1;

	if (len <= 0) {
		return NULL;
	}

	return start_pos;
}

