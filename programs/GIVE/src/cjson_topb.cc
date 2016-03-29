#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "mirrtalk-transfer-data.pb.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message_lite.h"

#include "libkv.h"
#include "cJSON.h"
#include "utils.h"
#include "cjson_topb.h"
#include "filter.h"

#define DATANUM 15

extern kv_handler_t *city_handler;

using namespace std;

typedef mirrtalk::traffic::fp::receiver::positionprotocol::CiTyWLPosition CiTyWLPosition;
typedef mirrtalk::traffic::fp::receiver::positionprotocol::CiTyWLGpsData CiTyWLGpsData;

using google::protobuf::io::FileOutputStream;
using google::protobuf::io::GzipOutputStream;
using google::protobuf::io::ArrayOutputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::uint8;
using google::protobuf::io::GzipInputStream;
using google::protobuf::io::ArrayInputStream;
using google::protobuf::io::StringOutputStream;

cJSON *getjsonitem(cJSON *obj, const char *item);

int     getjsonarry(cJSON * obj, cJSON * arr[5], int len, const char *arryitem);
int     single_data_encode(CiTyWLPosition *citywl_position,
	cJSON *idvalue, cJSON *latvalue,
	cJSON *lngvalue, cJSON *speedvalue,
	cJSON *anglevalue, cJSON *gpstimevalue);

int pack_data_encode(CiTyWLGpsData *citywl_gpsdata,
	cJSON *idvalue, cJSON **latvalue,
	cJSON **lngvalue, cJSON **speedvalue,
	cJSON **anglevalue, cJSON **gpstimevalue,
	int len);

int gzip_compress(const char *src, int srcLen, char *dst);

int gzip_decompress(const char *src, int srcLen, char *dst);

#define _min(x, y) (y) ^ (((x) ^ (y)) & - ((x) < (y)))
typedef unsigned long long ull;

int cjson_topb(const char *data, char **result, data_count_t *dt)
{
	cJSON           *idvalue = NULL;
	cJSON           *imei = NULL;
	cJSON           *latvalue[DATANUM] = { 0 };
	cJSON           *lngvalue[DATANUM] = { 0 };
	cJSON           *speedvalue[DATANUM] = { 0 };
	cJSON           *anglevalue[DATANUM] = { 0 };
	cJSON           *gpstimevalue[DATANUM] = { 0 };
	CiTyWLGpsData   citywl_gpsdata;

	int     retnum = 0;
	int     count = 0;
	string  str;
	char    ret[GZIP_BUFF_SIZE] = {};
	int     size = 0;

	if (!data) {
		x_printf(E, "gps data is null!.\n");
		return GV_ERR;
	}

	cJSON *obj = cJSON_Parse(data);

	if (!obj) {
		x_printf(E, "**[ FAILED **] Failed to parsedata: %s, error:%s\n", data, cJSON_GetErrorPtr());
		goto jsonerr;
	}

	idvalue = getjsonitem(obj, "tokenCode");

	if (!idvalue) {
		x_printf(E, "get tokenCode item failed !\n");
		goto jsonerr;
	}

	imei = getjsonitem(obj, "IMEI");

	if (!imei) {
		x_printf(E, "get IMEI item failed !\n");
		goto jsonerr;
	}

	// int min_retnum = 2147483;
	retnum = getjsonarry(obj, latvalue, DATANUM, "latitude");

	if (retnum < 0) {
		x_printf(W, "get latitude item failed !\n");
		goto emperr;
	}

	// min_retnum = min_retnum < retnum ? min_retnum : retnum;

	retnum = getjsonarry(obj, lngvalue, DATANUM, "longitude");

	if (retnum < 0) {
		x_printf(W, "get longitude item failed !\n");
		goto emperr;
	}

	// min_retnum = min_retnum < retnum ? min_retnum : retnum;

	retnum = getjsonarry(obj, speedvalue, DATANUM, "speed");

	if (retnum < 0) {
		x_printf(W, "get speed item failed !\n");
		goto emperr;
	}

	// min_retnum = min_retnum < retnum ? min_retnum : retnum;

	retnum = getjsonarry(obj, anglevalue, DATANUM, "direction");

	if (retnum < 0) {
		x_printf(W, "get direction item failed !\n");
		goto emperr;
	}

	// min_retnum = min_retnum < retnum ? min_retnum : retnum;

	retnum = getjsonarry(obj, gpstimevalue, DATANUM, "GPSTime");

	if (retnum < 0) {
		x_printf(W, "get GPSTime item failed !\n");
		goto emperr;
	}

	// min_retnum = min_retnum < retnum ? min_retnum : retnum;

	int citycode;

	if ((citycode = citycode_filter(city_handler, lngvalue[0]->valuedouble, latvalue[0]->valuedouble)) <= 0) {
		x_printf(D, "point [%.7lf, %.7lf] is out of valid city.\n", lngvalue[0]->valuedouble, latvalue[0]->valuedouble);
		cJSON_Delete(obj);
		return GV_FILTER;
	}

	count = pack_data_encode(&citywl_gpsdata,
			idvalue, latvalue, lngvalue,
			speedvalue, anglevalue, gpstimevalue, retnum);

	if (count < 0) {
		x_printf(E, "pack_data_encode failed !\n");
		goto jsonerr;
	}

	if (!citywl_gpsdata.SerializeToString(&str)) {
		x_printf(E, "serializetostring error! !\n");
		goto jsonerr;
	}

	size = gzip_compress(str.c_str(), str.size(), ret);

	if (size <= 0) {
		x_printf(E, "gzipcompress failed !\n");
		goto jsonerr;
	}

	*result = (char *)calloc(sizeof(char), size);

	if (*result == NULL) {
		x_printf(E, "calloc failed !\n");
		goto jsonerr;
	}

	memcpy(*result, ret, size);

	dt->IMEI = (ull)atoll(imei->valuestring);
	dt->data_cnt = count;
	dt->data_size = size;
	dt->city_code = citycode;

	cJSON_Delete(obj);
	return size;

jsonerr:
	cJSON_Delete(obj);
	return GV_ERR;

emperr:
	cJSON_Delete(obj);
	return GV_EMPTY;
}

cJSON *getjsonitem(cJSON *obj, const char *item)
{
	cJSON *retitem = NULL;

	retitem = cJSON_GetObjectItem(obj, item);

	if (!retitem) {
		x_printf(E, "Failed to decode %s .\n", item);

		return NULL;
	}

	return retitem;
}

int getjsonarry(cJSON *obj, cJSON *arr[DATANUM], int len, const char *arryitem)
{
	cJSON *temp = NULL;

	temp = cJSON_GetObjectItem(obj, arryitem);

	if (!temp) {
		x_printf(E, "Failed to decode %s .\n", arryitem);
		return GV_PARSEJSON;
	}

	int arrlen = cJSON_GetArraySize(temp);

	if ((arrlen <= 0) || (arrlen > DATANUM)) {
		x_printf(W, "array length out of range, length:%d\n", arrlen);
		return GV_PARSEJSON;
	}

	int i = 0;

	for (; i < arrlen; i++) {
		arr[i] = cJSON_GetArrayItem(temp, i);
	}

	return arrlen;
}

int single_data_encode(CiTyWLPosition *citywl_position,
	cJSON *idvalue, cJSON *latvalue,
	cJSON *lngvalue, cJSON *speedvalue,
	cJSON *anglevalue, cJSON *gpstimevalue)
{
	citywl_position->set_imei(idvalue->valuestring);
	citywl_position->set_lat((unsigned int)(latvalue->valuedouble * 1000000));
	citywl_position->set_lon((unsigned int)(lngvalue->valuedouble * 1000000));
	citywl_position->set_speed(speedvalue->valueint * 100);
	citywl_position->set_direction(anglevalue->valueint);
	citywl_position->set_collecttime(gpstimevalue->valueint);
	return GV_OK;
}

#include <math.h>
#define DETA 1e-7

int pack_data_encode(CiTyWLGpsData *citywl_gpsdata,
	cJSON *idvalue, cJSON **latvalue,
	cJSON **lngvalue, cJSON **speedvalue,
	cJSON **anglevalue, cJSON **gpstimevalue,
	int len)
{
	int     i, j;
	int     cnt = len;

	for (i = 1, j = 0; j < len; j++, i++) {
		if ((i < len) && (fabs(latvalue[i]->valuedouble - latvalue[i - 1]->valuedouble) <= DETA) &&	// if latitude is the same
			(fabs(lngvalue[i]->valuedouble - lngvalue[i - 1]->valuedouble) <= DETA) &&		// if longitude is the same
			(anglevalue[i]->valueint != -1) && (anglevalue[i - 1]->valueint != -1) &&		// if direction == -1
			(gpstimevalue[i]->valueint == gpstimevalue[i - 1]->valueint)) {				// if time is the same
			cnt--;
			continue;
		}

		CiTyWLPosition *position = citywl_gpsdata->add_position();
		single_data_encode(position,
			idvalue, latvalue[j],
			lngvalue[j], speedvalue[j],
			anglevalue[j], gpstimevalue[j]);
	}

	int count = cnt;
	citywl_gpsdata->set_gpscount(count);
	return count;
}

int gzip_compress(const char *src, int srcLen, char *dst)
{
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	int dstLen = GZIP_BUFF_SIZE;
	strm.avail_in = srcLen;
	strm.avail_out = dstLen;
	strm.next_in = (Bytef *)src;
	strm.next_out = (Bytef *)dst;

	int err = -1;
	err = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);

	if (err == Z_OK) {
		err = deflate(&strm, Z_FINISH);

		if (err == Z_STREAM_END) {
			(void)deflateEnd(&strm);
			x_printf(I, "compression succed, before compression size is %ld, after compressione size is:%d\n", strm.total_in, dstLen - strm.avail_out);
			return dstLen - strm.avail_out;
		} else {
			(void)deflateEnd(&strm);
			x_printf(E, "compression failed, deflate return: %d", err);
			return GV_ERR;
		}
	} else {
		(void)deflateEnd(&strm);
		x_printf(E, "compression initialization failed, quit!\n");
		return GV_ERR;
	}
}

string pb_data_uncompress(const string &data)
{
	string                  result;
	ArrayInputStream        input(data.data(), data.size());
	GzipInputStream         gzin(&input);
	const void              *buffer;
	int                     size;

	while (gzin.Next(&buffer, &size)) {
		result.append(reinterpret_cast <const char *> (buffer), size);
	}

	return result;
}

int gzip_decompress(const char *src, int srcLen, char *dst)
{
	int             dstLen = GZIP_BUFF_SIZE;
	z_stream        strm;

	strm.zalloc = NULL;
	strm.zfree = NULL;
	strm.opaque = NULL;

	strm.avail_in = srcLen;
	strm.avail_out = dstLen;
	strm.next_in = (Bytef *)src;
	strm.next_out = (Bytef *)dst;

	int err = -1;
	err = inflateInit2(&strm, MAX_WBITS + 16);

	if (err == Z_OK) {
		err = inflate(&strm, Z_FINISH);

		if (err == Z_STREAM_END) {
			(void)inflateEnd(&strm);
			x_printf(I, "decompression succed, before decompression size is %ld, after decompressione size is:%ld\n", strm.total_in, strm.total_out);
			return strm.total_out;
		} else {
			(void)inflateEnd(&strm);
			x_printf(E, "decompression failed, inflate return: %d", err);
			return GV_ERR;
		}
	} else {
		inflateEnd(&strm);
		x_printf(E, "decompression initialization failed, quit!\n");
		return GV_ERR;
	}
}

int gzpb_to_str(char *value, int data_len)
{
	if (!value) {
		x_printf(E, "unzip protobuf value is null\n");
		return GV_ERR;
	}

	char            dst[GZIP_BUFF_SIZE];
	int             zdatalen = gzip_decompress(value, data_len, dst);
	CiTyWLGpsData   new_data;
	new_data.ParseFromString(string(dst, zdatalen));

	x_printf(I, "==== unCompressed protocol buffer value ====\n");
	x_printf(I, "gpsCount: %d\n", new_data.gpscount());
	int i;

	for (i = 0; i < new_data.gpscount(); i++) {
		x_printf(I, "id:%s, lat:%d, lng:%d, speed:%d, angle:%d, gpstime:%d\n", new_data.position(i).imei().c_str(),
			new_data.position(i).lat(), new_data.position(i).lon(), new_data.position(i).speed(),
			new_data.position(i).direction(), new_data.position(i).collecttime());

		/*cout<< "id:" << new_data.position(i).imei() << ", ";
		 *   cout<< "lat:" << new_data.position(i).lat() << ", ";
		 *   cout<< "lng:" << new_data.position(i).lon() << ", ";
		 *   cout<< "speed:" << new_data.position(i).speed() << ", ";
		 *   cout<< "angle:" << new_data.position(i).angle() << ", ";
		 *   cout<< "gpstime:" << new_data.position(i).collecttime() <<endl;*/
	}

	return GV_OK;
}

/*
 *   int main()
 *   {
 *        char data[] = "{\"longitude\":[121.3613973,121.361327,121.3612662,121.3612118,121.3611633],\"latitude\":[31.2253355,31.2253577,31.2253755,31.2253873,31.225387],\"speed\":[26,22,19,16,14],\"tokenCode\":\"yuzwlqU8lX\",\"GPSTime\":[1407210985,1407210985,1407210985,1407210985,1407210985],\"direction\":[111,111,108,93,82]}";
 *        char *result = NULL;
 *        int count = cjson_topb(data, &result);
 *        gzpb_to_str(result, count);
 *        printf ("count is:%d\n", count);
 *        return 0;
 *   }
 */

