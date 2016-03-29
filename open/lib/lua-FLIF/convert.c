#include "convert.h"

#pragma pack(push,1)
typedef struct RGBA
{
	uint8_t r, g, b, a;
} RGBA;
#pragma pack(pop)

char *flif2png(char *buffer, size_t lSize, uint32_t w, uint32_t h)
{
	if (buffer == NULL) {
		fputs("Memory error", stderr);
		exit(1);
	}

	// 创建解码器并设值属性
	FLIF_DECODER *d = flif_create_decoder();
	flif_decoder_set_quality(d, 100);
	flif_decoder_set_scale(d, 1);

	flif_decoder_decode_memory(d, (const void *)buffer, lSize);

	FLIF_IMAGE *decoded = flif_decoder_get_image(d, 0);

	char    *buffer_out;
	int     i;
	buffer_out = (char *)malloc(sizeof(RGBA) * w);

	for (i = 0; i < h; ++i) {
		flif_image_read_row_RGBA8(decoded, i, (void *)buffer_out, sizeof(RGBA) * w);
		buffer_out = (char *)realloc(buffer_out, sizeof(RGBA) * w);
	}

	flif_destroy_decoder(d);
	flif_destroy_image(decoded);
	flif_free_memory(buffer);

	return buffer_out;
}

char *png2flif(char *buffer, size_t lSize, uint32_t w, uint32_t h)
{
	if (buffer == NULL) {
		fputs("Memory error", stderr);
		exit(1);
	}

	// 创建图片对象
	FLIF_IMAGE *im = flif_create_image(w, h);
	// 创建编码器，并设置属性
	FLIF_ENCODER *e = flif_create_encoder();
	flif_encoder_set_interlaced(e, 1);
	flif_encoder_set_learn_repeat(e, 3);
	flif_encoder_set_auto_color_buckets(e, 1);
	flif_encoder_set_palette_size(e, 512);
	flif_encoder_set_lookback(e, 1);

	// TODO:写入buffer信息到im中
	//     方式不对，需要修正

	int j;

	for (j = 0; j < h; ++j) {
		flif_image_write_row_RGBA8(im, j, (const void *)buffer, sizeof(RGBA) * w);
	}

	flif_encoder_add_image(e, im);

	char *buffer_out;
	buffer_out = (char *)malloc(sizeof(RGBA) * w * h);
	size_t buffer_size = sizeof(RGBA) * w * h;
	// 编码器e对buffer_out进行编码
	flif_encoder_encode_memory(e, (void **)&buffer_out, &buffer_size);

	flif_destroy_encoder(e);
	flif_destroy_image(im);
	flif_free_memory(buffer);

	return buffer_out;
}

