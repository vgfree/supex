/**
 * @file   http_header.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Jan  9 18:59:50 2011
 *
 * @brief  Functions to manager HTTP headers
 *
 *
 */

#include "rinoo/proto/http/module.h"

/**
 * Compare function used in the HTTP header hashtable to sort entries.
 *
 * @param node1 Pointer to node1
 * @param node2 Pointer to node2
 *
 * @return -1 if they are different (no need to sort them), 0 if they are equal
 */
static int rinoo_http_header_cmp(t_rbtree_node *node1, t_rbtree_node *node2)
{

	t_http_header *header1 = container_of(node1, t_http_header, node);
	t_http_header *header2 = container_of(node2, t_http_header, node);

	return buffer_cmp(&header1->key, &header2->key);
}

/**
 * Free's a HTTP header structure.
 *
 * @param node Pointer to the HTTP header.
 */
static void rinoo_http_header_free(t_rbtree_node *node)
{
	t_http_header *header = container_of(node, t_http_header, node);

	free(buffer_ptr(&header->key));
	free(buffer_ptr(&header->value));
	free(header);
}

/**
 * Initializes a HTTP header tree.
 *
 * @param tree HTTP header tree to initialize
 *
 * @return 0 on success, otherwise -1
 */
int rinoo_http_headers_init(t_rbtree *tree)
{
	return rbtree(tree, rinoo_http_header_cmp, rinoo_http_header_free);
}

/**
 * Flushes a HTTP header tree.
 *
 * @param headertree Pointer to the hashtable to destroy.
 */
void rinoo_http_headers_flush(t_rbtree *headertree)
{
	rbtree_flush(headertree);
}

/**
 *  Adds a new HTTP header to the hashtable.
 *
 * @param headertree Pointer to the hashtable where to store new header.
 * @param key HTTP header key.
 * @param value HTTP header value.
 * @param size Size of the HTTP header value.
 *
 * @return 0 on success, or -1 if an error occurs.
 */
int rinoo_http_header_setdata(t_rbtree *headertree, const char *key, const char *value, uint32_t size)
{
	char *new_value;
	t_http_header *new;
	t_http_header dummy;
	t_rbtree_node *found;

	XASSERT(headertree != NULL, -1);
	XASSERT(key != NULL, -1);
	XASSERT(value != NULL, -1);
	XASSERT(size > 0, -1);

	strtobuffer(&dummy.key, key);
	new_value = strndup(value, size);
	found = rbtree_find(headertree, &dummy.node);
	if (found != NULL) {
		new = container_of(found, t_http_header, node);
		free(new->value.ptr);
		strtobuffer(&new->value, new_value);
		return 0;
	}

	new = calloc(1, sizeof(*new));
	if (new == NULL) {
		free(new_value);
		return -1;
	}
	key = strdup(key);
	if (key == NULL) {
		free(new_value);
		free(new);
		return -1;
	}
	strtobuffer(&new->key, key);
	strtobuffer(&new->value, new_value);
	if (rbtree_put(headertree, &new->node) != 0) {
		rinoo_http_header_free(&new->node);
		return -1;
	}
	return 0;
}

/**
 * Adds a new HTTP header to the hashtable.
 *
 * @param headertree Pointer to the hashtable where to store new header.
 * @param key HTTP header key.
 * @param value HTTP header value.
 *
 * @return 0 on success, or -1 if an error occurs.
 */
int rinoo_http_header_set(t_rbtree *headertree, const char *key, const char *value)
{
	return rinoo_http_header_setdata(headertree, key, value, strlen(value));
}

/**
 * Removes a HTTP header from the hashtable.
 *
 * @param headertree Pointer to the hashtable to use.
 * @param key HTTP header key.
 */
void rinoo_http_header_remove(t_rbtree *headertree, const char *key)
{
	t_http_header dummy;
	t_rbtree_node *toremove;

	XASSERTN(headertree != NULL);
	XASSERTN(key != NULL);

	strtobuffer(&dummy.key, key);
	toremove = rbtree_find(headertree, &dummy.node);
	if (toremove != NULL) {
		rbtree_remove(headertree, toremove);
	}
}

/**
 * Looks for a HTTP header and returns the corresponding structure.
 *
 * @param headertree Pointer to the hashtable to use.
 * @param key HTTP header key.
 *
 * @return A pointer to a HTTP header structure, or NULL if not found.
 */
t_http_header *rinoo_http_header_get(t_rbtree *headertree, const char *key)
{
	t_http_header dummy;
	t_rbtree_node *node;

	XASSERT(headertree != NULL, NULL);
	XASSERT(key != NULL, NULL);

	strtobuffer(&dummy.key, key);
	node = rbtree_find(headertree, &dummy.node);
	if (node == NULL) {
		return NULL;
	}
	return container_of(node, t_http_header, node);
}
