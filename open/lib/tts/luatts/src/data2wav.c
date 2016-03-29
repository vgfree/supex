/**
 * copyright (c) 2015
 * All Right Reserved
 *
 * @file data2wav.c
 * @detail Format Conversion implement
 *
 * @date   2015-12-24
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define     WORD        short
#define     DWORD       int

struct data2wav
{
	char    *data;
	size_t  len;
	FILE    *fp;
};

// wav语音文件头部
struct RIFF_HEADER
{
	char    szRiffID[4];		// 'R','I','F','F'
	DWORD   dwRiffSize;
	char    szRiffFormat[4];	// 'W','A','V','E'
};

struct WAVE_FORMAT
{
	WORD    wFormatTag;
	WORD    wChannels;
	DWORD   dwSamplesPerSec;
	DWORD   dwAvgBytesPerSec;
	WORD    wBlockAlign;
	WORD    wBitsPerSample;
};

struct FMT_BLOCK
{
	char                    szFmtID[4];	// 'f','m','t',' '
	DWORD                   dwFmtSize;
	struct WAVE_FORMAT      wavFormat;
};

struct DATA_BLOCK
{
	char    szDataID[4];	// 'd','a','t','a'
	DWORD   dwDataSize;
};

union DWORD_CHAR
{
	int     nValue;
	char    charBuf[4];
};

/* ===== INITIALISATION ===== */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502

/* Compatibility for Lua 5.1.
 *
 * luaL_setfuncs() is used to create a module table where the functions have
 * json_config_t as their first upvalue. Code borrowed from Lua 5.2 source.
 */
static void luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup)
{
	int i;

	luaL_checkstack(l, nup, "too many upvalues");

	for (; reg->name != NULL; reg++) {		/* fill the table with given functions */
		for (i = 0; i < nup; i++) {		/* copy upvalues to the top */
			lua_pushvalue(l, -nup);
		}

		lua_pushcclosure(l, reg->func, nup);		/* closure with those upvalues , and pop all of them*/
		lua_setfield(l, -(nup + 2), reg->name);
	}

	lua_pop(l, nup);	/* remove upvalues */
}
#endif

// 写入文件头部
int writeWaveHead(FILE *fp, const int dw_size, const int dw_samples_per_sec,
	const int ui_bits_per_sample);

// 语音文件正文拼接
int writeWaveBody(FILE *fp, long filelength);

int writeFile2Int(FILE *fp, int nWhere, int nValue)
{
	if (fp == NULL) {
		return 0;
	}

	fseek(fp, nWhere, SEEK_SET);
	union DWORD_CHAR dc;
	dc.nValue = nValue;
	fwrite(dc.charBuf, 1, 4, fp);
	return 1;
}

int writeWaveHead(FILE *fp, const int dw_size, const int dw_samples_per_sec,
	const int ui_bits_per_sample)
{
	if (NULL == fp) {
		return -1;
	}

	if ((0 > dw_size) || (0 > dw_samples_per_sec) || (0 > ui_bits_per_sample)) {
		return -2;
	}

	// 写WAV文件头
	struct RIFF_HEADER rh;
	memset(&rh, 0, sizeof(rh));
	strncpy(rh.szRiffFormat, "WAVE", 4);
	strncpy(rh.szRiffID, "RIFF", 4);

	fwrite(&rh, 1, sizeof(rh), fp);

	struct FMT_BLOCK fb;
	strncpy(fb.szFmtID, "fmt ", 4);
	fb.dwFmtSize = dw_size;
	fb.wavFormat.wFormatTag = 0x0001;
	fb.wavFormat.wChannels = 1;
	fb.wavFormat.wBitsPerSample = ui_bits_per_sample;
	fb.wavFormat.dwSamplesPerSec = dw_samples_per_sec;
	fb.wavFormat.wBlockAlign = fb.wavFormat.wChannels * fb.wavFormat.wBitsPerSample / 8;	// 4;
	fb.wavFormat.dwAvgBytesPerSec = fb.wavFormat.dwSamplesPerSec * fb.wavFormat.wBlockAlign;
	fwrite(&fb, 1, sizeof(fb), fp);
	char buf[] = { "data0000" };
	fwrite(buf, 1, sizeof(buf), fp);
	return 1;
}

int writeWaveBody(FILE *fp, long filelength)
{
	if (NULL == fp) {
		return -1;
	}

	if (0 > filelength) {
		return -2;
	}

	// 更新WAV文件dwRiffSize字段中的值
	int nWhere = 4;
	writeFile2Int(fp, nWhere, filelength - 8);

	// 更新WAV文件DataChunk中Size字段的值
	nWhere = sizeof(struct RIFF_HEADER) + sizeof(struct FMT_BLOCK) + 4;
	writeFile2Int(fp, nWhere, filelength - (sizeof(struct RIFF_HEADER) + sizeof(struct FMT_BLOCK) + 8));
	return 1;
}

int data_to_wav(struct data2wav *data2wav, const int dw_size,
	const int dw_samples_per_sec, const int ui_bits_per_sample)
{
	if ((NULL == data2wav) || (NULL == data2wav->data) || (NULL == data2wav->fp) || (0 > dw_size) ||
		(0 > dw_samples_per_sec) || (0 > ui_bits_per_sample)) {
		// 错误的参数
		return -1;
	}

	FILE *fpD = data2wav->fp;

	long filelength = data2wav->len;

	if (filelength < 44) {
		return -2;
	}

	int ret = 0;
	ret = writeWaveHead(fpD, dw_size, dw_samples_per_sec, ui_bits_per_sample);

	if (ret < 0) {
		// 文件头部写入错误
		return -3;
	}

	ret = writeWaveBody(fpD, filelength);

	if (ret < 0) {
		// 语音内容拼接错误
		return -4;
	}

	int     *b = (int *)(data2wav->data + 44);
	int     len = data2wav->len - 44;

	while (len >= 4) {
		fwrite(b, sizeof(*b), 1, fpD);
		b++;
		len -= sizeof(*b);
	}

	return 1;
}

static int data2wav_create(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	struct data2wav *data2wav = (struct data2wav *)malloc(sizeof(struct data2wav));

	if (!data2wav) {
		return luaL_error(L, "%s", "no enough memory");
	}

	data2wav->data = NULL;
	data2wav->len = 0;
	data2wav->fp = fopen(lua_tostring(L, 1), "wb+");

	if (data2wav->fp == NULL) {
		free(data2wav);
		return luaL_error(L, "%s", "file open fail");
	}

	lua_pushlightuserdata(L, data2wav);

	return 1;
}

static int data2wav_put(lua_State *L)
{
	if (lua_gettop(L) != 3) {
		return luaL_error(L, "%s", "argument error");
	}

	struct data2wav *data2wav = (struct data2wav *)lua_touserdata(L, 1);
	void            *data = lua_touserdata(L, 2);
	int             len = lua_tointeger(L, 3);

	assert(data2wav);
	assert(data);

	data2wav->data = (char *)realloc(data2wav->data, len + data2wav->len);
	assert(data2wav->data);

	memcpy(data2wav->data + data2wav->len, data, len);
	data2wav->len += len;

	return 0;
}

static int data2wav_transfer(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	struct data2wav *data2wav = (struct data2wav *)lua_touserdata(L, 1);

	if (data2wav == NULL) {
		return luaL_error(L, "%s", "pcm2wav == NULL");
	}

	int ret = data_to_wav(data2wav, 16, 16000, 16);

	if (ret > 0) {
		lua_pushboolean(L, 1);
	} else {
		lua_pushboolean(L, 0);
	}

	fclose(data2wav->fp);
	free(data2wav->data);
	free(data2wav);

	return 1;
}

static const struct luaL_Reg lib[] = {
	{ "data2wav_create",   data2wav_create   },
	{ "data2wav_put",      data2wav_put      },
	{ "data2wav_transfer", data2wav_transfer },
	{ NULL,                NULL              }
};

int luaopen_data2wav(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, lib, 0);

	return 1;
}

