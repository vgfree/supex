#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmq_csv.h"
#include "slog/slog.h"

static int csv_parse(csv_parser_t *csv);

static unsigned int number_of_char(const char *str, const char c);

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
	size_t  size = field_len(field) + 1;
	char    *buf = (char *)malloc(size);

	if (buf) {
		snprintf(buf, size, "%s", field->ptr);
	}

	return buf;
}

/*number_of_char - 计算一个字符串中含有某个字符的个数
 * @str:           需要计算的字符串
 * @c:             需要计算个数的字符
 * return:         字符串str 中包含字符c 的个数*/
static unsigned int number_of_char(const char *str, const char c)
{
	const char      *pos;
	unsigned int    count = 0;

	for (pos = str; *pos; pos++) {
		if (*pos == c) {
			count++;
		}
	}

	return count;
}

/*csv_parse - csv 解析函数
 * @csv:      解析器指针，内部封装要解析的字符串
 * return:    csv字符串字段个数
 * */
static int csv_parse(csv_parser_t *csv)
{
	unsigned int    commas;
	csv_field_t     *fields;

	if (csv && csv->string) {
		commas = number_of_char(csv->string, ',');
		fields = (csv_field_t *)calloc(commas + 1, sizeof(csv_field_t));

		if (!fields) {
			x_printf(W, "momery is not enough");
			return 0;
		}
	} else {
		return xmq_csv_parser_init(csv);
	}

	char            *pos = csv->string;
	unsigned int    count = 0;

	do {
		if (',' == *pos) {
			fields[count].ptr = NULL;
			fields[count].len = 0;

			count++;
			pos++;

			x_printf(W, "there is an empty field in csv-string[%s]", csv->string);
		} else {
			fields[count].ptr = pos;
			pos = strchr(pos, ',');

			if (pos) {
				fields[count].len = pos - fields[count].ptr;

				if (fields[count].len >= FIELD_MAX_LEN) {
					x_printf(W, "field [%s] length [%u] is too large",
						fields[count].ptr, fields[count].len);
					freeif(csv->fields);
					csv->count = 0;
					return -1;
				}

				pos++;
				count++;
			} else {
				x_printf(W, "there is not a comma at the last, csv-string[%s]", csv->string);

				freeif(csv->fields);
				csv->count = 0;
				return -1;
			}
		}
	} while (*pos);

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

