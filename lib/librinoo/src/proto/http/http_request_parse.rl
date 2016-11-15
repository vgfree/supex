/**
 * @file   http_request_parse.rl
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Apr 15 22:29:07 2012
 *
 * @brief  HTTP request parsing
 *
 *
 */

#include "rinoo/proto/http/module.h"

%%{
  machine httpreq_reader;
  write data;

  action starturi	{ uri_start = fpc; }
  action enduri		{ uri_end = fpc; }
  action startcl	{ cl_start = fpc; }
  action endcl		{ cl_end = fpc; }
  action starthead	{ hd_start = fpc; }
  action endhead	{ hd_end = fpc; }
  action startheadv	{ hdv_start = fpc; }
  action endheadv	{
	  hdv_end = fpc;
	  if (hd_start != NULL && hd_end != NULL && hdv_start != NULL) {
		  tmp = *hd_end;
		  *hd_end = 0;
		  rinoo_http_header_setdata(&http->request.headers, hd_start, hdv_start, (hdv_end - hdv_start));
		  *hd_end = tmp;
	  }
  }
  action okac		{
	  buffer_static(&http->request.uri, uri_start, uri_end - uri_start);
	  if (cl_start != NULL && cl_end != NULL)
	  {
		  tmp = *cl_end;
		  *cl_end = 0;
		  http->request.content_length = atoi(cl_start);
		  *cl_end = tmp;
	  }
	  http->request.headers_length = fpc - ((char *) buffer_ptr(http->request.buffer)) + 1;
	  return 1;
  }
  action parseerror	{ return -1; }

  crlf = '\r\n';
  method = ( 'OPTIONS'i %{ http->request.method = RINOO_HTTP_METHOD_OPTIONS; } |
	     'GET'i %{ http->request.method = RINOO_HTTP_METHOD_GET; } |
	     'HEAD'i %{ http->request.method = RINOO_HTTP_METHOD_HEAD; } |
	     'POST'i %{ http->request.method = RINOO_HTTP_METHOD_POST; } |
	     'PUT'i %{ http->request.method = RINOO_HTTP_METHOD_PUT; } |
	     'DELETE'i %{ http->request.method = RINOO_HTTP_METHOD_DELETE; } |
	     'TRACE'i %{ http->request.method = RINOO_HTTP_METHOD_TRACE; } |
	     'CONNECT'i %{ http->request.method = RINOO_HTTP_METHOD_CONNECT; } );
  uri = (ascii* -- crlf);
  http = 'HTTP/1.' ('0' %{ http->version = RINOO_HTTP_VERSION_10; } |
		    '1' %{ http->version = RINOO_HTTP_VERSION_11; });
  contentlength = 'Content-length: 'i (digit+) >startcl %endcl crlf;
  header = (alnum | punct)+ >starthead %endhead ': ' ((ascii* -- crlf) (crlf (' ' | '\t')+ (ascii+ -- crlf))*) >startheadv %endheadv crlf;
  main := (method ' ' uri >starturi %enduri ' ' http crlf
	   header*
	   contentlength?
	   header*
	   crlf
	   @okac) $err(parseerror);
  }%%


int rinoo_http_request_parse(t_http *http)
{
	int cs = 0;
	char *p = buffer_ptr(http->request.buffer);
	char *pe = (char *) buffer_ptr(http->request.buffer) + buffer_size(http->request.buffer);
	char *eof = NULL;
	char *uri_start = NULL;
	char *uri_end = NULL;
	char *cl_start = NULL;
	char *cl_end = NULL;
	char *hd_start = NULL;
	char *hd_end = NULL;
	char *hdv_start = NULL;
	char *hdv_end = NULL;
	char tmp;

	%% write init;
	%% write exec;

	(void) httpreq_reader_en_main;
	(void) httpreq_reader_error;
	(void) httpreq_reader_first_final;
	return 0;
}
