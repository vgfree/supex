/**
 * copyright (c) 2015
 * All Right Reserved
 *
 * @file sample_multi_thread.cpp
 * @detail multi thread deal with multi file for merge multi voice
 *
 * @auther qianxiaoyuan
 * @date   2015-12-24
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

typedef struct thread
{
	pthread_t       threadA, threadB, threadC;
	const char      *thread1_name, *thread2_name, *thread3_name;
	pthread_mutex_t mutex;
} *thread_t;

thread_t p;

void *tts(void *arg);

static int text_to_speech(TTSHANDLE handle, const char *src_text, const char *pcm_filename, const char *wav_filename);

static int pcm_to_wav(const char *pcm_name, const char *wav_name, const int dw_size,
	const int dw_samples_per_sec, const int ui_bits_per_sample);

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "please choose asr/nlu/tts ! Ths first arg must be asr/nlu/tts ! \n");
		return -1;
	}

	char            *service = argv[1];
	const char      *tts_file1 = "../testfile/utf8.txt";
	const char      *tts_file2 = "../testfile/utf82.txt";
	const char      *tts_file3 = "../testfile/utf83.txt";

	p = (thread *)malloc(sizeof(struct thread));
	pthread_mutex_init(&p->mutex, NULL);
	p->thread1_name = "threadA->tts_file1->handleA";
	p->thread2_name = "threadB->tts_file2->handleB";
	p->thread3_name = "threadC->tts_file3->handleC";

	if (strcmp(service, "tts") == 0) {
		// E-TTS
		if (argc < 2) {
			fprintf(stderr, "Usage tts: sample.exe tts\n");
			return -1;
		}

		printf("\n语音合成：\n");

		pthread_create(&p->threadA, NULL, tts, (void *)tts_file1);
		pthread_create(&p->threadB, NULL, tts, (void *)tts_file2);
		pthread_create(&p->threadC, NULL, tts, (void *)tts_file3);
		// E-TTS
	}

	pthread_join(p->threadA, NULL);
	pthread_join(p->threadB, NULL);
	pthread_join(p->threadC, NULL);

	pthread_mutex_destroy(&p->mutex);
	free(p);
}

void *tts(void *arg)
{
	int ret = 0;

	pthread_mutex_lock(&p->mutex);
	printf("\n==>[%s]\n", pthread_equal(pthread_self(), p->threadA) ? p->thread1_name :
		pthread_equal(pthread_self(), p->threadB) ? p->thread2_name :
		pthread_equal(pthread_self(), p->threadC) ? p->thread3_name : "Thread can't be tracked!");

	const char                      *tts_file = (const char *)arg;
	ifstream                        in(tts_file, ios::in);
	istreambuf_iterator <char>      beg(in), end;
	string                          strdata(beg, end);

	// 创建合成实例
	TTSHANDLE handle = 0;
	// 设置appKey
	ret = usc_tts_create_service(&handle);

	if (ret != USC_SUCCESS) {
		printf("Login failed , Error code %d.\n", ret);
		return NULL;
	}

	ret = usc_tts_set_option(handle, "appkey", USC_ASR_SDK_APP_KEY);
	ret = usc_tts_set_option(handle, "secret", USC_ASR_SDK_SECRET_KEY);

	// 连接服务器启动合成
	ret = usc_tts_start_synthesis(handle);

	if (ret != USC_SUCCESS) {
		printf("usc_tts_start_synthesis: UscTts begin session failed Error code %d.\n", ret);
		usc_tts_release_service(handle);
		return NULL;
	}

	char    pcm_filename[100];
	char    wav_filename[100];
	snprintf(pcm_filename, sizeof(pcm_filename), "../testfile/pcm_wav/tts_audio_%lu.pcm", pthread_self());
	snprintf(wav_filename, sizeof(wav_filename), "../testfile/pcm_wav/tts_audio_%lu.wav", pthread_self());

	ret = text_to_speech(handle, strdata.c_str(), pcm_filename, wav_filename);

	if (ret != USC_SUCCESS) {
		printf("text_to_speech: failed , Error code %d.\n", ret);
		// break;
	}

	usc_tts_release_service(handle);
	pthread_mutex_unlock(&p->mutex);
	in.close();

	// 释放合成接口实例
	return NULL;
}

static int text_to_speech(TTSHANDLE handle, const char *src_text, const char *pcm_filename, const char *wav_filename)
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

union DWORD_CHAR
{
	int     nValue;
	char    charBuf[4];
};

static int writeWaveHead(FILE *fp, const int dw_size, const int dw_samples_per_sec,
	const int ui_bits_per_sample);

// 语音文件正文拼接
static int writeWaveBody(FILE *fp, long filelength);

static bool writeFile2Int(FILE *fp, int nWhere, int nValue)
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

static int writeWaveHead(FILE *fp, const int dw_size, const int dw_samples_per_sec,
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

static int writeWaveBody(FILE *fp, long filelength)
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

static int pcm_to_wav(const char *pcm_name, const char *wav_name, const int dw_size = 16,
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

