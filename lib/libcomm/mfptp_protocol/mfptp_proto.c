#include "mfptp_parse.h"


int mfptp_proto_init(struct mfptp_parser *info, char *const *data, const int *size)
{
	mfptp_parse_init(info, data, size);
	return 0;
}

int mfptp_proto_free(struct mfptp_parser *info)
{
	mfptp_parse_destroy(info);
	return 0;
}

int mfptp_proto_reqt(struct mfptp_parser *info)
{
	int size = mfptp_parse(info);
	if (size > 0 && info->ms.step == MFPTP_PARSE_OVER) {
		/* 成功解析了一个包 */
		return info->ms.dosize;
	}

	if (size == 0) {
		/* 数据未接收完毕 */
		return 0;
	}

	/* 数据解析出错 */
	return -1;
}
