// sample.cpp : Defines the entry point for the console application.

/**
 * @file sample_line.cpp
 * @detail each deal line of file for merge one voice
 */

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <string.h>
#include "../libs/libusc.h"
// E-TTS
#include "../libs/tts_sdk.h"
// E-TTS
#include "../libs/appKey.h"
using namespace std;
// E-ASR
int asr(int argc, char *argv[]);

// E-ASR
// E-NLU
int nlu(int argc, char *argv[]);

// E-NLU
// E-TTS
int tts(int argc, char *argv[]);

// E-TTS
int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "please choose asr/nlu/tts ! Ths first arg must be asr/nlu/tts ! \n");
		return -1;
	}

	char *service = argv[1];

	if (strcmp(service, "tts") == 0) {
		// E-TTS
		if (argc < 2) {
			fprintf(stderr, "Usage tts: sample.exe tts\n");
			return -1;
		}

		printf("\n语音合成：\n");
		tts(argc, argv);
		// E-TTS
	} else {
		fprintf(stderr, "please choose asr/nlu/tts ! Ths first arg must be asr/nlu/tts ! \n");
		return -1;
	}

	return 0;
}

// E-TTS
// pcm格式音频转换为wav格式

/*
 *   参数
 *   pcm_name:原语音文件
 *   wav_name:需转换的语音文件
 *   dw_size:采样精度,默认16
 *   dw_samples_per_sec:采样率,默认16000
 *   ui_bits_per_sample:数据格式位数,默认16
 */
int pcm_to_wav(const char *pcm_name, const char *wav_name, const int dw_size,
	const int dw_samples_per_sec, const int ui_bits_per_sample);

// 文字转换语音
int text_to_speech(TTSHANDLE handle, const char *src_text, const char *pcm_filename, const char *wav_filename)
{
	int             ret = -1;
	unsigned int    text_len = 0;
	unsigned int    audio_len = 0;
	int             synth_status = RECEIVING_AUDIO_DATA;
	ofstream        output_fd;

	if ((NULL == src_text) || (NULL == pcm_filename) || (NULL == wav_filename)) {
		printf("tts params is null!\n");
		return ret;
	}

	printf("begin to synth line : %s\n", src_text);

	text_len = (unsigned int)strlen(src_text);

	output_fd.open(pcm_filename, ios::binary);

	if (!output_fd.is_open()) {
		printf("open file %s error\n", pcm_filename);
		return ret;
	}

	// 设置合成语音格式  pcm/mp3
	usc_tts_set_option(handle, "audioFormat", "audio/x-wav");
	usc_tts_set_option(handle, "audioCodec", "opus");

	// 传入文本
	ret = usc_tts_text_put(handle, src_text, text_len);

	if (ret != USC_SUCCESS) {
		printf("usc_tts_text_put: UscTts put text failed Error code %d.\n", ret);
		usc_tts_stop_synthesis(handle);
		return ret;
	}

	while (1) {
		audio_len = 0;
		// 获取合成结果
		const void *data = usc_tts_get_result(handle, &audio_len, &synth_status, &ret);

		if (NULL != data) {
			output_fd.write((const char *)data, audio_len);
		}

		if ((synth_status == AUDIO_DATA_RECV_DONE) || (ret != 0)) {
			if (ret != 0) {
				printf("usc_tts_get_result error code =%d\n", ret);
				output_fd.close();
				return ret;
			}

			break;
		}
	}	// 合成状态synth_status取值可参考开发文档

	output_fd.close();

	// 停止语音输入
	ret = usc_tts_stop_synthesis(handle);

	if (ret != USC_SUCCESS) {
		printf("usc_tts_stop_synthesis: UscTts end failed Error code %d.\n", ret);
	} else {
		printf("success to pcm file : %s\n", pcm_filename);
	}

	// 以下是为将pcm文件生成为wav格式,如不需要可以注释掉
	ret = pcm_to_wav(pcm_filename, wav_filename, 16, 16000, 16);

	if (ret > 0) {
		printf("pcm_to_wav: success!\n");
	} else {
		printf("pcm_to_wav: error!\terror code : %d\n", ret);
	}

	return 0;
}

// 语音合成接口
int tts(int argc, char *argv[])
{
	int             ret = 0;
	const char      *tts_file = "../testfile/utf8.txt";
	ifstream        input_fd(tts_file);
	// 创建合成实例
	TTSHANDLE handle = 0;

	// 设置appKey
	ret = usc_tts_create_service(&handle);

	if (ret != USC_SUCCESS) {
		printf("Login failed , Error code %d.\n", ret);
		return -1;
	}

	ret = usc_tts_set_option(handle, "appkey", USC_ASR_SDK_APP_KEY);
	ret = usc_tts_set_option(handle, "secret", USC_ASR_SDK_SECRET_KEY);

	// 连接服务器启动合成
	ret = usc_tts_start_synthesis(handle);

	if (ret != USC_SUCCESS) {
		printf("usc_tts_start_synthesis: UscTts begin session failed Error code %d.\n", ret);
		usc_tts_release_service(handle);
		return -1;
	}

	string  line;
	int     line_num = 0;
	char    pcm_filename[100];
	char    wav_filename[100];

	while (getline(input_fd, line)) {
		sprintf(pcm_filename, "../testfile/pcm_wav/tts_audio_%03d.pcm", line_num);
		sprintf(wav_filename, "../testfile/pcm_wav/tts_audio_%03d.wav", line_num);
		ret = text_to_speech(handle, line.c_str(), pcm_filename, wav_filename);

		if (ret != USC_SUCCESS) {
			printf("text_to_speech: failed , Error code %d.\n", ret);
			// break;
		}

		line_num++;
	}

	input_fd.close();

	// 释放合成接口实例
	usc_tts_release_service(handle);

	return 0;
}

#define     UNIT        4
#define     WORD        short
#define     DWORD       int
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
	char            szFmtID[4];	// 'f','m','t',' '
	DWORD           dwFmtSize;
	WAVE_FORMAT     wavFormat;
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
// 写入文件头部
int writeWaveHead(FILE *fp, const int dw_size, const int dw_samples_per_sec,
	const int ui_bits_per_sample);

// 语音文件正文拼接
int writeWaveBody(FILE *fp, long filelength);

bool writeFile2Int(FILE *fp, int nWhere, int nValue)
{
	if (fp == NULL) {
		return false;
	}

	fseek(fp, nWhere, SEEK_SET);
	DWORD_CHAR dc;
	dc.nValue = nValue;
	fwrite(dc.charBuf, 1, 4, fp);
	return true;
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
	RIFF_HEADER rh;
	memset(&rh, 0, sizeof(rh));
	strncpy(rh.szRiffFormat, "WAVE", 4);
	strncpy(rh.szRiffID, "RIFF", 4);

	fwrite(&rh, 1, sizeof(rh), fp);

	FMT_BLOCK fb;
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
	nWhere = sizeof(RIFF_HEADER) + sizeof(FMT_BLOCK) + 4;
	writeFile2Int(fp, nWhere, filelength - (sizeof(RIFF_HEADER) + sizeof(FMT_BLOCK) + 8));
	return 1;
}

int pcm_to_wav(const char *pcm_name, const char *wav_name, const int dw_size = 16,
	const int dw_samples_per_sec = 16000, const int ui_bits_per_sample = 16)
{
	if ((NULL == pcm_name) || (NULL == wav_name) || (0 > dw_size) ||
		(0 > dw_samples_per_sec) || (0 > ui_bits_per_sample)) {
		// 错误的参数
		return -1;
	}

	FILE    *fpS;
	FILE    *fpD;
	fpS = fopen(pcm_name, "rb");
	fpD = fopen(wav_name, "wb+");

	if ((fpS == NULL) || (fpD == NULL)) {
		// 文件打开错误
		return -2;
	}

	fseek(fpS, 0, SEEK_END);
	long filelength = ftell(fpS);

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

	fseek(fpS, 44, SEEK_SET);

	char buf[UNIT];

	while (UNIT == fread(buf, 1, UNIT, fpS)) {
		fwrite(buf, 1, UNIT, fpD);
	}

	fclose(fpS);
	fclose(fpD);

	return 1;
}

