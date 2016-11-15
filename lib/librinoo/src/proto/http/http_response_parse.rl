/**
 * @file   http_response_parse.rl
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Apr 15 22:29:07 2012
 *
 * @brief  HTTP response parsing
 *
 *
 */

#include "rinoo/proto/http/module.h"

%%{
  machine httpres_reader;
  write data;

  action startcode	{ code_start = fpc; }
  action startmsg	{ msg_start = fpc; }
  action endmsg		{ msg_end = fpc; }
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
		  rinoo_http_header_setdata(&http->response.headers, hd_start, hdv_start, (hdv_end - hdv_start));
		  *hd_end = tmp;
	  }
  }
  action okac		{
	  http->response.code = atoi(code_start);
	  buffer_static(&http->response.msg, msg_start, msg_end - msg_start);
	  if (cl_start != NULL && cl_end != NULL)
	  {
		  tmp = *cl_end;
		  *cl_end = 0;
		  http->response.content_length = atoi(cl_start);
		  *cl_end = tmp;
	  }
	  http->response.headers_length = fpc - ((char *) buffer_ptr(http->response.buffer)) + 1;
	  return 1;
  }
  action parseerror	{ return -1; }

  crlf = '\r\n';
  code = digit{3};
  msg = (ascii* -- crlf);
  http = 'HTTP/1.' ('0' %{ http->version = RINOO_HTTP_VERSION_10; } |
		    '1' %{ http->version = RINOO_HTTP_VERSION_11; });
  contentlength = 'Content-length: 'i (digit+) >startcl %endcl crlf;
  header = (alnum | punct)+ >starthead %endhead ': ' ((ascii* -- crlf) (crlf (' ' | '\t')+ (ascii+ -- crlf))*) >startheadv %endheadv crlf;
  main := (http ' ' code >startcode ' ' msg >startmsg %endmsg crlf
	   header*
	   contentlength?
	   header*
	   crlf
	   @okac) $err(parseerror);
  }%%


int rinoo_http_response_parse(t_http *http)
{
	int cs = 0;
	char *p = buffer_ptr(http->response.buffer);
	char *pe = (char *) buffer_ptr(http->response.buffer) + buffer_size(http->response.buffer);
	char *eof = NULL;
	char *code_start = NULL;
	char *msg_start = NULL;
	char *msg_end = NULL;
	char *cl_start = NULL;
	char *cl_end = NULL;
	char *hd_start = NULL;
	char *hd_end = NULL;
	char *hdv_start = NULL;
	char *hdv_end = NULL;
	char tmp;

	%% write init;
	%% write exec;

	(void) httpres_reader_en_main;
	(void) httpres_reader_error;
	(void) httpres_reader_first_final;
	return 0;
}
