/**
 * @file   utils.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Mar 20 18:15:13 2012
 *
 * @brief  Utility functions
 *
 *
 */

#include "rinoo/global/module.h"

/**
 * Log function
 *
 * @param format printf's like format.
 */
void rinoo_log(const char *format, ...)
{
	uint32_t i;
	uint32_t res;
	uint32_t offset;
	char *logline;
	char *esclogline;
	va_list ap;
	struct tm tmp;
	struct timeval tv;

	logline = malloc(sizeof(*logline) * RINOO_LOG_MAXLENGTH);
	XASSERTN(logline != NULL);
	esclogline = malloc(sizeof(*esclogline) * RINOO_LOG_MAXLENGTH);
	XASSERTN(esclogline != NULL);
	XASSERTN(gettimeofday(&tv, NULL) == 0);
	XASSERTN(localtime_r(&tv.tv_sec, &tmp) != NULL);
	offset = strftime(logline, RINOO_LOG_MAXLENGTH, "[%Y/%m/%d %T.", &tmp);
	if (offset == 0) {
		free(logline);
		free(esclogline);
		XASSERTN(0);
	}
	offset += snprintf(logline + offset, RINOO_LOG_MAXLENGTH - offset, "%03d] ", (int) (tv.tv_usec / 1000));
	va_start(ap, format);
	res = vsnprintf(logline + offset, RINOO_LOG_MAXLENGTH - offset, format, ap);
	va_end(ap);
	res = (offset + res > RINOO_LOG_MAXLENGTH ? RINOO_LOG_MAXLENGTH : offset + res);
	for (i = 0, offset = 0; i < res && offset < RINOO_LOG_MAXLENGTH - 3; i++, offset++) {
		switch (logline[i]) {
		case '\a':
			strcpy(esclogline + offset, "\\a");
			offset++;
			break;
		case '\b':
			strcpy(esclogline + offset, "\\b");
			offset++;
			break;
		case '\t':
			strcpy(esclogline + offset, "\\t");
			offset++;
			break;
		case '\n':
			strcpy(esclogline + offset, "\\n");
			offset++;
			break;
		case '\v':
			strcpy(esclogline + offset, "\\v");
			offset++;
			break;
		case '\f':
			strcpy(esclogline + offset, "\\f");
			offset++;
			break;
		case '\r':
			strcpy(esclogline + offset, "\\r");
			offset++;
			break;
		case '"':
			strcpy(esclogline + offset, "\\\"");
			offset++;
			break;
		case '\\':
			strcpy(esclogline + offset, "\\\\");
			offset++;
			break;
		default:
			if (isprint(logline[i]) == 0)
			{
				/*
				 * Not printable characters are represented
				 * by their octal value
				 */
				snprintf(esclogline + offset,
					 RINOO_LOG_MAXLENGTH - offset,
					 "\\%03o",
					 (unsigned char) logline[i]);
				offset += 2;
			}
			else
			{
				esclogline[offset] = logline[i];
			}
			break;
		}
	}
	esclogline[offset++] = '\n';
	printf("%.*s", offset, esclogline);
	free(logline);
	free(esclogline);
}
