/*
 * Author       : chenzutao
 * Date         : 2015-10-15
 * Function     : protobuf_encode.cc
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "protobuf_encode.h"
#include "henanwl-transfer-data.pb.h"
#include "decode_data.h"
#include "stdio.h"
#include "stdlib.h"

#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/stubs/shared_ptr.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "google/protobuf/stubs/scoped_ptr.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message_lite.h"

using namespace std;

typedef autonavi::traffic::fp::receiver::positionprotocol::HeNanWLPosition HeNanWLPosition;
typedef autonavi::traffic::fp::receiver::positionprotocol::HeNanWLGpsData HeNanWLGpsData;

using google::protobuf::io::FileOutputStream;
using google::protobuf::io::GzipOutputStream;
using google::protobuf::io::ArrayOutputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::uint8;
using google::protobuf::io::GzipInputStream;
using google::protobuf::io::ArrayInputStream;
using google::protobuf::io::StringOutputStream;

bool WriteToOutput(ZeroCopyOutputStream *output, const void *data, int size)
{
	const uint8     *in = reinterpret_cast <const uint8 *> (data);
	int             in_size = size;

	void    *out;
	int     out_size;

	while (true) {
		if (!output->Next(&out, &out_size)) {
			return false;
		}

		if (in_size <= out_size) {
			memcpy(out, in, in_size);
			output->BackUp(out_size - in_size);
			return true;
		}

		memcpy(out, in, out_size);
		in += out_size;
		in_size -= out_size;
	}
}

void WriteString(ZeroCopyOutputStream *output, const string &str)
{
	WriteToOutput(output, str.c_str(), str.size());
}

int WriteStuff(ZeroCopyOutputStream *output)
{
	WriteString(output, "hello world !\n");
	WriteString(output, "Some test.");
	WriteString(output, "xt. Blah blash .");

	int result = output->ByteCount();
	return result;
}

#if 1
int single_data_encode(HeNanWLPosition *henanwl_position, gps_data_t *gps_data, int idx)
{
	if (!gps_data) {
		x_printf(E, "gps data invalid !\n");
		return -1;
	}

	henanwl_position->set_id(gps_data->id);
	henanwl_position->set_lat(gps_data->lat[idx]);
	henanwl_position->set_lng(gps_data->lng[idx]);
	henanwl_position->set_speed(gps_data->speed[idx]);
	henanwl_position->set_angle(gps_data->angle[idx]);
	henanwl_position->set_gpstime(gps_data->gpsTime[idx]);
	return 0;
}

// len : gps points in every package
// cnt : how many gps point will be encoded to protocol buffer
int pack_data_encode(HeNanWLGpsData *henanwl_gpsdata, gps_data_t *gps_data[], int len, int cnt)
{
	int i, j;

	for (i = 0; i < cnt; i++) {
		for (j = 0; j < len; j++) {
			HeNanWLPosition *position = henanwl_gpsdata->add_position();
			single_data_encode(position, gps_data[i], j);
		}
	}

	int count = len * cnt;
	henanwl_gpsdata->set_gpscount(count);
	return count;
}
#endif	// if 1

string compress(HeNanWLGpsData *data)
{
	string str;

	data->SerializeToString(&str);

	string                          result = "";
	StringOutputStream              output(&result);
	GzipOutputStream::Options       options;
	options.format = GzipOutputStream::GZIP;

	GzipOutputStream gzout(&output, options);
	WriteToOutput(&gzout, str.data(), str.size());

	return result;
}

string uncompress(const string &data)
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

int cjson_to_gzpb(gps_data_t **gps_data, int len, int cnt, char **result)
{
	HeNanWLGpsData  henanwl_gpsdata;
	int             count = pack_data_encode(&henanwl_gpsdata, gps_data, len, cnt);

	x_printf(D, "%d data changed to protobuf .\n", count);
	string ret = compress(&henanwl_gpsdata);

	*result = (char *)calloc(sizeof(char), ret.size());
	memcpy(*result, ret.data(), ret.size());
	return ret.size();
}

int gzpb_to_str(char *value, int data_len)
{
	if (!value) {
		x_printf(E, "value is null\n");
		return -1;
	}

	string          uncom_ret = uncompress(string(value, data_len));
	HeNanWLGpsData  new_data;
	new_data.ParseFromString(uncom_ret);

	x_printf(D, "==== unCompressed protocol buffer value ====\n");
	cout << "gpsCount:" << new_data.gpscount() << endl;
	int i;

	for (i = 0; i < new_data.gpscount(); i++) {
		cout << "id:" << new_data.position(i).id() << ", ";
		cout << "lat:" << new_data.position(i).lat() << ", ";
		cout << "lng:" << new_data.position(i).lng() << ", ";
		cout << "speed:" << new_data.position(i).speed() << ", ";
		cout << "angle:" << new_data.position(i).angle() << ", ";
		cout << "gpstime:" << new_data.position(i).gpstime() << endl;
	}

	return 0;
}

#if 0
int main()
{
	gps_data_t      *value[10] = { NULL };
	char            data[] = "{\"longitude\":[121.3613973,121.361327,121.3612662,121.3612118,121.3611633],\"latitude\":[31.2253355,31.2253577,31.2253755,31.2253873,31.225387],\"speed\":[26,22,19,16,14],\"tokenCode\":\"yuzwlqU8lX\",\"GPSTime\":[1407210985,1407210985,1407210985,1407210985,1407210985],\"direction\":[111,111,108,93,82]}";
	int             ret = decode_data(&value[0], data);

	if (ret < 0) {
		x_printf(E, "decode data error.\n");
		return -1;
	}

	int     len = 5;
	int     cnt = 1;
	char    *result = NULL;
	int     data_len = cjson_to_gzpb(value, len, cnt, &result);

	if (!result) {
		x_printf(E, "cjson to gzip protobuf error .\n");
		return -1;
	}

	x_printf(D, "data len:%d\n", data_len);

	gzpb_to_str(result, data_len);
	return 0;
}
#endif	// if 0

