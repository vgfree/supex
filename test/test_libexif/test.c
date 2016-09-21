#include <stdio.h>  
#include <assert.h>  
#include <string.h>  
#include <libexif/exif-data.h>  


char g_buf[1024*1024*30] = {0};


int get_image_gps_fmt_json(const char *buff, int size)
{	
	ExifData* ed = exif_data_new_from_data (buff, size);
	if (!ed) {  
		fprintf(stderr, "An error occur");  
		return -1;  
	}  

	exif_data_set_option(ed,   EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS);
	
	ExifContent *ec = ed->ifd[ EXIF_IFD_GPS ];
	
#if 1
	ExifIfd ifd = exif_content_get_ifd(ec);  
	printf("======IFD: %d %s======\n", ifd, exif_ifd_get_name(ifd));
	char v[1024];
#endif
	unsigned int i;
	char j_time[128] = {0};
	char j_altitude[128] = {0};
	char j_direction[128] = {0};
	char j_speed[128] = {0};
	char j_latitude[128] = {0};
	char j_longitude[128] = {0};
	for (i = 0; i < ec->count; i++) {
		ExifEntry *ee = ec->entries[i];
		switch (ee->tag) {
			case EXIF_TAG_GPS_TIME_STAMP:
				exif_entry_get_value(ee, j_time, sizeof(j_time));
				break;
			case EXIF_TAG_GPS_ALTITUDE:
				exif_entry_get_value(ee, j_altitude, sizeof(j_altitude));
				break;
			case EXIF_TAG_GPS_IMG_DIRECTION:
				exif_entry_get_value(ee, j_direction, sizeof(j_direction));
				break;
			case EXIF_TAG_GPS_SPEED:
				exif_entry_get_value(ee, j_speed, sizeof(j_speed));
				break;
			case EXIF_TAG_GPS_LATITUDE:
				exif_entry_get_value(ee, j_latitude, sizeof(j_latitude));
				break;
			case EXIF_TAG_GPS_LONGITUDE:
				exif_entry_get_value(ee, j_longitude, sizeof(j_longitude));
				break;
		}
#if 1
		printf("%x  ---> %s: %s\n", ee->tag 
				, exif_tag_get_name_in_ifd(ee->tag, ifd)  
				//, exif_tag_get_title_in_ifd(ee->tag, ifd))  
				//, exif_tag_get_description_in_ifd(ee->tag, ifd))  
			, exif_entry_get_value(ee, v, sizeof(v)));
#endif
	}
	printf("{\"GPSTime\":\"%s\",\"longitude\":\"%s\",\"latitude\":\"%s\",\"direction\":\"%s\",\"speed\":\"%s\",\"altitude\":\"%s\"}",
			j_time,
			j_longitude,
			j_latitude,
			j_direction,
			j_speed,
			j_altitude);

	exif_data_unref(ed);
	return 0;
}

int main(int argc, char** argv)  
{

	FILE *fs = fopen(argv[1], "r");
	size_t size = fread(g_buf, 1, sizeof(g_buf), fs);
	
	int ret = get_image_gps_fmt_json(g_buf, size);
	return 0;  
}
