#pragma once

/*CSV字符串域类型*/
typedef struct csv_field_struct
{
	char            *ptr;
	unsigned int    len;
} csv_field_t;

#define FIELD_MAX_LEN 256
#define field_len(f) ((f)->len)

/*CSV解析器类型*/
typedef struct csv_parser_struct
{
	char            *string;
	csv_field_t     *fields;// struct [] -> malloc()
	unsigned int    count;
} csv_parser_t;

#define csv_string(c)           ((c)->string)
#define csv_field_num(c)        ((c)->count)

#define for_each_field(f, c) \
	for (f = (c)->fields; f < (c)->fields + (c)->count; f ++)

/*xmq_csv_field_to_string - 将csv字段类型转换成字符串
 * @field:              要转换的字段
 * return:              以\0结束的字符串
 * */
extern char *xmq_csv_field_to_string(csv_field_t *field);

/*xmq_csv_parser_init - 初始化解析器
 * @csv:            需要初始化的解析器指针
 * return:          成功返回0，失败返回-1
 * */
extern int xmq_csv_parser_init(csv_parser_t *csv);

/*xmq_csv_parse_string - 解析csv 字符串
 * @csv:             解析器指针
 * @str:             需要解析的csv字符串
 * return:           csv字符串字段个数，失败返回-1
 * */
extern int xmq_csv_parse_string(csv_parser_t *csv, const char *str);

/*xmq_csv_parser_destroy - 释放解析器
 * @csv:               需要释放的解析器指针
 * return:             无*/
extern void xmq_csv_parser_destroy(csv_parser_t *csv);

