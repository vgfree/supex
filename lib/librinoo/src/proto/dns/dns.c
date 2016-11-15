/**
 * @file   dns.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Oct 20 14:14:47 2013
 *
 * @brief  DNS functions
 *
 *
 */

#include "rinoo/proto/dns/module.h"

void rinoo_dns_init(t_sched *sched, t_dns *dns, t_dns_type type, const char *host)
{
	res_init();
	dns->socket = rinoo_udp_client(sched, (t_ip *) &(_res.nsaddr_list[0]), ntohs(_res.nsaddr_list[0].sin_port));
	dns->host = host;
	dns->answer = NULL;
	dns->authority = NULL;
	dns->additional = NULL;
	buffer_set(&dns->buffer, dns->packet, sizeof(dns->packet));
	dns->type = type;
}

void rinoo_dns_destroy(t_dns *dns)
{
	if (dns->answer != NULL) {
		free(dns->answer);
	}
	if (dns->authority != NULL) {
		free(dns->authority);
	}
	if (dns->additional != NULL) {
		free(dns->additional);
	}
	rinoo_socket_destroy(dns->socket);
}

int rinoo_dns_query(t_dns *dns, t_dns_type type, const char *host)
{
	char len;
	char *dot;
	unsigned short tmp;
	t_dns_header header = { 0 };

	header.id = htons((unsigned short) ((unsigned long long) dns % USHRT_MAX));
	DNS_QUERY_SET_QR(header.flags, 0);
	DNS_QUERY_SET_OPCODE(header.flags, 0);
	DNS_QUERY_SET_AA(header.flags, 0);
	DNS_QUERY_SET_TC(header.flags, 0);
	DNS_QUERY_SET_RD(header.flags, 1);
	DNS_QUERY_SET_RA(header.flags, 0);
	DNS_QUERY_SET_Z(header.flags, 0);
	DNS_QUERY_SET_AD(header.flags, 0);
	DNS_QUERY_SET_CD(header.flags, 0);
	DNS_QUERY_SET_RCODE(header.flags, 0);
	header.flags = htons(header.flags);
	header.qdcount = htons(1);
	header.ancount = 0;
	header.nscount = 0;
	header.arcount = 0;
	buffer_add(&dns->buffer, (char *) &header, sizeof(header));
	while (*host) {
		dot = strchrnul(host, '.');
		len = dot - host;
		buffer_add(&dns->buffer, &len, 1);
		buffer_add(&dns->buffer, host, (size_t) len);
		host += len;
		if (*host == '.') {
			host++;
		}
	}
	len = 0;
	buffer_add(&dns->buffer, &len, 1);
	/* Query type */
	tmp = htons(type);
	buffer_add(&dns->buffer, (char *) &tmp, sizeof(tmp));
	/* Query class */
	tmp = htons(1);
	buffer_add(&dns->buffer, (char *) &tmp, sizeof(tmp));
	if (rinoo_socket_writeb(dns->socket, &dns->buffer) <= 0) {
		return -1;
	}
	return 0;
}

int rinoo_dns_get(t_dns *dns, t_dns_query *query, t_ip *from)
{
	ssize_t count;
	socklen_t len;
	t_buffer_iterator iterator;

	buffer_erase(&dns->buffer, 0);
	buffer_set(&query->name.buffer, query->name.value, sizeof(query->name.value));
	len = sizeof(from->v4);
	count = rinoo_socket_recvfrom(dns->socket, buffer_ptr(&dns->buffer), buffer_msize(&dns->buffer), (struct sockaddr *) &from->v4, &len);
	if (count <= 0) {
		return -1;
	}
	dns->buffer.size = count;
	buffer_iterator_set(&iterator, &dns->buffer);
	if (rinoo_dns_header_get(&iterator, &dns->header) != 0) {
		return -1;
	}
	if (rinoo_dns_query_get(&iterator, query) != 0) {
		return -1;
	}
	return 0;
}

int rinoo_dns_reply_get(t_dns *dns, uint32_t timeout)
{
	unsigned int i;
	t_dns_query query;
	t_buffer_iterator iterator;

	buffer_erase(&dns->buffer, 0);
	buffer_set(&query.name.buffer, query.name.value, sizeof(query.name.value));
	if (rinoo_socket_timeout(dns->socket, timeout) != 0) {
		return -1;
	}
	if (rinoo_socket_readb(dns->socket, &dns->buffer) <= 0) {
		return -1;
	}
	buffer_iterator_set(&iterator, &dns->buffer);
	if (rinoo_dns_header_get(&iterator, &dns->header) != 0) {
		return -1;
	}
	if (dns->header.id != (unsigned short) ((unsigned long long) dns % USHRT_MAX)) {
		return -1;
	}
	if (rinoo_dns_query_get(&iterator, &query) != 0) {
		return -1;
	}
	if (query.type != dns->type) {
		return -1;
	}
	if (dns->header.ancount > 50) {
		return -1;
	}
	if (dns->header.nscount > 50) {
		return -1;
	}
	if (dns->header.arcount > 50) {
		return -1;
	}
	if (dns->header.ancount > 0) {
		dns->answer = malloc(sizeof(*dns->answer) * dns->header.ancount);
		if (dns->answer == NULL) {
			return -1;
		}
		for (i = 0; i < dns->header.ancount && !buffer_iterator_end(&iterator); i++) {
			if (rinoo_dns_record_get(&iterator, &dns->answer[i]) != 0) {
				return -1;
			}
		}
	}
	if (dns->header.nscount > 0) {
		dns->authority = malloc(sizeof(*dns->authority) * dns->header.nscount);
		if (dns->authority == NULL) {
			return -1;
		}
		for (i = 0; i < dns->header.nscount && !buffer_iterator_end(&iterator); i++) {
			if (rinoo_dns_record_get(&iterator, &dns->authority[i]) != 0) {
				return -1;
			}
		}
	}
	if (dns->header.arcount > 0) {
		dns->additional = malloc(sizeof(*dns->additional) * dns->header.arcount);
		if (dns->additional == NULL) {
			return -1;
		}
		for (i = 0; i < dns->header.arcount && !buffer_iterator_end(&iterator); i++) {
			if (rinoo_dns_record_get(&iterator, &dns->additional[i]) != 0) {
				return -1;
			}
		}
	}
	return 0;
}

int rinoo_dns_ip_get(t_sched *sched, const char *host, t_ip *ip)
{
	unsigned int i;
	t_dns dns;

	rinoo_dns_init(sched, &dns, DNS_TYPE_A, host);
	if (rinoo_dns_query(&dns, DNS_TYPE_A, host) != 0) {
		goto dns_error;
	}
	if (rinoo_dns_reply_get(&dns, 1000) != 0) {
		goto dns_error;
	}
	if (dns.header.ancount == 0) {
		goto dns_error;
	}
	i = 0;
	while (i < dns.header.ancount && dns.answer[i].type != DNS_TYPE_A) {
		i++;
	}
	if (i >= dns.header.ancount) {
		goto dns_error;
	}
	ip->v4.sin_family = AF_INET;
	ip->v4.sin_addr.s_addr = dns.answer[i].rdata.a.address;
	rinoo_dns_destroy(&dns);
	return 0;
dns_error:
	rinoo_dns_destroy(&dns);
	return -1;
}
