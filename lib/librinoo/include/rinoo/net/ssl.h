/**
 * @file   ssl.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Jun  5 10:45:19 2012
 *
 * @brief  Header file for Secure TCP socket function declaractions
 *
 *
 */

#ifndef RINOO_NET_SSL_H_
#define RINOO_NET_SSL_H_

typedef struct s_ssl_ctx {
	X509 *x509;
	EVP_PKEY *pkey;
	SSL_CTX *ctx;
} t_ssl_ctx;

typedef struct s_ssl {
	SSL *ssl;
	t_ssl_ctx *ctx;
	t_socket socket;
} t_ssl;

t_ssl_ctx *rinoo_ssl_context(void);
void rinoo_ssl_context_destroy(t_ssl_ctx *ctx);
t_ssl *rinoo_ssl_get(t_socket *socket);
t_socket *rinoo_ssl_client(t_sched *sched, t_ssl_ctx *ctx, t_ip *ip, uint32_t port, uint32_t timeout);
t_socket *rinoo_ssl_server(t_sched *sched, t_ssl_ctx *ctx, t_ip *ip, uint32_t port);
t_socket *rinoo_ssl_accept(t_socket *socket, t_ip *fromip, uint32_t *fromport);

#endif /* !RINOO_NET_SSL_H_ */
