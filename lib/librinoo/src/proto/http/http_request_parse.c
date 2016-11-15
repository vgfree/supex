
#line 1 "http_request_parse.rl"
/**
 * @file   http_request_parse.rl
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Apr 15 22:29:07 2012
 *
 * @brief  HTTP request parsing
 *
 *
 */

#include "rinoo/rinoo.h"


#line 17 "http_request_parse.c"
static const char _httpreq_reader_actions[] = {
	0, 1, 0, 1, 1, 1, 4, 1, 
	5, 1, 6, 1, 7, 1, 8, 1, 
	9, 1, 10, 1, 11, 1, 12, 1, 
	13, 1, 14, 1, 15, 1, 16, 1, 
	17, 1, 18, 1, 19, 2, 0, 1, 
	2, 6, 2, 2, 6, 7, 2, 7, 
	3
};

static const short _httpreq_reader_key_offsets[] = {
	0, 0, 14, 16, 18, 20, 22, 24, 
	26, 27, 31, 35, 41, 46, 51, 56, 
	61, 66, 71, 76, 82, 86, 91, 96, 
	97, 100, 104, 107, 110, 114, 121, 124, 
	129, 134, 139, 144, 149, 154, 159, 163, 
	168, 173, 178, 183, 188, 193, 196, 200, 
	207, 214, 218, 220, 222, 224, 226, 228, 
	229, 231, 233, 234, 236, 238, 240, 241, 
	243, 245, 247, 249, 251, 253, 254, 258, 
	260, 262, 263, 265, 266, 268, 270, 272, 
	274, 275
};

static const char _httpreq_reader_trans_keys[] = {
	67, 68, 71, 72, 79, 80, 84, 99, 
	100, 103, 104, 111, 112, 116, 79, 111, 
	78, 110, 78, 110, 69, 101, 67, 99, 
	84, 116, 32, 13, 32, 0, 127, 13, 
	32, 0, 127, 13, 32, 0, 9, 11, 
	127, 13, 32, 72, 0, 127, 13, 32, 
	84, 0, 127, 13, 32, 84, 0, 127, 
	13, 32, 80, 0, 127, 13, 32, 47, 
	0, 127, 13, 32, 49, 0, 127, 13, 
	32, 46, 0, 127, 13, 32, 48, 49, 
	0, 127, 13, 32, 0, 127, 10, 13, 
	32, 0, 127, 13, 67, 99, 33, 126, 
	10, 58, 33, 126, 32, 58, 33, 126, 
	13, 0, 127, 13, 0, 127, 10, 13, 
	0, 127, 9, 13, 32, 67, 99, 33, 
	126, 13, 0, 127, 13, 0, 9, 11, 
	127, 58, 79, 111, 33, 126, 58, 78, 
	110, 33, 126, 58, 84, 116, 33, 126, 
	58, 69, 101, 33, 126, 58, 78, 110, 
	33, 126, 58, 84, 116, 33, 126, 45, 
	58, 33, 126, 58, 76, 108, 33, 126, 
	58, 69, 101, 33, 126, 58, 78, 110, 
	33, 126, 58, 71, 103, 33, 126, 58, 
	84, 116, 33, 126, 58, 72, 104, 33, 
	126, 58, 33, 126, 32, 58, 33, 126, 
	13, 0, 47, 48, 57, 58, 127, 13, 
	0, 47, 48, 57, 58, 127, 13, 32, 
	0, 127, 69, 101, 76, 108, 69, 101, 
	84, 116, 69, 101, 32, 69, 101, 84, 
	116, 32, 69, 101, 65, 97, 68, 100, 
	32, 80, 112, 84, 116, 73, 105, 79, 
	111, 78, 110, 83, 115, 32, 79, 85, 
	111, 117, 83, 115, 84, 116, 32, 84, 
	116, 32, 82, 114, 65, 97, 67, 99, 
	69, 101, 32, 0
};

static const char _httpreq_reader_single_lengths[] = {
	0, 14, 2, 2, 2, 2, 2, 2, 
	1, 2, 2, 2, 3, 3, 3, 3, 
	3, 3, 3, 4, 2, 3, 3, 1, 
	1, 2, 1, 1, 2, 5, 1, 1, 
	3, 3, 3, 3, 3, 3, 2, 3, 
	3, 3, 3, 3, 3, 1, 2, 1, 
	1, 2, 2, 2, 2, 2, 2, 1, 
	2, 2, 1, 2, 2, 2, 1, 2, 
	2, 2, 2, 2, 2, 1, 4, 2, 
	2, 1, 2, 1, 2, 2, 2, 2, 
	1, 0
};

static const char _httpreq_reader_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 2, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 0, 
	1, 1, 1, 1, 1, 1, 1, 2, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 3, 
	3, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0
};

static const short _httpreq_reader_index_offsets[] = {
	0, 0, 15, 18, 21, 24, 27, 30, 
	33, 35, 39, 43, 48, 53, 58, 63, 
	68, 73, 78, 83, 89, 93, 98, 103, 
	105, 108, 112, 115, 118, 122, 129, 132, 
	136, 141, 146, 151, 156, 161, 166, 170, 
	175, 180, 185, 190, 195, 200, 203, 207, 
	212, 217, 221, 224, 227, 230, 233, 236, 
	238, 241, 244, 246, 249, 252, 255, 257, 
	260, 263, 266, 269, 272, 275, 277, 282, 
	285, 288, 290, 293, 295, 298, 301, 304, 
	307, 309
};

static const char _httpreq_reader_indicies[] = {
	1, 2, 3, 4, 5, 6, 7, 1, 
	2, 3, 4, 5, 6, 7, 0, 8, 
	8, 0, 9, 9, 0, 10, 10, 0, 
	11, 11, 0, 12, 12, 0, 13, 13, 
	0, 14, 0, 16, 17, 15, 0, 19, 
	20, 18, 0, 19, 20, 18, 18, 0, 
	19, 20, 21, 18, 0, 19, 20, 22, 
	18, 0, 19, 20, 23, 18, 0, 19, 
	20, 24, 18, 0, 19, 20, 25, 18, 
	0, 19, 20, 26, 18, 0, 19, 20, 
	27, 18, 0, 19, 20, 28, 29, 18, 
	0, 30, 20, 18, 0, 31, 19, 20, 
	18, 0, 32, 34, 34, 33, 0, 35, 
	0, 37, 36, 0, 38, 37, 36, 0, 
	40, 39, 0, 42, 41, 0, 43, 42, 
	41, 0, 44, 32, 44, 34, 34, 33, 
	0, 45, 41, 0, 42, 41, 41, 0, 
	37, 46, 46, 36, 0, 37, 47, 47, 
	36, 0, 37, 48, 48, 36, 0, 37, 
	49, 49, 36, 0, 37, 50, 50, 36, 
	0, 37, 51, 51, 36, 0, 52, 37, 
	36, 0, 37, 53, 53, 36, 0, 37, 
	54, 54, 36, 0, 37, 55, 55, 36, 
	0, 37, 56, 56, 36, 0, 37, 57, 
	57, 36, 0, 37, 58, 58, 36, 0, 
	59, 36, 0, 60, 37, 36, 0, 40, 
	39, 61, 39, 0, 62, 41, 63, 41, 
	0, 64, 20, 18, 0, 65, 65, 0, 
	66, 66, 0, 67, 67, 0, 68, 68, 
	0, 69, 69, 0, 70, 0, 71, 71, 
	0, 72, 72, 0, 73, 0, 74, 74, 
	0, 75, 75, 0, 76, 76, 0, 77, 
	0, 78, 78, 0, 79, 79, 0, 80, 
	80, 0, 81, 81, 0, 82, 82, 0, 
	83, 83, 0, 84, 0, 85, 86, 85, 
	86, 0, 87, 87, 0, 88, 88, 0, 
	89, 0, 90, 90, 0, 91, 0, 92, 
	92, 0, 93, 93, 0, 94, 94, 0, 
	95, 95, 0, 96, 0, 0, 0
};

static const char _httpreq_reader_trans_targs[] = {
	0, 2, 50, 56, 59, 63, 70, 76, 
	3, 4, 5, 6, 7, 8, 9, 10, 
	11, 12, 10, 11, 12, 13, 14, 15, 
	16, 17, 18, 19, 20, 49, 21, 22, 
	23, 24, 32, 81, 24, 25, 26, 27, 
	28, 27, 28, 29, 30, 31, 33, 34, 
	35, 36, 37, 38, 39, 40, 41, 42, 
	43, 44, 45, 46, 47, 48, 28, 48, 
	21, 51, 52, 53, 54, 55, 9, 57, 
	58, 9, 60, 61, 62, 9, 64, 65, 
	66, 67, 68, 69, 9, 71, 74, 72, 
	73, 9, 75, 9, 77, 78, 79, 80, 
	9
};

static const char _httpreq_reader_trans_actions[] = {
	15, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 31, 1, 
	1, 37, 0, 0, 3, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 33, 0, 
	0, 5, 5, 13, 0, 7, 0, 9, 
	43, 0, 11, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 7, 0, 40, 46, 0, 
	35, 0, 0, 0, 0, 0, 27, 0, 
	0, 19, 0, 0, 0, 21, 0, 0, 
	0, 0, 0, 0, 17, 0, 0, 0, 
	0, 23, 0, 25, 0, 0, 0, 0, 
	29
};

static const char _httpreq_reader_eof_actions[] = {
	0, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 
	15, 0
};

static const int httpreq_reader_start = 1;
static const int httpreq_reader_first_final = 81;
static const int httpreq_reader_error = 0;

static const int httpreq_reader_en_main = 1;


#line 67 "http_request_parse.rl"



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

	
#line 238 "http_request_parse.c"
	{
	cs = httpreq_reader_start;
	}

#line 87 "http_request_parse.rl"
	
#line 245 "http_request_parse.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _httpreq_reader_trans_keys + _httpreq_reader_key_offsets[cs];
	_trans = _httpreq_reader_index_offsets[cs];

	_klen = _httpreq_reader_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _httpreq_reader_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _httpreq_reader_indicies[_trans];
	cs = _httpreq_reader_trans_targs[_trans];

	if ( _httpreq_reader_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _httpreq_reader_actions + _httpreq_reader_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 17 "http_request_parse.rl"
	{ uri_start = p; }
	break;
	case 1:
#line 18 "http_request_parse.rl"
	{ uri_end = p; }
	break;
	case 2:
#line 19 "http_request_parse.rl"
	{ cl_start = p; }
	break;
	case 3:
#line 20 "http_request_parse.rl"
	{ cl_end = p; }
	break;
	case 4:
#line 21 "http_request_parse.rl"
	{ hd_start = p; }
	break;
	case 5:
#line 22 "http_request_parse.rl"
	{ hd_end = p; }
	break;
	case 6:
#line 23 "http_request_parse.rl"
	{ hdv_start = p; }
	break;
	case 7:
#line 24 "http_request_parse.rl"
	{
	  hdv_end = p;
	  if (hd_start != NULL && hd_end != NULL && hdv_start != NULL) {
		  tmp = *hd_end;
		  *hd_end = 0;
		  rinoo_http_header_setdata(&http->request.headers, hd_start, hdv_start, (hdv_end - hdv_start));
		  *hd_end = tmp;
	  }
  }
	break;
	case 8:
#line 33 "http_request_parse.rl"
	{
	  buffer_static(&http->request.uri, uri_start, uri_end - uri_start);
	  if (cl_start != NULL && cl_end != NULL)
	  {
		  tmp = *cl_end;
		  *cl_end = 0;
		  http->request.content_length = atoi(cl_start);
		  *cl_end = tmp;
	  }
	  http->request.headers_length = p - ((char *) buffer_ptr(http->request.buffer)) + 1;
	  return 1;
  }
	break;
	case 9:
#line 45 "http_request_parse.rl"
	{ return -1; }
	break;
	case 10:
#line 48 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_OPTIONS; }
	break;
	case 11:
#line 49 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_GET; }
	break;
	case 12:
#line 50 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_HEAD; }
	break;
	case 13:
#line 51 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_POST; }
	break;
	case 14:
#line 52 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_PUT; }
	break;
	case 15:
#line 53 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_DELETE; }
	break;
	case 16:
#line 54 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_TRACE; }
	break;
	case 17:
#line 55 "http_request_parse.rl"
	{ http->request.method = RINOO_HTTP_METHOD_CONNECT; }
	break;
	case 18:
#line 57 "http_request_parse.rl"
	{ http->version = RINOO_HTTP_VERSION_10; }
	break;
	case 19:
#line 58 "http_request_parse.rl"
	{ http->version = RINOO_HTTP_VERSION_11; }
	break;
#line 418 "http_request_parse.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _httpreq_reader_actions + _httpreq_reader_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 9:
#line 45 "http_request_parse.rl"
	{ return -1; }
	break;
#line 438 "http_request_parse.c"
		}
	}
	}

	_out: {}
	}

#line 88 "http_request_parse.rl"

	(void) httpreq_reader_en_main;
	(void) httpreq_reader_error;
	(void) httpreq_reader_first_final;
	return 0;
}
