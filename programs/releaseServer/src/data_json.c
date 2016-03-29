#include <stdio.h>
#include <string.h>
#include <json.h>
#include "data_json.h"
#include "log.h"
#include <cJSON.h>

cJSON *json_imei_get(char *data, char *imei, int n)
{
	cJSON   *json = NULL;
	cJSON   *value = NULL;

	json = cJSON_Parse(data);

	if (!json) {
		log_info(LOG_E, "cJSON_Parse Error before: [%s]\n", cJSON_GetErrorPtr());
		return NULL;
	}

	value = cJSON_GetObjectItem(json, "IMEI");

	if (!value) {
		log_info(LOG_E, "没有 IMEI\n");
		cJSON_Delete(json);
		return NULL;
	}

	if (strlen(value->valuestring) > n - 1) {
		log_info(LOG_E, "imei[%s]长度大于[%d]\n", value->valuestring, n - 1);
		cJSON_Delete(json);
		return NULL;
	}

	strncpy(imei, value->valuestring, n - 1);

	//	cJSON_Delete(json);
	return json;
}

