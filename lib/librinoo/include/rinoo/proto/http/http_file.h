/**
 * @file   http_file.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Apr 17 21:14:27 2012
 *
 * @brief  Header file for HTTP file
 *
 *
 */

#ifndef		RINOO_PROTO_HTTP_FILE_H_
# define	RINOO_PROTO_HTTP_FILE_H_

int rinoo_http_send_dir(t_http *http, const char *path);
int rinoo_http_send_file(t_http *http, const char *path);

#endif		/* !RINOO_PROTO_HTTP_FILE_H_ */

