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
#include <signal.h>
#include <ctype.h>

/*-----------------------------------------------------------------------------
 * C-level DB API
 *----------------------------------------------------------------------------*/

void setExpire(struct kv_handler *handler, redisDb *db, robj *key, long long when)
{
        dictEntry *kde, *de, *tn;

        kde = dictFind(db->dict, key->ptr);
        logicErrorExpr(kde != NULL, "Db(dict) failed");
        tn = dictFind(db->expires, key->ptr);
        if (tn != NULL) { /** remove from expire tree */
                kv_expire_manager_remove_complete(handler, dictGetSignedIntegerVal(tn), key->ptr);
        }
        de = dictReplaceRaw(db->expires, dictGetKey(kde));
        dictSetSignedIntegerVal(de, when);
        kv_expire_manager_add(handler, when, dictGetKey(de)); /** add to expire tree */
}

long long getExpire(redisDb *db, robj *key)
{
        dictEntry *de;
        
        if (dictSize(db->expires) == 0 ||
            (de = dictFind(db->expires, key->ptr)) == NULL)
        {
                return -1;
        }

        logicErrorExpr(dictFind(db->dict, key->ptr) != NULL, "Db failed");
        return dictGetSignedIntegerVal(de);
}

int removeExpire(redisDb *db, robj *key)
{
        logicErrorExpr(dictFind(db->dict, key->ptr) != NULL, "Db failed");
        return dictDelete(db->expires, key->ptr) == DICT_OK;
}

/*
 * @param count -1 delete all the expire keys.
 */
int del_expire_keys(struct kv_handler *handler, redisDb *db, int count)
{
        REDIS_NOTUSED(db);
        return kv_expire_manager_remove(handler, count);
}

robj *lookupKey(redisDb *db, robj *key) {
        dictEntry *de = dictFind(db->dict,key->ptr);
        if (de) {
                robj *val = dictGetVal(de);
                return val;
        } else {
                return NULL;
        }
}

robj *lookupKeyRead(struct kv_handler *handler, redisDb *db, robj *key) {
    robj *val;

    del_expire_keys(handler, &handler->db[handler->dbindex], -1);
    val = lookupKey(db,key);
    if (val == NULL)
            handler->stat_keyspace_misses++;
    else
            handler->stat_keyspace_hits++;
    return val;
}

robj *lookupKeyWrite(struct kv_handler *handler, redisDb *db, robj *key)
{
        del_expire_keys(handler, &handler->db[handler->dbindex], -1);
        return lookupKey(db,key);
}

/* Add the key to the DB. It's up to the caller to increment the reference
 * counter of the value if needed.
 *
 * The program is aborted if the key already exists. */
void dbAdd(redisDb *db, robj *key, robj *val) {
    sds copy = sdsdup(key->ptr);
    int retval = dictAdd(db->dict, copy, val);

    logicErrorExpr(retval == REDIS_OK, "dictAdd error!");
}

/* Overwrite an existing key with a new value. Incrementing the reference
 * count of the new value is up to the caller.
 * This function does not modify the expire time of the existing key.
 *
 * The program is aborted if the key was not already present. */
void dbOverwrite(redisDb *db, robj *key, robj *val) {
    dictEntry *de = dictFind(db->dict,key->ptr);

    logicErrorExpr(de != NULL, NULL);
    dictReplace(db->dict, key->ptr, val);
}

/* High level Set operation. This function can be used in order to set
 * a key, whatever it was existing or not, to a new object.
 *
 * 1) The ref count of the value object is incremented.
 * 2) clients WATCHing for the destination key notified.
 * 3) The expire time of the key is reset (the key is made persistent). */
void setKey(struct kv_handler *handler, redisDb *db, robj *key, robj *val)
{
        if (lookupKeyWrite(handler, db, key) == NULL) {
                dbAdd(db, key, val);
        } else {
                dbOverwrite(db, key, val);
        }

        incrRefCount(val);
        removeExpire(db, key);
}

int dbExists(redisDb *db, robj *key) {
    return dictFind(db->dict,key->ptr) != NULL;
}

/* Delete a key, value, and associated expiration entry if any, from the DB */
int dbDelete(redisDb *db, robj *key)
{
        if (dictSize(db->expires) > 0) dictDelete(db->expires,key->ptr);
        if (dictDelete(db->dict,key->ptr) == DICT_OK) {
                return 1;
        } else {
                return 0;
        }
}

int selectDb(struct kv_handler *handler, int id)
{
        reply_t *c = handler->reply;
        if (id < 0 || id >= handler->dbnum)
                return REDIS_ERR;
        c->db = &(handler->db[id]);
        handler->dbindex = id;
        return REDIS_OK;
}

void flushdbCommand(struct kv_handler *handler)
{
        reply_t *c = handler->reply;
        
        handler->dirty += dictSize(c->db->dict);
        dictEmpty(c->db->dict, NULL);
        dictEmpty(c->db->expires, NULL);
        kv_expire_manager_clear(handler, handler->db[handler->dbindex].expire_manager);

        reply_add_result(c, handler->shared.ok);
        incrRefCount(handler->shared.ok);
}

void flushallCommand(struct kv_handler *handler)
{
        int i;
        reply_t *c = handler->reply;

        for (i = 0; i < handler->dbnum; i++) {
                dictEmpty(handler->db[i].dict, NULL);
                dictEmpty(handler->db[i].expires, NULL);
                kv_expire_manager_clear(handler, handler->db[i].expire_manager);
        }
        reply_add_result(c, handler->shared.ok);
        incrRefCount(handler->shared.ok);
}

void delCommand(struct kv_handler *handler)
{
        int i;
        int del = 0;
        reply_t *c = handler->reply;

        del_expire_keys(handler, &handler->db[handler->dbindex], -1);
        
        for (i = 1; i < c->argc; i++) {
                if (dbDelete(c->db, c->argv[i])) {
                        del++;
                        handler->dirty++;
                }
        }
        reply_add_result_long(c, del);
}

void existsCommand(struct kv_handler *handler)
{
        robj *result;
        reply_t *c = handler->reply;

        del_expire_keys(handler, &handler->db[handler->dbindex], -1);
        if (dbExists(c->db, c->argv[1]))
                result = handler->shared.integers[1];
        else
                result = handler->shared.integers[0];

        incrRefCount(result);
        reply_add_result(c, result);
        return;
}

void selectCommand(struct kv_handler *handler)
{
        long long id;
        reply_t *c = handler->reply;

        del_expire_keys(handler, &handler->db[handler->dbindex], -1);
        if (getLongLongFromObject(c->argv[1], &id) != REDIS_OK) {
                reply_set_err(c, ERR_DB_INDEX);
                return;
        }

        if (selectDb(handler, id) == REDIS_ERR) {
                reply_set_err(c, ERR_DB_INDEX);
                return;
        }

        reply_add_result(c, handler->shared.ok);
        incrRefCount(handler->shared.ok);
}

void _expire(struct kv_handler *handler, long long basetime, int unit)
{
        reply_t *c = handler->reply;
        long long when;
        robj *key = c->argv[1], *param = c->argv[2];
        robj *de;

        if (getLongLongFromObject(param, &when) != REDIS_OK) {
                reply_set_err(c, ERR_VALUE);
                return;
        }

        if (unit == UNIT_SECONDS) when *= 1000;
        when += basetime;
        if ((de = lookupKeyRead(handler, c->db, key)) == NULL) {
                reply_add_result(c, handler->shared.integers[0]);
                incrRefCount(handler->shared.integers[0]);
                return;
        }

        if (mstime() >= when) {
                dictEntry *de;
                de = dictFind(c->db->expires, key->ptr);
                if (de != NULL) { /** delete from expire tree */
                        kv_expire_manager_remove_complete(handler, dictGetSignedIntegerVal(de), key->ptr);
                }

                dbDelete(c->db, key);
                reply_add_result(c, handler->shared.integers[1]);
                incrRefCount(handler->shared.integers[1]);
                handler->dirty++;
                return;
        } else {
                setExpire(handler, c->db, key, when);
                reply_add_result(c, handler->shared.integers[1]);
                incrRefCount(handler->shared.integers[1]);
                handler->dirty++;
                return;
        }
}

void expireCommand(struct kv_handler *handler)
{
        _expire(handler, mstime(), UNIT_SECONDS);
}

void expireatCommand(struct kv_handler *handler)
{
        _expire(handler, 0, UNIT_SECONDS);  
}

void pexpireCommand(struct kv_handler *handler)
{
        _expire(handler, mstime(), UNIT_MILLISECONDS);
}

void pexpireatCommand(struct kv_handler *handler)
{
        _expire(handler, 0, UNIT_MILLISECONDS);
}

/* This callback is used by scanGenericCommand in order to collect elements
 * returned by the dictionary iterator into a list. */
void scanCallback(void *privdata, const dictEntry *de) {
    void **pd = (void**) privdata;
    list *keys = pd[0];
    robj *o = pd[1];
    robj *key = NULL, *val = NULL;

    if (o == NULL) {
        sds sdskey = dictGetKey(de);
        key = createStringObject(sdskey, sdslen(sdskey));
    } else if (o->type == REDIS_SET) {
        key = dictGetKey(de);
        incrRefCount(key);
    } else if (o->type == REDIS_HASH) {
        key = dictGetKey(de);
        incrRefCount(key);
        val = dictGetVal(de);
        incrRefCount(val);
    } else if (o->type == REDIS_ZSET) {
        key = dictGetKey(de);
        incrRefCount(key);
        val = createStringObjectFromLongDouble(*(double*)dictGetVal(de),0);
    } else {
        logicError("Type not handled in SCAN callback.");
    }

    listAddNodeTail(keys, key);
    if (val) listAddNodeTail(keys, val);
}


void dbsizeCommand(struct kv_handler *handler)
{
        reply_t *c = handler->reply;

        del_expire_keys(handler, &handler->db[handler->dbindex], -1);
        reply_add_result_long(c, dictSize(c->db->dict));
}

void typeCommand(struct kv_handler *handler)
{
        robj *o;
        char* type;
        reply_t *c = handler->reply;

        o = lookupKeyRead(handler, c->db, c->argv[1]);
        if (o) {
                type = "unknow";
                switch(o->type) {
                case REDIS_HASH:   type = "hash"; break;
                case REDIS_LIST:   type = "list"; break;
                case REDIS_SET:    type = "set";  break;
                case REDIS_ZSET:   type = "zset"; break;
                case REDIS_STRING: type = "string"; break;
                }
        } else {
                type = "none";
        }

        reply_add_result_str(c, type);
        return;
}

/* -----------------------------------------------------------------------------
 * API to get key arguments from commands
 * ---------------------------------------------------------------------------*/

/* The base case is to use the keys position as given in the command table
 * (firstkey, lastkey, step). */
int *getKeysUsingCommandTable(struct redisCommand *cmd,robj **argv, int argc, int *numkeys) {
    int j, i = 0, last, *keys;
    REDIS_NOTUSED(argv);

    if (cmd->firstkey == 0) {
        *numkeys = 0;
        return NULL;
    }
    last = cmd->lastkey;
    if (last < 0) last = argc+last;
    keys = zmalloc(sizeof(int)*((last - cmd->firstkey)+1));
    for (j = cmd->firstkey; j <= last; j += cmd->keystep) {
            logicErrorExpr(j < argc, "Invalid argument");
        keys[i++] = j;
    }
    *numkeys = i;
    return keys;
}

/* Return all the arguments that are keys in the command passed via argc / argv.
 *
 * The command returns the positions of all the key arguments inside the array,
 * so the actual return value is an heap allocated array of integers. The
 * length of the array is returned by reference into *numkeys.
 *
 * 'cmd' must be point to the corresponding entry into the redisCommand
 * table, according to the command name in argv[0].
 *
 * This function uses the command table if a command-specific helper function
 * is not required, otherwise it calls the command-specific function. */
int *getKeysFromCommand(struct redisCommand *cmd, robj **argv, int argc, int *numkeys) {
    if (cmd->getkeys_proc) {
        return cmd->getkeys_proc(cmd,argv,argc,numkeys);
    } else {
        return getKeysUsingCommandTable(cmd,argv,argc,numkeys);
    }
}

/* Free the result of getKeysFromCommand. */
void getKeysFreeResult(int *result) {
    zfree(result);
}

/* Helper function to extract keys from following commands:
 * ZUNIONSTORE <destkey> <num-keys> <key> <key> ... <key> <options>
 * ZINTERSTORE <destkey> <num-keys> <key> <key> ... <key> <options> */
int *zunionInterGetKeys(struct redisCommand *cmd, robj **argv, int argc, int *numkeys) {
    int i, num, *keys;
    REDIS_NOTUSED(cmd);

    num = atoi(argv[2]->ptr);
    /* Sanity check. Don't return any key if the command is going to
     * reply with syntax error. */
    if (num > (argc-3)) {
        *numkeys = 0;
        return NULL;
    }

    /* Keys in z{union,inter}store come from two places:
     * argv[1] = storage key,
     * argv[3...n] = keys to intersect */
    keys = zmalloc(sizeof(int)*(num+1));

    /* Add all key positions for argv[3...n] to keys[] */
    for (i = 0; i < num; i++) keys[i] = 3+i;

    /* Finally add the argv[1] key position (the storage key target). */
    keys[num] = 1;
    *numkeys = num+1;  /* Total keys = {union,inter} keys + storage key */
    return keys;
}

/* Helper function to extract keys from the following commands:
 * EVAL <script> <num-keys> <key> <key> ... <key> [more stuff]
 * EVALSHA <script> <num-keys> <key> <key> ... <key> [more stuff] */
int *evalGetKeys(struct redisCommand *cmd, robj **argv, int argc, int *numkeys) {
    int i, num, *keys;
    REDIS_NOTUSED(cmd);

    num = atoi(argv[2]->ptr);
    /* Sanity check. Don't return any key if the command is going to
     * reply with syntax error. */
    if (num > (argc-3)) {
        *numkeys = 0;
        return NULL;
    }

    keys = zmalloc(sizeof(int)*num);
    *numkeys = num;

    /* Add all key positions for argv[3...n] to keys[] */
    for (i = 0; i < num; i++) keys[i] = 3+i;

    return keys;
}

/* Helper function to extract keys from the SORT command.
 *
 * SORT <sort-key> ... STORE <store-key> ...
 *
 * The first argument of SORT is always a key, however a list of options
 * follow in SQL-alike style. Here we parse just the minimum in order to
 * correctly identify keys in the "STORE" option. */
int *sortGetKeys(struct redisCommand *cmd, robj **argv, int argc, int *numkeys) {
    int i, j, num, *keys, found_store = 0;
    REDIS_NOTUSED(cmd);

    num = 0;
    keys = zmalloc(sizeof(int)*2); /* Alloc 2 places for the worst case. */

    keys[num++] = 1; /* <sort-key> is always present. */

    /* Search for STORE option. By default we consider options to don't
     * have arguments, so if we find an unknown option name we scan the
     * next. However there are options with 1 or 2 arguments, so we
     * provide a list here in order to skip the right number of args. */
    struct {
        char *name;
        int skip;
    } skiplist[] = {
        {"limit", 2},
        {"get", 1},
        {"by", 1},
        {NULL, 0} /* End of elements. */
    };

    for (i = 2; i < argc; i++) {
        for (j = 0; skiplist[j].name != NULL; j++) {
            if (!strcasecmp(argv[i]->ptr,skiplist[j].name)) {
                i += skiplist[j].skip;
                break;
            } else if (!strcasecmp(argv[i]->ptr,"store") && i+1 < argc) {
                /* Note: we don't increment "num" here and continue the loop
                 * to be sure to process the *last* "STORE" option if multiple
                 * ones are provided. This is same behavior as SORT. */
                found_store = 1;
                keys[num] = i+1; /* <store-key> */
                break;
            }
        }
    }
    *numkeys = num + found_store;
    return keys;
}
