/**
 * @file   dns.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Sun Oct 20 11:12:51 2013
 *
 * @brief  Header file for DNS structure
 *
 *
 */

#ifndef RINOO_PROTO_DNS_DNS_H_
#define RINOO_PROTO_DNS_DNS_H_

typedef enum s_dns_type{
	DNS_TYPE_A = 0x01,
	DNS_TYPE_NS = 0x02,
	/* DNS_TYPE_MD = 0x03, Obsolete */
	/* DNS_TYPE_MF = 0x04, Obsolete */
	DNS_TYPE_CNAME = 0x05,
	DNS_TYPE_SOA = 0x06,
	/* DNS_TYPE_MB = 0x07 Experimental */
	/* DNS_TYPE_MG = 0x08 Experimental */
	/* DNS_TYPE_MR = 0x09 Experimental */
	/* DNS_TYPE_NULL = 0x0a Experimental */
	/* DNS_TYPE_WKS = 0x0b Unused */
	DNS_TYPE_PTR = 0x0c,
	DNS_TYPE_HINFO = 0x0d,
	/* DNS_TYPE_MINFO = 0x0e, Experimental */
	DNS_TYPE_MX = 0x0f,
	DNS_TYPE_TXT = 0x10,
	DNS_TYPE_AAAA = 0x1c
} t_dns_type;

typedef struct s_dns_name {
	char value[256];
	t_buffer buffer;
} t_dns_name;

typedef struct s_dns_header {
	unsigned short id;
	unsigned short flags;
	unsigned short qdcount;
	unsigned short ancount;
	unsigned short nscount;
	unsigned short arcount;
} t_dns_header;

typedef struct s_dns_query {
	t_dns_name name;
	unsigned short type;
	unsigned short qclass;
} t_dns_query;

/* RDATA */
/* Yes, there is some useless structs here, but it perfectly matches RFC. */

typedef struct s_dns_rdata_a {
	unsigned long address;
} t_dns_rdata_a;

typedef struct s_dns_rdata_ns {
	t_dns_name nsname;
} t_dns_rdata_ns;

typedef struct s_dns_rdata_cname {
	t_dns_name cname;
} t_dns_rdata_cname;

typedef struct s_dns_rdata_soa {
	t_dns_name mname;
	t_dns_name rname;
	unsigned int serial;
	int refresh;
	int retry;
	int expire;
	unsigned int minimum;
} t_dns_rdata_soa;

typedef struct s_dns_rdata_ptr {
	t_dns_name ptrname;
} t_dns_rdata_ptr;

typedef struct s_dns_rdata_hinfo {
	t_dns_name cpu;
	t_dns_name os;
} t_dns_rdata_hinfo;

typedef struct s_dns_rdata_mx {
	short preference;
	t_dns_name exchange;
} t_dns_rdata_mx;

typedef struct s_dns_rdata_txt {
	t_dns_name txtdata;
} t_dns_rdata_txt;

typedef struct s_dns_rdata_aaaa {
	char aaaadata[16];
} t_dns_rdata_aaaa;

typedef union u_dns_rdata {
	t_dns_rdata_a a;
	t_dns_rdata_ns ns;
	t_dns_rdata_cname cname;
	t_dns_rdata_soa soa;
	t_dns_rdata_ptr ptr;
	t_dns_rdata_hinfo hinfo;
	t_dns_rdata_mx mx;
	t_dns_rdata_txt txt;
	t_dns_rdata_aaaa aaaa;
} t_dns_rdata;

typedef struct s_dns_record {
	t_dns_name name;
	unsigned short type;
	unsigned short aclass;
	int ttl;
	unsigned short rdlength;
	t_dns_rdata rdata;
} t_dns_record;

typedef struct s_dns {
	t_buffer buffer;
	char packet[512];
	const char *host;
	t_dns_type type;
	t_socket *socket;
	t_dns_header header;
	t_dns_record *answer;
	t_dns_record *authority;
	t_dns_record *additional;
} t_dns;

#define DNS_QUERY_SET_QR(flags, value)		do { flags = (flags & ~0x8000) | (value << 15); } while (0)
#define DNS_QUERY_SET_OPCODE(flags, value)	do { flags = (flags & ~0x7800) | (0x7800 & (value << 11)); } while (0)
#define DNS_QUERY_SET_AA(flags, value)		do { flags = (flags & ~0x0400) | (0x0400 & (value << 10)); } while (0)
#define DNS_QUERY_SET_TC(flags, value)		do { flags = (flags & ~0x0200) | (0x0200 & (value << 9)); } while (0)
#define DNS_QUERY_SET_RD(flags, value)		do { flags = (flags & ~0x0100) | (0x0100 & (value << 8)); } while (0)
#define DNS_QUERY_SET_RA(flags, value)		do { flags = (flags & ~0x0080) | (0x0080 & (value << 7)); } while (0)
#define DNS_QUERY_SET_Z(flags, value)		do { flags = (flags & ~0x0040) | (0x0040 & (value << 6)); } while (0)
#define DNS_QUERY_SET_AD(flags, value)		do { flags = (flags & ~0x0020) | (0x0020 & (value << 5)); } while (0)
#define DNS_QUERY_SET_CD(flags, value)		do { flags = (flags & ~0x0010) | (0x0010 & (value << 4)); } while (0)
#define DNS_QUERY_SET_RCODE(flags, value)	do { flags = (flags & ~0x000f) | (0x000f & (value)); } while (0)

#define DNS_QUERY_GET_QR(flags)			(flags & 0x8000)
#define DNS_QUERY_GET_OPCODE(flags)		(flags & 0x7800)
#define DNS_QUERY_GET_AA(flags)			(flags & 0x0400)
#define DNS_QUERY_GET_TC(flags)			(flags & 0x0200)
#define DNS_QUERY_GET_RD(flags)			(flags & 0x0100)
#define DNS_QUERY_GET_RA(flags)			(flags & 0x0080)
#define DNS_QUERY_GET_Z(flags)			(flags & 0x0070)
#define DNS_QUERY_GET_RCODE(flags)		(flags & 0x000f)

#define DNS_QUERY_NAME_IS_COMPRESSED(byte)	((byte & 0xc0) == 0xc0)
#define DNS_QUERY_NAME_GET_OFFSET(offset)	(offset & 0x3fff)

void rinoo_dns_init(t_sched *sched, t_dns *dns, t_dns_type type, const char *host);
void rinoo_dns_destroy(t_dns *dns);
int rinoo_dns_ip_get(t_sched *sched, const char *host, t_ip *ip);
int rinoo_dns_query(t_dns *dns, t_dns_type type, const char *host);
int rinoo_dns_header_get(t_buffer_iterator *iterator, t_dns_header *header);
int rinoo_dns_name_get(t_buffer_iterator *iterator, t_buffer *name);
int rinoo_dns_rdata_get(t_buffer_iterator *iterator, size_t rdlength, t_dns_type type, t_dns_rdata *rdata);
int rinoo_dns_query_get(t_buffer_iterator *iterator, t_dns_query *query);
int rinoo_dns_record_get(t_buffer_iterator *iterator, t_dns_record *record);
int rinoo_dns_get(t_dns *dns, t_dns_query *query, t_ip *from);
int rinoo_dns_reply_get(t_dns *dns, uint32_t timeout);

#endif /* !RINOO_PROTO_DNS_DNS_H_ */
