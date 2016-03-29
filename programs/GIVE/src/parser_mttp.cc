/**
 *
 **/

#include "parser_mttp.h"

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
			x_printf(I, "decompression succed, before decompression size is %ld, after decompressione size is:%ld\n", strm.total_in, strm.total_out);
			return strm.total_out;
		} else {
			(void)inflateEnd(&strm);
			x_printf(E, "decompression failed, inflate return: %d", err);
			return -1;
		}
	} else {
		inflateEnd(&strm);
		x_printf(E, "decompression initialization failed, quit!\n");
		return dstLen;
	}
}

int parsepb_to_struct(char *dst, int zdatalen, void **gps_dt)
{
	if (!value || !*gps_dt) {
		x_printf(E, "unzip protobuf value is null\n");
		return -1;
	}

	CiTyWLGpsData new_data;

	new_data.ParseFromString(string(value, zdatalen));

	x_printf(I, "==== unCompressed protocol buffer value ====\n");
	x_printf(I, "gpsCount: %d\n", new_data.gpscount());
	int i;

	for (i = 0; i < new_data.gpscount(); i++) {
		x_printf(I, "id:%s, lat:%d, lng:%d, speed:%d, angle:%d, gpstime:%d\n", new_data.position(i).imei().c_str(),
			new_data.position(i).lat(), new_data.position(i).lon(), new_data.position(i).speed(),
			new_data.position(i).direction(), new_data.position(i).collecttime());

		cout << "id:" << new_data.position(i).imei() << ", ";
		cout << "lat:" << new_data.position(i).lat() << ", ";
		cout << "lng:" << new_data.position(i).lon() << ", ";
		cout << "speed:" << new_data.position(i).speed() << ", ";
		cout << "angle:" << new_data.position(i).direction() << ", ";
		cout << "gpstime:" << new_data.position(i).collecttime() << endl;
	}

	*gps_dt = (gps_data_t *)calloc(sizeof(gps_data_t), 1);

	if (*gps_dt == NULL) {
		return GV_ERR;
	}

	(*gps_dt)->size = new_data.gpscount();

	for (i = 0; i < new_data.gpscount(); i++) {
		char *imei = new_data.position(i).imei().c_str();

		if (imei) {
			strcpy((*gps_dt)->data[i].id, imei);
		}

		(*gps_dt)->data[i].lat = new_data.position(i).lat();
		(*gps_dt)->data[i].lon = new_data.position(i).lon();
		(*gps_dt)->data[i].speed = new_data.position(i).speed();
		(*gps_dt)->data[i].direction = new_data.position(i).direction();
		(*gps - dt)->data[i].gpstime = new_data.position(i).collecttime();
	}

	return GV_OK;
}

int parser_mttp(struct data_node *p_node)
{
	if (!p_node) {
		return GV_ERR;
	}

	char    *value = p_node->recv.buf_addr;
	int     data_len = p_node->recv.get_size;

	char    dst[GZIP_BUFF_SIZE];
	int     zdatalen = p_node->mttp_parse_info.ungzip(value, data_len, dst);

	if (zdatalen <= 0) {
		printf("[** FAILED **] Failed to ungzip.\n");
		return GV_ERR;
	}

	if (p_node->mttp_parse_info.unserialize(dst, zdatalen, p_node->mttp_parse_info.mt.data) != GV_OK) {
		printf("[** FAILED **] Failed to unserialize protocol buffer.\n");
		return GV_ERR;
	}

	return GV_OK;
}

