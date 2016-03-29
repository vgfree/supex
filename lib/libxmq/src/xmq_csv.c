#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmq_csv.h"
#include "slog/slog.h"

static int csv_parse(csv_parser_t *csv);

static size_t number_of_fields(const char *str, const char c);

/*freeif - 释放一个指针*/
#undef freeif
#define freeif(ptr)		\
	do { if (ptr) {		\
		     free(ptr);	\
	     }			\
	     ptr = NULL; } while (0)

/*xmq_csv_field_to_string - 将csv字段类型转换成字符串
 * @field:              要转换的字段
 * return:              以\0结束的字符串
 * */
extern char *xmq_csv_field_to_string(csv_field_t *field)
{
	if (!field) {
		return NULL;
	}

	char *buf = (char *)malloc(field->len + 1);

	if (buf) {
		snprintf(buf, field->len + 1, "%s", field->ptr);
	}

	return buf;
}

/*number_of_fields - 计算一个字符串中含有bei某个字符fenge的yu个数
 * @str:           需要计算的字符串
 * @split:             fen ge fu.
 * return:         字符串str 中包含字符c 的个数*/
static size_t number_of_fields(const char *str, char split)
{
	const char      *pos = str;
	size_t          count = 0, index = strlen(str) - 1;

	/* reject the type: ",one,two,three"*/
	if (*pos == split) {
		pos = str + 1;
	}

	for (; *pos != '\0'; pos++) {
		/*  *(pos-1) != split  reject type: one,two,three,,,,, */
		if ((*pos == split) && (*(pos - 1) != split)) {
			count++;
		}
	}

	/* reject the type: one,two,three,*/
	if (*(--pos) != split) {
		count++;
	}

	return count;
}

/*csv_parse - csv 解析函数
 * @csv:      解析器指针，内部封装要解析的字符串
 * return:    csv字符串字段个数
 * */
static int csv_parse(csv_parser_t *csv)
{
	csv_field_t *fields = NULL;

	if (csv && csv->string) {
		fields = (csv_field_t *)calloc(number_of_fields(csv->string, ','), sizeof(csv_field_t));

		if (!fields) {
			x_printf(W, "csv_parse: fail. Memory is not enough.");
			return 0;
		}
	} else {
		return xmq_csv_parser_init(csv);
	}

	size_t  count = 0, len, vl;
	char    *pos = csv->string;
	/* such as: ,xx,xx, ignore the last ','. */
	len = strlen(pos);
	vl = (*(pos + (len - 1)) == ',') ? len - 1 : len;

	if (*pos == ',') {
		pos++;
	}

	for (; *pos != '\0'; pos++) {
		if (*pos != ',') {
			fields[count].ptr = pos;
			pos = strchr(pos, ',');
			pos = (pos) ? pos : (csv->string + vl);
			fields[count].len = pos - fields[count].ptr;
			count++;

			/* Situation: csv->string = "A", only one character. */
			if (*pos == '\0') {
				break;
			}
		}
	}

	csv->fields = fields;
	csv->count = count;

	return count;
}

/*xmq_csv_parser_init - 初始化解析器
 * @csv:            需要初始化的解析器指针
 * return:          成功返回0，失败返回-1
 * */
extern int xmq_csv_parser_init(csv_parser_t *csv)
{
	if (csv) {
		csv->string = NULL;
		csv->fields = NULL;
		csv->count = 0;

		return 0;
	}

	return -1;
}

/*xmq_csv_parse_string - 解析csv 字符串
 * @csv:             解析器指针
 * @str:             需要解析的csv字符串
 * return:           csv字符串字段个数，失败返回-1
 * */
extern int xmq_csv_parse_string(csv_parser_t *csv, const char *str)
{
	if (csv) {
		csv->string = strdup(str);

		if (csv->string) {
			return csv_parse(csv);
		}
	}

	return -1;
}

/*xmq_csv_parser_destroy - 释放解析器
 * @csv:               需要释放的解析器指针
 * return:             无*/
extern void xmq_csv_parser_destroy(csv_parser_t *csv)
{
	if (csv) {
		freeif(csv->string);
		freeif(csv->fields);
		csv->count = 0;
	}
}

