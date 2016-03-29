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
	int     result;

	// 打开图片，将其以二进制方式读取到buffer中
	fp = fopen("./miku.png", "rb");

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

	// 创建FLIF_IMAGE对象
	uint32_t        w = 1920;
	uint32_t        h = 1080;
	FLIF_IMAGE      *im = flif_create_image(w, h);

	// 创建编码器并设置属性
	FLIF_ENCODER *e = flif_create_encoder();
	flif_encoder_set_interlaced(e, 1);
	flif_encoder_set_learn_repeat(e, 3);
	flif_encoder_set_auto_color_buckets(e, 1);
	flif_encoder_set_palette_size(e, 512);
	flif_encoder_set_lookback(e, 1);

	int i;

	for (i = 0; i < h; ++i) {
		flif_image_write_row_RGBA8(im, i, (const void *)buffer, sizeof(RGBA) * w);
		printf("i = %d\n", i);
	}

	//       im->image.load("./miku.png");
	flif_encoder_add_image(e, im);
	flif_encoder_encode_file(e, "./miku_p_f.flif");

	/* 运行流程:
	 * 1. 先将源图片数据信息加载到im中
	 * 2. 将im加载到编码器中
	 * 3. 利用编码器对输出文件进行编码
	 * TODO
	 * 1. 传入数据为buffer，怎样把buffer数据加载到im中
	 * 2. 在对输出buffer进行编码时，需要确定输出buffer的文件大小，如何确定？
	 */

	return 0;
}

