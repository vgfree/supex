/**
 * @file   dns_parse.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Sun Feb 16 00:24:11 2014
 *
 * @brief  DNS parsing functions
 *
 *
 */

#include "rinoo/proto/dns/module.h"

int rinoo_dns_header_get(t_buffer_iterator *iterator, t_dns_header *header)
{
	if (buffer_iterator_gethushort(iterator, &header->id) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &header->flags) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &header->qdcount) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &header->ancount) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &header->nscount) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &header->arcount) != 0) {
		return -1;
	}
	return 0;
}

int rinoo_dns_name_get(t_buffer_iterator *iterator, t_buffer *name)
{
	char *label;
	unsigned char size;
	unsigned short tmp;
	t_buffer_iterator tmp_iter;

	while (!buffer_iterator_end(iterator)) {
		size = *(unsigned char *)(buffer_iterator_ptr(iterator));
		if (size == 0) {
			/* Move iterator position */
			buffer_iterator_getchar(iterator, NULL);
			buffer_addnull(name);
			return 0;
		} else if (DNS_QUERY_NAME_IS_COMPRESSED(size)) {
			if (buffer_iterator_gethushort(iterator, &tmp) != 0) {
				return -1;
			}
			buffer_iterator_set(&tmp_iter, iterator->buffer);
			if (buffer_iterator_position_set(&tmp_iter, DNS_QUERY_NAME_GET_OFFSET(tmp)) != 0) {
				return -1;
			}
			if (rinoo_dns_name_get(&tmp_iter, name) != 0) {
				return -1;
			}
			/* Only end of domain can be compressed */
			return 0;
		} else {
			buffer_iterator_getchar(iterator, NULL);
			label = buffer_iterator_ptr(iterator);
			if (buffer_iterator_position_inc(iterator, size) != 0) {
				return -1;
			}
			if (buffer_size(name) > 0) {
				buffer_addstr(name, ".");
			}
			buffer_add(name, label, size);
		}
	}
	buffer_addnull(name);
	return 0;
}

int rinoo_dns_rdata_get(t_buffer_iterator *iterator, size_t rdlength, t_dns_type type, t_dns_rdata *rdata)
{
	int ip;
	size_t position;

	position = buffer_iterator_position_get(iterator);
	switch (type) {
		case DNS_TYPE_A:
			if (buffer_iterator_getint(iterator, &ip) != 0) {
				return -1;
			}
			rdata->a.address = ip;
			break;
		case DNS_TYPE_NS:
			if (rinoo_dns_name_get(iterator, &rdata->ns.nsname.buffer) != 0) {
				return -1;
			}
			break;
		case DNS_TYPE_CNAME:
			if (rinoo_dns_name_get(iterator, &rdata->cname.cname.buffer) != 0) {
				return -1;
			}
			break;
		case DNS_TYPE_SOA:
			if (rinoo_dns_name_get(iterator, &rdata->soa.mname.buffer) != 0) {
				return -1;
			}
			if (rinoo_dns_name_get(iterator, &rdata->soa.rname.buffer) != 0) {
				return -1;
			}
			if (buffer_iterator_gethuint(iterator, &rdata->soa.serial) != 0) {
				return -1;
			}
			if (buffer_iterator_gethint(iterator, &rdata->soa.refresh) != 0) {
				return -1;
			}
			if (buffer_iterator_gethint(iterator, &rdata->soa.retry) != 0) {
				return -1;
			}
			if (buffer_iterator_gethint(iterator, &rdata->soa.expire) != 0) {
				return -1;
			}
			if (buffer_iterator_gethuint(iterator, &rdata->soa.minimum) != 0) {
				return -1;
			}
			break;
		case DNS_TYPE_PTR:
			if (rinoo_dns_name_get(iterator, &rdata->ptr.ptrname.buffer) != 0) {
				return -1;
			}
			break;
		case DNS_TYPE_HINFO:
			if (rinoo_dns_name_get(iterator, &rdata->hinfo.cpu.buffer) != 0) {
				return -1;
			}
			if (rinoo_dns_name_get(iterator, &rdata->hinfo.os.buffer) != 0) {
				return -1;
			}
			break;
		case DNS_TYPE_MX:
			if (buffer_iterator_gethshort(iterator, &rdata->mx.preference) != 0) {
				return -1;
			}
			if (rinoo_dns_name_get(iterator, &rdata->mx.exchange.buffer) != 0) {
				return -1;
			}
			break;
		case DNS_TYPE_TXT:
			if (rinoo_dns_name_get(iterator, &rdata->txt.txtdata.buffer) != 0) {
				return -1;
			}
			break;
		case DNS_TYPE_AAAA:
			/* FIXME: Support of IPv6 */
			if (buffer_iterator_position_inc(iterator, 16) != 0) {
				return -1;
			}
			break;
		default:
			return -1;
	}
	if (buffer_iterator_position_get(iterator) - position != rdlength) {
		return -1;
	}
	return 0;
}

int rinoo_dns_query_get(t_buffer_iterator *iterator, t_dns_query *query)
{
	if (rinoo_dns_name_get(iterator, &query->name.buffer) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &query->type) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &query->qclass) != 0) {
		return -1;
	}
	return 0;
}

int rinoo_dns_record_get(t_buffer_iterator *iterator, t_dns_record *record)
{
	buffer_set(&record->name.buffer, record->name.value, sizeof(record->name.value));
	if (rinoo_dns_name_get(iterator, &record->name.buffer) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &record->type) != 0) {
		return -1;
	}
	switch (record->type) {
		case DNS_TYPE_A:
			break;
		case DNS_TYPE_NS:
			buffer_set(&record->rdata.ns.nsname.buffer, record->rdata.ns.nsname.value, sizeof(record->rdata.ns.nsname.value));
			break;
		case DNS_TYPE_CNAME:
			buffer_set(&record->rdata.cname.cname.buffer, record->rdata.cname.cname.value, sizeof(record->rdata.cname.cname.value));
			break;
		case DNS_TYPE_SOA:
			buffer_set(&record->rdata.soa.mname.buffer, record->rdata.soa.mname.value, sizeof(record->rdata.soa.mname.value));
			buffer_set(&record->rdata.soa.rname.buffer, record->rdata.soa.rname.value, sizeof(record->rdata.soa.rname.value));
			break;
		case DNS_TYPE_PTR:
			buffer_set(&record->rdata.ptr.ptrname.buffer, record->rdata.ptr.ptrname.value, sizeof(record->rdata.ptr.ptrname));
			break;
		case DNS_TYPE_HINFO:
			buffer_set(&record->rdata.hinfo.cpu.buffer, record->rdata.hinfo.cpu.value, sizeof(record->rdata.hinfo.cpu.value));
			buffer_set(&record->rdata.hinfo.os.buffer, record->rdata.hinfo.os.value, sizeof(record->rdata.hinfo.os.value));
			break;
		case DNS_TYPE_MX:
			buffer_set(&record->rdata.mx.exchange.buffer, record->rdata.mx.exchange.value, sizeof(record->rdata.mx.exchange.value));
			break;
		case DNS_TYPE_TXT:
			buffer_set(&record->rdata.txt.txtdata.buffer, record->rdata.txt.txtdata.value, sizeof(record->rdata.txt.txtdata.value));
			break;
	}
	if (buffer_iterator_gethushort(iterator, &record->aclass) != 0) {
		return -1;
	}
	if (buffer_iterator_gethint(iterator, &record->ttl) != 0) {
		return -1;
	}
	if (buffer_iterator_gethushort(iterator, &record->rdlength) != 0) {
		return -1;
	}
	if (rinoo_dns_rdata_get(iterator, record->rdlength, record->type, &record->rdata) != 0) {
		return -1;
	}
	return 0;
}
