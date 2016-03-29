/**
 *
 **/
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

#include "utils.h"
#include "parser_mttp.h"

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
			x_printf(D, "decompression succed, before decompression size is %ld, after decompressione size is:%ld\n", strm.total_in, strm.total_out);
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

int parsepb_to_struct(char *value, int zdatalen, data_count_t *dt, gps_data_t *gps_dt)
// int parsepb_to_struct(char *value, int zdatalen, void **gps_dt)
{
	if (!value) {
		x_printf(E, "unzip protobuf value is null\n");
		return GV_ERR;
	}

	// gps_data_t **gps_dt = (gps_data_t **)gpsdt;
	// gps_data_t *gps_dt = NULL;

	CiTyWLGpsData new_data;

	if (!new_data.ParseFromString(string(value, zdatalen))) {
		x_printf(E, ".ParseFromString error\n");
		return GV_ERR;
	}

	x_printf(D, "==== unCompressed protocol buffer value ====\n");
	x_printf(D, "gpsCount: %d\n", new_data.gpscount());
	int i;

	for (i = 0; i < new_data.gpscount(); i++) {
		x_printf(D, "id:%s, lat:%d, lng:%d, speed:%d, angle:%d, gpstime:%d\n", new_data.position(i).imei().c_str(),
			new_data.position(i).lat(), new_data.position(i).lon(), new_data.position(i).speed(),
			new_data.position(i).direction(), new_data.position(i).collecttime());

		/*	cout << "id:" << new_data.position(i).imei() << ", ";
		 *        cout << "lat:" << new_data.position(i).lat() << ", ";
		 *        cout << "lng:" << new_data.position(i).lon() << ", ";
		 *        cout << "speed:" << new_data.position(i).speed() << ", ";
		 *        cout << "angle:" << new_data.position(i).direction() << ", ";
		 *        cout << "gpstime:" << new_data.position(i).collecttime() << endl;*/
	}

	// gps_dt = (gps_data_t *)calloc(1, sizeof(gps_data_t));

	if (gps_dt == NULL) {
		x_printf(E, "gpsdata calloc error\n");
		return GV_ERR;
	}

	memset(gps_dt, 0, sizeof(gps_data_t));
	gps_dt->size = new_data.gpscount();

	for (i = 0; i < new_data.gpscount(); i++) {
		const char *imei = new_data.position(i).imei().c_str();

		if (imei) {
			strcpy(gps_dt->data[i].imei, imei);
		}

		gps_dt->data[i].lat = new_data.position(i).lat();
		gps_dt->data[i].lon = new_data.position(i).lon();
		gps_dt->data[i].speed = new_data.position(i).speed();
		gps_dt->data[i].direction = new_data.position(i).direction();
		gps_dt->data[i].gpstime = new_data.position(i).collecttime();
	}

	int citycode;

	if ((citycode = citycode_filter(city_handler, (double)gps_dt->data[0].lon / 1000000, (double)gps_dt->data[0].lat / 1000000)) <= 0) {
		x_printf(E, "point [%.7lf, %.7lf] is out of valid city.\n", (double)gps_dt->data[0].lon / 1000000, (double)gps_dt->data[0].lat / 1000000);
		citycode = 0;
		// return GV_FILTER;
	}

	strcpy(dt->IMEI, gps_dt->data[0].imei);
	dt->data_cnt = gps_dt->size;
	dt->city_code = citycode;

	// free(gps_dt);
	return GV_OK;
}

int parse_body(const char *value, size_t data_len, data_count_t *dt, gps_data_t *gps_dt)
{
	char    dst[GZIP_BUFF_SIZE];
	int     zdatalen = gzip_decompress(value, data_len, dst);

	dt->data_size = data_len;

	if (zdatalen <= 0) {
		x_printf(E, "[** FAILED **] Failed to ungzip.\n");
		return GV_ERR;
	}

	if (parsepb_to_struct(dst, zdatalen, dt, gps_dt) != GV_OK) {
		x_printf(E, "[** FAILED **] Failed to unserialize protocol buffer.\n");
		return GV_ERR;
	}

	return GV_OK;
}

