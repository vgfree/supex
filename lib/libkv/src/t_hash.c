/*
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "kv_inner.h"
#include <math.h>

/*-----------------------------------------------------------------------------
 * Hash type API
 *----------------------------------------------------------------------------*/

/* Check the length of a number of objects to see if we need to convert a
 * ziplist to a real hash. Note that we only check string encoded objects
 * as their string length can be queried in constant time. */
void hashTypeTryConversion(struct kv_handler *handler, robj *o, robj **argv, int start, int end)
{
        int i;

        if (o->encoding != REDIS_ENCODING_ZIPLIST) return;

        for (i = start; i <= end; i++) {
                if (sdsEncodedObject(argv[i]) &&
                    sdslen(argv[i]->ptr) > handler->hash_max_ziplist_value)
                {
                        hashTypeConvert(handler, o, REDIS_ENCODING_HT);
                        break;
                }
        }
}

/* Encode given objects in-place when the hash uses a dict. */
void hashTypeTryObjectEncoding(struct kv_handler *handler, robj *subject, robj **o1, robj **o2)
{
        if (subject->encoding == REDIS_ENCODING_HT) {
                if (o1) *o1 = tryObjectEncoding(handler, *o1);
                if (o2) *o2 = tryObjectEncoding(handler, *o2);
        }
}

/* Get the value from a ziplist encoded hash, identified by field.
 * Returns -1 when the field cannot be found. */
int hashTypeGetFromZiplist(robj *o, robj *field,
                           unsigned char **vstr,
                           unsigned int *vlen,
                           long long *vll)
{
    unsigned char *zl, *fptr = NULL, *vptr = NULL;
    int ret;

    logicErrorExpr(o->encoding == REDIS_ENCODING_ZIPLIST, "Never happend");

    field = getDecodedObject(field);

    zl = o->ptr;
    fptr = ziplistIndex(zl, ZIPLIST_HEAD);
    if (fptr != NULL) {
        fptr = ziplistFind(fptr, field->ptr, sdslen(field->ptr), 1);
        if (fptr != NULL) {
            /* Grab pointer to the value (fptr points to the field) */
            vptr = ziplistNext(zl, fptr);
            logicErrorExpr(vptr != NULL, "Never happend");
        }
    }

    decrRefCount(field);

    if (vptr != NULL) {
        ret = ziplistGet(vptr, vstr, vlen, vll);
        logicErrorExpr(ret, "Never happend");
        return 0;
    }

    return -1;
}

/* Get the value from a hash table encoded hash, identified by field.
 * Returns -1 when the field cannot be found. */
int hashTypeGetFromHashTable(robj *o, robj *field, robj **value) {
    dictEntry *de;

    logicErrorExpr(o->encoding == REDIS_ENCODING_HT, "Invalid type");

    de = dictFind(o->ptr, field);
    if (de == NULL) return -1;
    *value = dictGetVal(de);
    return 0;
}

/* Higher level function of hashTypeGet*() that always returns a Redis
 * object (either new or with refcount incremented), so that the caller
 * can retain a reference or call decrRefCount after the usage.
 *
 * The lower level function can prevent copy on write so it is
 * the preferred way of doing read operations. */
robj *hashTypeGetObject(struct kv_handler *handler, robj *o, robj *field) {
        robj *value = NULL;

        if (o->encoding == REDIS_ENCODING_ZIPLIST) {
                unsigned char *vstr = NULL;
                unsigned int vlen = UINT_MAX;
                long long vll = LLONG_MAX;

                if (hashTypeGetFromZiplist(o, field, &vstr, &vlen, &vll) == 0) {
                        if (vstr) {
                                value = createStringObject((char*)vstr, vlen);
                        } else {
                                value = createStringObjectFromLongLong(handler, vll);
                        }
                }

        } else if (o->encoding == REDIS_ENCODING_HT) {
                robj *aux;

                if (hashTypeGetFromHashTable(o, field, &aux) == 0) {
                        incrRefCount(aux);
                        value = aux;
                }
        } else {
                logicError("Unknown hash encoding");
        }
        return value;
}

/* Test if the specified field exists in the given hash. Returns 1 if the field
 * exists, and 0 when it doesn't. */
int hashTypeExists(robj *o, robj *field) {
    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *vstr = NULL;
        unsigned int vlen = UINT_MAX;
        long long vll = LLONG_MAX;

        if (hashTypeGetFromZiplist(o, field, &vstr, &vlen, &vll) == 0) return 1;
    } else if (o->encoding == REDIS_ENCODING_HT) {
        robj *aux;

        if (hashTypeGetFromHashTable(o, field, &aux) == 0) return 1;
    } else {
        logicError("Unknown hash encoding");
    }
    return 0;
}

/* Add an element, discard the old if the key already exists.
 * Return 0 on insert and 1 on update.
 * This function will take care of incrementing the reference count of the
 * retained fields and value objects. */
int hashTypeSet(struct kv_handler *handler, robj *o, robj *field, robj *value) {
        int update = 0;

        if (o->encoding == REDIS_ENCODING_ZIPLIST) {
                unsigned char *zl, *fptr, *vptr;

                field = getDecodedObject(field);
                value = getDecodedObject(value);

                zl = o->ptr;
                fptr = ziplistIndex(zl, ZIPLIST_HEAD);
                if (fptr != NULL) {
                        fptr = ziplistFind(fptr, field->ptr, sdslen(field->ptr), 1);
                        if (fptr != NULL) {
                                /* Grab pointer to the value (fptr points to the field) */
                                vptr = ziplistNext(zl, fptr);
                                logicErrorExpr(vptr != NULL, "Never happend");
                                update = 1;

                                /* Delete value */
                                zl = ziplistDelete(zl, &vptr);

                                /* Insert new value */
                                zl = ziplistInsert(zl, vptr, value->ptr, sdslen(value->ptr));
                        }
                }

                if (!update) {
                        /* Push new field/value pair onto the tail of the ziplist */
                        zl = ziplistPush(zl, field->ptr, sdslen(field->ptr), ZIPLIST_TAIL);
                        zl = ziplistPush(zl, value->ptr, sdslen(value->ptr), ZIPLIST_TAIL);
                }
                o->ptr = zl;
                decrRefCount(field);
                decrRefCount(value);

                /* Check if the ziplist needs to be converted to a hash table */
                if (hashTypeLength(o) > handler->hash_max_ziplist_entries)
                        hashTypeConvert(handler, o, REDIS_ENCODING_HT);
        } else if (o->encoding == REDIS_ENCODING_HT) {
                if (dictReplace(o->ptr, field, value)) { /* Insert */
                        incrRefCount(field);
                } else { /* Update */
                        update = 1;
                }
                incrRefCount(value);
        } else {
                logicError("Unknown hash encoding");
        }
        return update;
}

/* Delete an element from a hash.
 * Return 1 on deleted and 0 on not found. */
int hashTypeDelete(robj *o, robj *field) {
    int deleted = 0;

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *zl, *fptr;

        field = getDecodedObject(field);

        zl = o->ptr;
        fptr = ziplistIndex(zl, ZIPLIST_HEAD);
        if (fptr != NULL) {
            fptr = ziplistFind(fptr, field->ptr, sdslen(field->ptr), 1);
            if (fptr != NULL) {
                zl = ziplistDelete(zl,&fptr);
                zl = ziplistDelete(zl,&fptr);
                o->ptr = zl;
                deleted = 1;
            }
        }

        decrRefCount(field);

    } else if (o->encoding == REDIS_ENCODING_HT) {
        if (dictDelete((dict*)o->ptr, field) == REDIS_OK) {
            deleted = 1;

            /* Always check if the dictionary needs a resize after a delete. */
            if (htNeedsResize(o->ptr)) dictResize(o->ptr);
        }

    } else {
        logicError("Unknown hash encoding");
    }

    return deleted;
}

/* Return the number of elements in a hash. */
unsigned long hashTypeLength(robj *o) {
    unsigned long length = ULONG_MAX;

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        length = ziplistLen(o->ptr) / 2;
    } else if (o->encoding == REDIS_ENCODING_HT) {
        length = dictSize((dict*)o->ptr);
    } else {
        logicError("Unknown hash encoding");
    }

    return length;
}

hashTypeIterator *hashTypeInitIterator(robj *subject) {
    hashTypeIterator *hi = zmalloc(sizeof(hashTypeIterator));
    hi->subject = subject;
    hi->encoding = subject->encoding;

    if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
        hi->fptr = NULL;
        hi->vptr = NULL;
    } else if (hi->encoding == REDIS_ENCODING_HT) {
        hi->di = dictGetIterator(subject->ptr);
    } else {
        logicError("Unknown hash encoding");
    }

    return hi;
}

void hashTypeReleaseIterator(hashTypeIterator *hi) {
    if (hi->encoding == REDIS_ENCODING_HT) {
        dictReleaseIterator(hi->di);
    }

    zfree(hi);
}

/* Move to the next entry in the hash. Return REDIS_OK when the next entry
 * could be found and REDIS_ERR when the iterator reaches the end. */
int hashTypeNext(hashTypeIterator *hi) {
    if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *zl;
        unsigned char *fptr, *vptr;

        zl = hi->subject->ptr;
        fptr = hi->fptr;
        vptr = hi->vptr;

        if (fptr == NULL) {
            /* Initialize cursor */
                logicErrorExpr(vptr == NULL, "Never happend");
            fptr = ziplistIndex(zl, 0);
        } else {
            /* Advance cursor */
                logicErrorExpr(vptr != NULL, "Never happend");
            fptr = ziplistNext(zl, vptr);
        }
        if (fptr == NULL) return REDIS_ERR;

        /* Grab pointer to the value (fptr points to the field) */
        vptr = ziplistNext(zl, fptr);
        logicErrorExpr(vptr != NULL, "Never happend");

        /* fptr, vptr now point to the first or next pair */
        hi->fptr = fptr;
        hi->vptr = vptr;
    } else if (hi->encoding == REDIS_ENCODING_HT) {
        if ((hi->de = dictNext(hi->di)) == NULL) return REDIS_ERR;
    } else {
        logicError("Unknown hash encoding");
    }
    return REDIS_OK;
}

/* Get the field or value at iterator cursor, for an iterator on a hash value
 * encoded as a ziplist. Prototype is similar to `hashTypeGetFromZiplist`. */
void hashTypeCurrentFromZiplist(hashTypeIterator *hi, int what,
                                unsigned char **vstr,
                                unsigned int *vlen,
                                long long *vll)
{
    int ret;

    logicErrorExpr(hi->encoding == REDIS_ENCODING_ZIPLIST, "Invalid type");

    if (what & REDIS_HASH_KEY) {
        ret = ziplistGet(hi->fptr, vstr, vlen, vll);
        logicErrorExpr(ret, "Never happend");
    } else {
            ret = ziplistGet(hi->vptr, vstr, vlen, vll);
            logicErrorExpr(ret, "Never happend");
    }
}

/* Get the field or value at iterator cursor, for an iterator on a hash value
 * encoded as a ziplist. Prototype is similar to `hashTypeGetFromHashTable`. */
void hashTypeCurrentFromHashTable(hashTypeIterator *hi, int what, robj **dst) {
        logicErrorExpr(hi->encoding == REDIS_ENCODING_HT, "Invalid type");

    if (what & REDIS_HASH_KEY) {
        *dst = dictGetKey(hi->de);
    } else {
        *dst = dictGetVal(hi->de);
    }
}

/* A non copy-on-write friendly but higher level version of hashTypeCurrent*()
 * that returns an object with incremented refcount (or a new object). It is up
 * to the caller to decrRefCount() the object if no reference is retained. */
robj *hashTypeCurrentObject(struct kv_handler *handler, hashTypeIterator *hi, int what) {
        robj *dst;

        if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
                unsigned char *vstr = NULL;
                unsigned int vlen = UINT_MAX;
                long long vll = LLONG_MAX;

                hashTypeCurrentFromZiplist(hi, what, &vstr, &vlen, &vll);
                if (vstr) {
                        dst = createStringObject((char*)vstr, vlen);
                } else {
                        dst = createStringObjectFromLongLong(handler, vll);
                }
        } else if (hi->encoding == REDIS_ENCODING_HT) {
                hashTypeCurrentFromHashTable(hi, what, &dst);
                incrRefCount(dst);
        } else {
                logicError("Unknown hash encoding");
        }
        return dst;
}

robj* hashTypeLookupWriteElseCreate(struct kv_handler *handler, reply_t *c, robj *key)
{
        robj *o = lookupKeyWrite(handler, c->db,key);
        if (o == NULL) {
                o = createHashObject();
                dbAdd(c->db,key,o);
        } else {
                if (o->type != REDIS_HASH) {
                        reply_set_err(c, ERR_TYPE);
                        return NULL;
                }
        }
        return o;
}

void hashTypeConvertZiplist(struct kv_handler *handler, robj *o, int enc) {
        logicErrorExpr(o->encoding == REDIS_ENCODING_ZIPLIST, "Invalid type");

        if (enc == REDIS_ENCODING_ZIPLIST) {
                /* Nothing to do... */

        } else if (enc == REDIS_ENCODING_HT) {
                hashTypeIterator *hi;
                dict *dict;
                int ret;

                hi = hashTypeInitIterator(o);
                dict = dictCreate(&hashDictType, NULL);

                while (hashTypeNext(hi) != REDIS_ERR) {
                        robj *field, *value;

                        field = hashTypeCurrentObject(handler, hi, REDIS_HASH_KEY);
                        field = tryObjectEncoding(handler, field);
                        value = hashTypeCurrentObject(handler, hi, REDIS_HASH_VALUE);
                        value = tryObjectEncoding(handler, value);
                        ret = dictAdd(dict, field, value);
                        if (ret != DICT_OK) {
                                logicErrorExpr(ret == DICT_OK, "Never happend");
                        }
                }

                hashTypeReleaseIterator(hi);
                zfree(o->ptr);

                o->encoding = REDIS_ENCODING_HT;
                o->ptr = dict;

        } else {
                logicError("Unknown hash encoding");
        }
}

void hashTypeConvert(struct kv_handler *handler, robj *o, int enc)
{
        if (o->encoding == REDIS_ENCODING_ZIPLIST) {
                hashTypeConvertZiplist(handler, o, enc);
        } else if (o->encoding == REDIS_ENCODING_HT) {
                logicError("Not implemented");
        } else {
                logicError("Unknown hash encoding");
        }
}

/*-----------------------------------------------------------------------------
 * Hash type commands
 *----------------------------------------------------------------------------*/

void hsetCommand(struct kv_handler *handler)
{
    robj *o;
    int update;
    reply_t *c = handler->reply;

    if ((o = hashTypeLookupWriteElseCreate(handler, c,c->argv[1])) == NULL) return;

    hashTypeTryConversion(handler, o,c->argv,2,3);
    hashTypeTryObjectEncoding(handler, o,&c->argv[2], &c->argv[3]);
    update = hashTypeSet(handler, o,c->argv[2],c->argv[3]);

    update = update ? 0 : 1;
    reply_add_result(c, handler->shared.integers[update]);
    incrRefCount(handler->shared.integers[update]);
    handler->dirty++;
}

void hmsetCommand(struct kv_handler *handler)
{
	int i;
	reply_t *c = handler->reply;
	robj *o;
	
	if((c->argc %2) ==1) {
		reply_set_err(c,ERR_ARGUMENTS);
		return ;
	}
	
	if((o = hashTypeLookupWriteElseCreate(handler, c, c->argv[1])) == NULL) return ;
	hashTypeTryConversion(handler, o, c->argv, 2, c->argc-1);
	for(i = 2; i < c->argc; i+=2) {
		hashTypeTryObjectEncoding(handler, o, &c->argv[i], &c->argv[i+1]);
		hashTypeSet(handler, o, c->argv[i], c->argv[i+1]);
	}

	handler->dirty ++;
	reply_add_result(c, handler->shared.ok);
	incrRefCount(handler->shared.ok);
}
static void addHashFieldToReply(reply_t *c, robj *o, robj *field)
{
	int ret;
	robj *result;
	if(o ==NULL) {
		reply_set_err(c, ERR_NIL);
	}

        if (o->encoding == REDIS_ENCODING_ZIPLIST) {
                unsigned char *vstr = NULL;
                unsigned int vlen = UINT_MAX;
                long long vll = LLONG_MAX;

                ret = hashTypeGetFromZiplist(o,field,&vstr,&vlen,&vll);
                if (ret < 0) {
                        reply_set_err(c, ERR_NIL);
                        return;
                } else {
                        if (vstr) {
                                result = createObject(REDIS_STRING, sdsnewlen(vstr, vlen));
                                reply_add_result(c, result);
                                return;
                        } else {
                                result = createObject(REDIS_STRING, (void*)vll);
                                result->encoding = REDIS_ENCODING_INT;
                                reply_add_result(c, result);
                                return;
                        }
                }
        } else if (o->encoding == REDIS_ENCODING_HT) { 
                ret = hashTypeGetFromHashTable(o,field,&result);
                if (ret < 0) {
                        reply_set_err(c, ERR_NIL);
                        return;
                } else {
                        reply_add_result(c, result);
                        incrRefCount(result);
                        return;
                }
        } else {
                logicError("Unknown hash encoding");
        }
}

void hgetCommand(struct kv_handler *handler)
{
        reply_t *c = handler->reply;
        robj *o;
        robj *field = c->argv[2];

        if ((o = lookupKeyRead(handler, c->db, c->argv[1])) == NULL) {
                reply_set_err(c, ERR_NIL);
                return;
        }

        if (o->type != REDIS_HASH) {
                reply_set_err(c, ERR_TYPE);
                return;
        }
	
	addHashFieldToReply(c, o, field);
}

void hmgetCommand(struct kv_handler *handler)
{
	reply_t *c = handler->reply;
        robj *o;
	int i;

        if ((o = lookupKeyRead(handler, c->db, c->argv[1])) == NULL) {
                reply_set_err(c, ERR_NIL);
                return;
        }

        if (o->type != REDIS_HASH) {
                reply_set_err(c, ERR_TYPE);
                return;
        }
	
	for(i =2; i<c->argc; i++) {
		addHashFieldToReply(c, o, c->argv[i]);
	}
}

void hdelCommand(struct kv_handler *handler)
{
	reply_t *c = handler->reply;
	robj *o;
	int i,deleted = 0;

	if((o = lookupKeyWrite(handler, c->db, c->argv[1])) ==NULL) {
		reply_add_result_long(c,deleted);
		return ;
	}

	if(o->type != REDIS_HASH) {
		reply_set_err(c,ERR_TYPE);
		return ;
	}

	for(i =2; i < c->argc; i++) {
		if(hashTypeDelete(o, c->argv[i])) {
			deleted++;
			handler->dirty++;
			if(hashTypeLength(o) == 0) {
				dbDelete(c->db, c->argv[1]);
				break;
			}
		}
	}

	reply_add_result_long(c, deleted);
}

static void addHashIteratorCursorToReply(reply_t *c, hashTypeIterator *hi, int what) {
	robj *result;

	if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
		unsigned char *vstr = NULL;
		unsigned int vlen = UINT_MAX;
		long long vll = LLONG_MAX;
		 
		hashTypeCurrentFromZiplist(hi, what, &vstr, &vlen, &vll);
		if (vstr) {
			result = createObject(REDIS_STRING, sdsnewlen(vstr,vlen));
			reply_add_result(c, result);
			return ;
		} else {
			result = createObject(REDIS_STRING,(void*)vll);
			result->encoding = REDIS_ENCODING_INT;
			reply_add_result(c, result);
			return ;
		}
	} else if (hi->encoding == REDIS_ENCODING_HT) {
		robj *value;
		hashTypeCurrentFromHashTable(hi, what, &value);
		sds str= value->ptr;
		reply_add_result_str(c, str);
		}
	else {
		logicError("Unknown hash encoding");
	}
}

void _hgetall(struct kv_handler *handler, reply_t *c, int flags)
{
	robj *o;
	hashTypeIterator *hi;
	if ((o = lookupKeyRead(handler, c->db, c->argv[1])) ==NULL) {
		reply_set_err(c, ERR_NIL);
		return ;
	}
	if (o->type !=REDIS_HASH) {
		reply_set_err(c, ERR_TYPE);
		return ;
	}

	hi = hashTypeInitIterator(o);	
	while (hashTypeNext(hi) != REDIS_ERR) {
		if (flags & REDIS_HASH_KEY) {
			addHashIteratorCursorToReply(c, hi, REDIS_HASH_KEY);
		}
		if (flags & REDIS_HASH_VALUE) {
			addHashIteratorCursorToReply(c, hi, REDIS_HASH_VALUE);
		}
	}
	hashTypeReleaseIterator(hi);
} 

void hgetallCommand(struct kv_handler *handler)
{
	reply_t *c =handler->reply;
	int flags = REDIS_HASH_KEY | REDIS_HASH_VALUE;
	_hgetall(handler, c, flags);		
}
