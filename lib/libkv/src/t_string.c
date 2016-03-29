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
#include <math.h> /* isnan(), isinf() */

/*-----------------------------------------------------------------------------
 * String Commands
 *----------------------------------------------------------------------------*/


/* The setGenericCommand() function implements the SET operation with different
 * options and variants. This function is called in order to implement the
 * following commands: SET, SETEX, PSETEX, SETNX.
 *
 * 'flags' changes the behavior of the command (NX or XX, see belove).
 *
 * 'expire' represents an expire to set in form of a Redis object as passed
 * by the user. It is interpreted according to the specified 'unit'.
 *
 * 'ok_reply' and 'abort_reply' is what the function will reply to the client
 * if the operation is performed, or when it is not because of NX or
 * XX flags.
 *
 * If ok_reply is NULL "+OK" is used.
 * If abort_reply is NULL, "$-1" is used. */

#define REDIS_SET_NO_FLAGS 0
#define REDIS_SET_NX (1<<0)     /* Set if key not exists. */
#define REDIS_SET_XX (1<<1)     /* Set if key exists. */

/* SET key value [NX] [XX] [EX <seconds>] [PX <milliseconds>] */
void setCommand(struct kv_handler *handler)
{
        reply_t *c = handler->reply;
        
        c->argv[2] = tryObjectEncoding(handler, c->argv[2]);
	setKey(handler,c->db,c->argv[1],c->argv[2]);
        handler->dirty++;
        reply_add_result(c, handler->shared.ok);
        incrRefCount(handler->shared.ok);
}

void getCommand(struct kv_handler *handler) {
        robj* o;
        reply_t *c = handler->reply;
        o = lookupKeyRead(handler, c->db, c->argv[1]);
        if (o) {
                if (o->type != REDIS_STRING) {
                        reply_set_err(c, ERR_TYPE);
                        return ;
                }
                reply_add_result(c, o);
                incrRefCount(o);
        } else {
                reply_set_err(c, ERR_NIL);
        }
}


//void _incrCommand(caller_t *c, int incr)
void _incrCommand(struct kv_handler *handler, int incr)
{
        robj *o;
        robj *new;
        long long value, oldvalue;
        reply_t *c = handler->reply;

        o = lookupKeyWrite(handler, c->db, c->argv[1]);
        if (o && o->type != REDIS_STRING) {
                reply_set_err(c, ERR_TYPE);
                return;
        }
        if (getLongLongFromObject(o, &value) != REDIS_OK) {
                reply_set_err(c, ERR_VALUE);
                return;
        }

        oldvalue = value;
        if ((incr < 0 && oldvalue < 0 && incr < (LLONG_MIN - oldvalue)) ||
            (incr > 0 && oldvalue > 0 && incr > (LLONG_MAX - oldvalue))) {
                reply_set_err(c, ERR_VALUE);
                return;
        }

        value += incr;
        if (o && o->refcount == 1 && o->encoding == REDIS_ENCODING_INT &&
            (value < 0 || value >= REDIS_SHARED_INTEGERS) &&
            (value >= LONG_MIN && value <= LONG_MAX))
        {
                new = o;
                new->ptr = (void*)((long)value);
        } else {
                new = createStringObjectFromLongLong(handler, value);
                if (o) {
                        dbOverwrite(c->db, c->argv[1], new);
                } else {
                        dbAdd(c->db, c->argv[1], new);
                }
        }
        handler->dirty++;
        reply_add_result(c, new);
        incrRefCount(new);
}

//void incrCommand(caller_t *c)
void incrCommand(struct kv_handler *handler)
{
        _incrCommand(handler, 1);
}

void incrbyCommand(struct kv_handler *handler)
{
        long long value = 0;
        reply_t *c = handler->reply;
        
        if (getLongLongFromObject(c->argv[2], &value) != REDIS_OK) {
                reply_set_err(c, ERR_VALUE);
                return;
        }
        _incrCommand(handler, value);
}

void decrCommand(struct kv_handler *handler)
{
        _incrCommand(handler, -1);
}

void decrbyCommand(struct kv_handler *handler)
{
        long long value = 0;
        reply_t *c = handler->reply;
        
        if (getLongLongFromObject(c->argv[2], &value) != REDIS_OK) {
                reply_set_err(c, ERR_VALUE);
                return;
        }
        _incrCommand(handler, -value);
}

void echoCommand(struct kv_handler *handler)
{
	reply_t *c =handler->reply;
	sds str=c->argv[1]->ptr;
	
	reply_add_result_str(c,str);
}

void msetCommand(struct kv_handler *handler)
{	
	int index;
	reply_t *c = handler->reply;

	if(c ->argc % 2 == 0) {
		reply_set_err(c,ERR_ARGUMENTS);
		return;
	}
	for(index =1; index < c ->argc; index += 2) {
		c ->argv[index+1] = tryObjectEncoding(handler, c ->argv[index+1]);
		setKey(handler, c->db, c->argv[index], c->argv[index+1]);
	}
	handler->dirty +=(c ->argc-1)/2;
	reply_add_result(c, handler->shared.ok);
	incrRefCount(handler->shared.ok);
}
