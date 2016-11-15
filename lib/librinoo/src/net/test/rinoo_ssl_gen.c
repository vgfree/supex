/**
 * @file   rinoo_ssl_gen.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Mar 20 16:59:54 2012
 *
 * @brief  Test file for rinoossl function
 *
 *
 */

#include "rinoo/rinoo.h"

/**
 * Main function for this unit test.
 *
 * @return 0 if test passed
 */
int main()
{
	t_ssl_ctx *ssl;

	ssl = rinoo_ssl_context();
	XTEST(ssl != NULL);
	rinoo_ssl_context_destroy(ssl);
	XPASS();
}
