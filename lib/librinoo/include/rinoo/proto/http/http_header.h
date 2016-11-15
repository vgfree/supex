/**
 * @file   http_header.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Jan  9 19:57:32 2011
 *
 * @brief  Header file which describes HTTP header functions
 *
 *
 */

#ifndef RINOO_PROTO_HTTP_HEADER_H_
#define RINOO_PROTO_HTTP_HEADER_H_

typedef struct s_http_header {
	t_buffer key;
	t_buffer value;
	t_rbtree_node node;
} t_http_header;

int rinoo_http_headers_init(t_rbtree *tree);
void rinoo_http_headers_flush(t_rbtree *headertree);
int rinoo_http_header_setdata(t_rbtree *headertree, const char *key, const char *value, uint32_t size);
int rinoo_http_header_set(t_rbtree *headertree, const char *key, const char *value);
void rinoo_http_header_remove(t_rbtree *headertree, const char *key);
t_http_header *rinoo_http_header_get(t_rbtree *headertab, const char *key);

#endif /* !RINOO_PROTO_HTTP_HEADER_H_ */
