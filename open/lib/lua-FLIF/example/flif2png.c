/* 测试flif2png功能*/

#include "../convert.h"

#pragma pack(push,1)
typedef struct RGBA
{
	uint8_t r, g, b, a;
} RGBA;
#pragma pack(pop)

int main()
{
	FILE    *fp;
	size_t  lSize;
	char    *buffer;
	size_t  result;

	// 打开图片，将其以二进制方式读取到buffer中
	fp = fopen("../image/miku.flif", "rb");

	if (fp == NULL) {
		fputs("File error", stderr);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);

	buffer = (char *)malloc(sizeof(char) * lSize);

	if (buffer == NULL) {
		fputs("Memory error", stderr);
		exit(2);
	}

	result = fread(buffer, 1, lSize, fp);

	if (result != lSize) {
		fputs("Reading error", stderr);
		exit(3);
	}

	uint32_t        w = 1920;
	uint32_t        h = 1080;

	flif2png(buffer, lSize, w, h);

	return 0;
}

