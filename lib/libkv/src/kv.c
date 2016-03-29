#include "libkv.h"
#include "kv_inner.h"
#include <sys/wait.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <float.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <locale.h>
 


/* 
 *
 * Every entry is composed of the following fields:
 *
 * name: a string representing the command name.
 * function: pointer to the C function implementing the command.
 * arity: number of arguments, it is possible to use -N to say >= N
 * sflags: command flags as string. See below for a table of flags.
 * flags: flags as bitmask. Computed by Redis using the 'sflags' field.
 * get_keys_proc: an optional function to get key arguments from a command.
 *                This is only used when the following three fields are not
 *                enough to specify what arguments are keys.
 * first_key_index: first argument that is a key
 * last_key_index: last argument that is a key
 * key_step: step to get all the keys from first to last argument. For instance
 *           in MSET the step is two since arguments are key,val,key,val,...
 * microseconds: microseconds of total execution time for this command.
 * calls: total number of calls of this command.
 *
 * The flags, microseconds and calls fields are computed by Redis and should
 * always be set to zero.
 *
 * Command flags are expressed using strings where every character represents
 * a flag. Later the populateCommandTable() function will take care of
 * populating the real 'flags' field using this characters.
 *
 * This is the meaning of the flags:
 *
 * w: write command (may modify the key space).
 * r: read command  (will never modify the key space).
 * m: may increase memory usage once called. Don't allow if out of memory. 
 * a: admin command, like SAVE or SHUTDOWN.
 * p: Pub/Sub related command.
 * f: force replication of this command, regardless of server.dirty.
 * s: command not allowed in scripts.
 * R: random command. Command is not deterministic, that is, the same command
 *    with the same arguments, with the same key space, may have different
 *    results. For instance SPOP and RANDOMKEY are two random commands.
 * S: Sort command output array if called from script, so that the output
 *    is deterministic.
 * l: Allow command while loading the database.
 * t: Allow command while a slave has stale data but is not allowed to
 *    server this data. Normally no command is accepted in this condition
 *    but just a few.
 * M: Do not automatically propagate the command on MONITOR.
 * k: Perform an implicit ASKING for this command, so the command will be
 *    accepted in cluster mode if the slot is marked as 'importing'.
 * F: Fast command: O(1) or O(log(N)) command that should never delay
 *    its execution as long as the kernel scheduler is giving us time.
 *    Note that commands that may trigger a DEL as a side effect (like SET)
 *    are not fast commands.
 */
struct redisCommand redisCommandTable[] = {
        {"set",setCommand,3,"wm",0,NULL,1,1,1,0,0},
        {"get",getCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"del",delCommand,-2,"w",0,NULL,1,-1,1,0,0},
        {"incr",incrCommand,2,"wmF",0,NULL,1,1,1,0,0},
        {"incrby",incrbyCommand,3,"wmF",0,NULL,1,1,1,0,0},
        {"decr",decrCommand,2,"wmF",0,NULL,1,1,1,0,0},
        {"decrby",decrbyCommand,3,"wmF",0,NULL,1,1,1,0,0},
        {"dbsize",dbsizeCommand,1,"rF",0,NULL,0,0,0,0,0},
        {"flushdb",flushdbCommand,1,"w",0,NULL,0,0,0,0,0},
        {"flushall",flushallCommand,1,"w",0,NULL,0,0,0,0,0},
        {"lpush",lpushCommand,-3,"wmF",0,NULL,1,1,1,0,0},
        {"lrange",lrangeCommand,4,"r",0,NULL,1,1,1,0,0},
        {"exists",existsCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"sadd",saddCommand,-3,"wmF",0,NULL,1,1,1,0,0},
       	{"srem",sremCommand,-3,"wF",0,NULL,1,1,1,0,0},
	{"smembers",smembersCommand,2,"rS",0,NULL,1,1,1,0,0},
        {"expire",expireCommand,3,"wF",0,NULL,1,1,1,0,0},
        {"expireat",expireatCommand,3,"wF",0,NULL,1,1,1,0,0},
        {"pexpire",pexpireCommand,3,"wF",0,NULL,1,1,1,0,0},
       	{"pexpireat",pexpireatCommand,3,"wF",0,NULL,1,1,1,0,0},
	{"echo",echoCommand,2,"rF",0,NULL,1,1,1,0,0},
	{"llen",llenCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"type",typeCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"select",selectCommand,2,"rlF",0,NULL,0,0,0,0,0},
        {"mset",msetCommand,-3,"wm",0,NULL,1,-1,2,0,0},
	{"rpush",rpushCommand,-3,"wmF",0,NULL,1,1,1,0,0},
	{"lpop",lpopCommand,2,"wF",0,NULL,1,1,1,0,0},
	{"rpop",rpopCommand,2,"wF",0,NULL,1,1,1,0,0},
//        {"zadd",zaddCommand,-4,"wmF",0,NULL,1,1,1,0,0},
//        {"zrange",zrangeCommand,-4,"r",0,NULL,1,1,1,0,0},
        {"hset",hsetCommand,4,"wmF",0,NULL,1,1,1,0,0},
        {"hget",hgetCommand,3,"rF",0,NULL,1,1,1,0,0},
	{"hmset",hmsetCommand,-4,"wm",0,NULL,1,1,1,0,0},
	{"hmget",hmgetCommand,-3,"r",0,NULL,1,1,1,0,0},
	{"hgetall",hgetallCommand,2,"r",0,NULL,1,1,1,0,0},
	{"hdel",hdelCommand,-3,"wF",0,NULL,1,1,1,0,0},
        {"scard",scardCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"sismember",sismemberCommand,3,"rF",0,NULL,1,1,1,0,0},
        {"srandmember",srandmemberCommand,-2,"rR",0,NULL,1,1,1,0,0}
//        {"zcard",zcardCommand,2,"rF",0,NULL,1,1,1,0,0}
};

unsigned int dictSdsCaseHash(const void *key);
int dictSdsKeyCaseCompare(void *privdata, const void *key1, const void *key2);
void dictSdsDestructor(void *privdata, void *val);
unsigned int dictSdsHash(const void *key);
int dictSdsKeyCompare(void *privdata, const void *key1, const void *key2);
void dictSdsDestructor(void *privdata, void *val);
void dictRedisObjectDestructor(void *privdata, void *val);


dictType commandTableDictType = {
        dictSdsCaseHash,           /* hash function */
        NULL,                      /* key dup */
        NULL,                      /* val dup */
        dictSdsKeyCaseCompare,     /* key compare */
        dictSdsDestructor,         /* key destructor */
        NULL                       /* val destructor */
};

dictType dbDictType = {
        dictSdsHash,
        NULL,
        NULL,
        dictSdsKeyCompare,
        dictSdsDestructor,
        dictRedisObjectDestructor
};


static void populateCommandTable(struct kv_handler *handler)
{
        int j;
        int numcommands = sizeof(redisCommandTable)/sizeof(struct redisCommand);

        for (j = 0; j < numcommands; j++) {
                struct redisCommand *c = redisCommandTable+j;
                char *f = c->sflags;

                while(*f != '\0') {
                        switch(*f) {
                        case 'w': c->flags |= REDIS_CMD_WRITE; break;
                        case 'r': c->flags |= REDIS_CMD_READONLY; break;
                        case 'm': c->flags |= REDIS_CMD_DENYOOM; break;
                        case 'a': c->flags |= REDIS_CMD_ADMIN; break;
                        case 'p': c->flags |= REDIS_CMD_PUBSUB; break;
                        case 's': c->flags |= REDIS_CMD_NOSCRIPT; break;
                        case 'R': c->flags |= REDIS_CMD_RANDOM; break;
                        case 'S': c->flags |= REDIS_CMD_SORT_FOR_SCRIPT; break;
                        case 'l': c->flags |= REDIS_CMD_LOADING; break;
                        case 't': c->flags |= REDIS_CMD_STALE; break;
                        case 'M': c->flags |= REDIS_CMD_SKIP_MONITOR; break;
                        case 'k': c->flags |= REDIS_CMD_ASKING; break;
                        case 'F': c->flags |= REDIS_CMD_FAST; break;
                    
                        default:logicError("Unsupported command flag");
                        }
                        f++;
                }
                logicErrorExpr(dictAdd(handler->commands, sdsnew(c->name), c) == DICT_OK, "Create command table failed");
        }
}

void createSharedObjects(struct kv_handler *handler)
{
        int i;
    
        handler->shared.ok = createObject(REDIS_STRING,sdsnew("OK"));

        for (i = 0; i < REDIS_SHARED_INTEGERS; i++) {
                handler->shared.integers[i] = createObject(REDIS_STRING, (void*)(long)i);
                handler->shared.integers[i]->encoding = REDIS_ENCODING_INT;
        }
}


/* Global vars that are actually used as constants. The following double
 * values are used for double on-disk serialization, and are initialized
 * at runtime to avoid strange compiler optimizations. */

double R_Zero, R_PosInf, R_NegInf, R_Nan;

struct evictionPoolEntry *evictionPoolAlloc(void);

/* This is a hash table type that uses the SDS dynamic strings library as
 * keys and redis objects as values (objects can hold SDS strings,
 * lists, sets). */

void dictVanillaFree(void *privdata, void *val)
{
        REDIS_NOTUSED(privdata);
        REDIS_NOTUSED(val);
}

void dictListDestructor(void *privdata, void *val)
{
        REDIS_NOTUSED(privdata);
        REDIS_NOTUSED(val);
}

int dictSdsKeyCompare(void *privdata, const void *key1,
                      const void *key2)
{
        int l1,l2;
        DICT_NOTUSED(privdata);

        l1 = sdslen((sds)key1);
        l2 = sdslen((sds)key2);
        if (l1 != l2) return 0;
        return memcmp(key1, key2, l1) == 0;
}

/* A case insensitive version used for the command lookup table and other
 * places where case insensitive non binary-safe comparison is needed. */
int dictSdsKeyCaseCompare(void *privdata, const void *key1,
                          const void *key2)
{
        DICT_NOTUSED(privdata);

        return strcasecmp(key1, key2) == 0;
}

void dictRedisObjectDestructor(void *privdata, void *val)
{
        DICT_NOTUSED(privdata);

        if (val == NULL) return; /* Values of swapped out keys as set to NULL */
        decrRefCount(val);
}

void dictSdsDestructor(void *privdata, void *val)
{
        DICT_NOTUSED(privdata);
        
        sdsfree(val);
}

int dictObjKeyCompare(void *privdata, const void *key1,
                      const void *key2)
{
        REDIS_NOTUSED(privdata);
        REDIS_NOTUSED(key1);
        REDIS_NOTUSED(key2);
        return 0; 
}

unsigned int dictObjHash(const void *key)
{
        REDIS_NOTUSED(key);
        return 0;
}

unsigned int dictSdsHash(const void *key)
{
        return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

unsigned int dictSdsCaseHash(const void *key)
{
        return dictGenCaseHashFunction((unsigned char*)key, sdslen((char*)key));
}

int dictEncObjKeyCompare(void *privdata, const void *key1,
                         const void *key2)
{
        robj *o1 = (robj*) key1, *o2 = (robj*) key2;
        int cmp;

        if (o1->encoding == REDIS_ENCODING_INT &&
            o2->encoding == REDIS_ENCODING_INT)
                return o1->ptr == o2->ptr;

        o1 = getDecodedObject(o1);
        o2 = getDecodedObject(o2);
        cmp = dictSdsKeyCompare(privdata,o1->ptr,o2->ptr);
        decrRefCount(o1);
        decrRefCount(o2);
        return cmp;
}

unsigned int dictEncObjHash(const void *key)
{
    robj *o = (robj*) key;

    if (sdsEncodedObject(o)) {
        return dictGenHashFunction(o->ptr, sdslen((sds)o->ptr));
    } else {
        if (o->encoding == REDIS_ENCODING_INT) {
            char buf[32];
            int len;

            len = ll2string(buf,32,(long)o->ptr);
            return dictGenHashFunction((unsigned char*)buf, len);
        } else {
            unsigned int hash;

            o = getDecodedObject(o);
            hash = dictGenHashFunction(o->ptr, sdslen((sds)o->ptr));
            decrRefCount(o);
            return hash;
        }
    }
}


/* Sets type hash table */
dictType setDictType = {
    dictEncObjHash,            /* hash function */
    NULL,                      /* key dup */
    NULL,                      /* val dup */
    dictEncObjKeyCompare,      /* key compare */
    dictRedisObjectDestructor, /* key destructor */
    NULL                       /* val destructor */
};

/* Sorted sets hash (note: a skiplist is used in addition to the hash table) */
dictType zsetDictType = {
    dictEncObjHash,            /* hash function */
    NULL,                      /* key dup */
    NULL,                      /* val dup */
    dictEncObjKeyCompare,      /* key compare */
    dictRedisObjectDestructor, /* key destructor */
    NULL                       /* val destructor */
};


/* server.lua_scripts sha (as sds string) -> scripts (as robj) cache. */
dictType shaScriptObjectDictType = {
        dictSdsCaseHash,            /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCaseCompare,      /* key compare */
        dictSdsDestructor,          /* key destructor */
        dictRedisObjectDestructor   /* val destructor */
};

/* Db->expires */
dictType keyptrDictType = {
        dictSdsHash,               /* hash function */
        NULL,                      /* key dup */
        NULL,                      /* val dup */
        dictSdsKeyCompare,         /* key compare */
        NULL,                      /* key destructor */
        NULL                       /* val destructor */
};



/* Hash type hash table (note that small hashes are represented with ziplists) */
dictType hashDictType = {
        dictEncObjHash,             /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictEncObjKeyCompare,       /* key compare */
        dictRedisObjectDestructor,  /* key destructor */
        dictRedisObjectDestructor   /* val destructor */
};

/* Keylist hash table type has unencoded redis objects as keys and
 * lists as values. It's used for blocking operations (BLPOP) and to
 * map swapped keys to a list of clients waiting for this keys to be loaded. */
dictType keylistDictType = {
        dictObjHash,                /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictObjKeyCompare,          /* key compare */
        dictRedisObjectDestructor,  /* key destructor */
        dictListDestructor          /* val destructor */
};

/* Cluster nodes hash table, mapping nodes addresses 1.2.3.4:6379 to
 * clusterNode structures. */
dictType clusterNodesDictType = {
        dictSdsHash,                /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCompare,          /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

/* Cluster re-addition blacklist. This maps node IDs to the time
 * we can re-add this node. The goal is to avoid readding a removed
 * node for some time. */
dictType clusterNodesBlackListDictType = {
        dictSdsCaseHash,            /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCaseCompare,      /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

/* Migrate cache dict type. */
dictType migrateCacheDictType = {
        dictSdsHash,                /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCompare,          /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

/* Replication cached script dict (server.repl_scriptcache_dict).
 * Keys are sds SHA1 strings, while values are not used at all in the current
 * implementation. */
dictType replScriptCacheDictType = {
        dictSdsCaseHash,            /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCaseCompare,      /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

int htNeedsResize(dict *dict)
{
        REDIS_NOTUSED(dict);
        return 0;
}

/* If the percentage of used slots in the HT reaches REDIS_HT_MINFILL
 * we resize the hash table to save memory */
void tryResizeHashTables(int dbid)
{
        REDIS_NOTUSED(dbid);
}

/* Our hash table implementation performs rehashing incrementally while
 * we write/read from the hash table. Still if the server is idle, the hash
 * table will use two tables for a long time. So we try to use 1 millisecond
 * of CPU time at every call of this function to perform some rehahsing.
 *
 * The function returns 1 if some rehashing was performed, otherwise 0
 * is returned. */
int incrementallyRehash(int dbid)
{
        REDIS_NOTUSED(dbid);
        return 0;
}

/* This function is called once a background process of some kind terminates,
 * as we want to avoid resizing the hash tables when there is a child in order
 * to play well with copy-on-write (otherwise when a resize happens lots of
 * memory pages are copied). The goal of this function is to update the ability
 * for dict.c to resize the hash tables accordingly to the fact we have o not
 * running childs. */
void updateDictResizePolicy(void) {
}


int activeExpireCycleTryExpire(redisDb *db, dictEntry *de, long long now)
{
        REDIS_NOTUSED(db);
        REDIS_NOTUSED(de);
        REDIS_NOTUSED(now);
        return REDIS_ERR;
}


void activeExpireCycle(int type)
{
        REDIS_NOTUSED(type);
}

unsigned int getLRUClock(void)
{
        return 0;
}

/* Add a sample to the operations per second array of samples. */
void trackInstantaneousMetric(int metric, long long current_reading)
{
        REDIS_NOTUSED(metric);
        REDIS_NOTUSED(current_reading);
}

/* Return the mean of all the samples. */
long long getInstantaneousMetric(int metric)
{
        REDIS_NOTUSED(metric);
        return 0;
}

/* Check for timeouts. Returns non-zero if the client was terminated */

/* This function handles 'background' operations we are required to do
 * incrementally in Redis databases, such as active key expiring, resizing,
 * rehashing. */
void databasesCron(void) {

}

/* We take a cached value of the unix time in the global state because with
 * virtual memory and aging there is to store the current time in objects at
 * every object access, and accuracy is not needed. To access a global var is
 * a lot faster than calling time(NULL) */
void updateCachedTime(void) {
}

void resetCommandTableStats(void) {

}


void redisOpArrayInit(redisOpArray *oa)
{
        REDIS_NOTUSED(oa);
}

int redisOpArrayAppend(redisOpArray *oa, struct redisCommand *cmd, int dbid,
                       robj **argv, int argc, int target)
{
        REDIS_NOTUSED(oa);
        REDIS_NOTUSED(cmd);
        REDIS_NOTUSED(dbid);
        REDIS_NOTUSED(argv);
        REDIS_NOTUSED(argc);
        REDIS_NOTUSED(target);

        return 0;
}

void redisOpArrayFree(redisOpArray *oa)
{
        REDIS_NOTUSED(oa);
}

/* Create a new eviction pool. */
struct evictionPoolEntry *evictionPoolAlloc(void)
{
        return NULL;
}

#define EVICTION_SAMPLES_ARRAY_SIZE 16
void evictionPoolPopulate(dict *sampledict, dict *keydict, struct evictionPoolEntry *pool)
{
        REDIS_NOTUSED(sampledict);
        REDIS_NOTUSED(keydict);
        REDIS_NOTUSED(pool);
}

long long ustime()
{
        struct timeval cur;
        gettimeofday(&cur, NULL);
        return cur.tv_sec*1000000 + cur.tv_usec;
}

long long mstime()
{
        return ustime()/1000;
}

struct redisCommand *lookupCommand(struct kv_handler *handler, sds name)
{
        return dictFetchValue(handler->commands, name);
}


/**
 * Create and initialize default values.
 */
static struct kv_handler* _kv_handler_create_default(const char *param1, va_list ap)
{
        int i;
        struct kv_handler *handler;

        handler = zcalloc(sizeof(*handler));
        if (handler == NULL) return NULL;

        /** analysis config list */
        REDIS_NOTUSED(param1);
        REDIS_NOTUSED(ap);
        /** end */
        
        handler->state = STATE_LOADING;
        handler_mutex_lock_init(handler);
        handler->maxmemory = 0;
        handler->dbindex = 0;
        handler->dbnum = KV_DEFAULT_DBNUM;
        handler->commands = dictCreate(&commandTableDictType, NULL);
        populateCommandTable(handler);
        createSharedObjects(handler);
        
        handler->db = zmalloc(sizeof(redisDb)*handler->dbnum);
        for (i = 0; i < handler->dbnum; i++) {
                handler->db[i].dict = dictCreate(&dbDictType, NULL);
                handler->db[i].expires = dictCreate(&keyptrDictType, NULL);
                handler->db[i].expire_manager = kv_expire_manager_create();
                handler->db[i].id = i;
        }

        handler->list_max_ziplist_entries = REDIS_LIST_MAX_ZIPLIST_ENTRIES;
        handler->list_max_ziplist_value = REDIS_LIST_MAX_ZIPLIST_VALUE;
        handler->set_max_intset_entries = REDIS_SET_MAX_INTSET_ENTRIES;
        handler->zset_max_ziplist_entries = REDIS_ZSET_MAX_ZIPLIST_ENTRIES;
        handler->zset_max_ziplist_value = REDIS_ZSET_MAX_ZIPLIST_VALUE;
        handler->hash_max_ziplist_entries = REDIS_HASH_MAX_ZIPLIST_ENTRIES;
        handler->hash_max_ziplist_value = REDIS_HASH_MAX_ZIPLIST_VALUE;

        handler->stat_keyspace_misses = 0;
        handler->stat_keyspace_hits = 0;
        handler->stat_expiredkeys = 0;
        handler->dirty = 0;

        handler->state = STATE_IDLE;

        return handler;
}

/**
 * Create a new handler.
 */
struct kv_handler* kv_handler_create(const char *param1, va_list ap)
{
        return _kv_handler_create_default(param1, ap);
}


void kv_handler_release(struct kv_handler *handler)
{
        int i;
        
        if (handler->state & STATE_PENDING)
                return; // init error

        LOOP_WAIT((handler->state & STATE_LOADING) ||
                  (handler->state & STATE_RUNNING));

        handler_mutex_lock(handler); /** lock for release handler */
        
        dictRelease(handler->commands);
        for (i = 0; i < handler->dbnum; i++) {
                dictRelease(handler->db[i].dict);
                dictRelease(handler->db[i].expires);
                kv_expire_manager_destroy(handler->db[i].expire_manager);
        }
        zfree(handler->db);
        if (handler->shared.ok->refcount > 1)
                handler->shared.ok->refcount = 1;
        decrRefCount(handler->shared.ok);
        for (i = 0; i < REDIS_SHARED_INTEGERS; i++) {
                if (handler->shared.integers[i]->refcount > 1)
                        handler->shared.integers[i]->refcount = 1;
                decrRefCount(handler->shared.integers[i]);    
        }

        handler_mutex_unlock(handler);
        zfree(handler);
}

/**
 * Create new kv_answer_t structure
 * @return A new malloc initailized kv_answer_t returned, otherwise NULL. 
 */
kv_answer_t* kv_answer_create(struct kv_handler *handler)
{
        kv_answer_t* r = zmalloc(sizeof(*r));
        if (r == NULL) return r;

        if (handler->state & STATE_IDLE) {
                r->errnum = ERR_NONE;
                r->err = get_err(r->errnum);
        } else if (handler->state & STATE_PENDING) {
                r->errnum = ERR_NOT_INIT;
                r->err = get_err(r->errnum);
        } else {
                r->errnum = ERR_BUSY;
                r->err = get_err(r->errnum);
        }
        
        r->count = 0;
        r->head = r->tail = NULL;

        return r;
}

void kv_answer_release(kv_answer_t *a)
{
        kv_answer_value_t *cur, *del;
        if (a == NULL) return;
        cur = a->head;
        while(cur) {
                del = cur;
                cur = cur->next;
                zfree(del->ptr);
                zfree(del);
        }
    
        zfree(a);
}

unsigned long kv_answer_length(kv_answer_t *a)
{
        return a == NULL ? 0 : a->count;
}

/**
 * Malloc a new value entry and initialized by ptr, ptrlen.
 * @param ptr Value entry holds content.
 * @param ptrlen ptr length.
 * @return A new malloc value entry.
 */
kv_answer_value_t *kv_answer_value_create(const void *ptr, unsigned long ptrlen)
{
        kv_answer_value_t* value;
        if (!ptr || ptrlen == 0) return NULL;

        value = zmalloc(sizeof(kv_answer_value_t));
        value->ptr = (char*)ptr;
        value->ptrlen = ptrlen;
        value->prev = value->next = NULL;

        return value;
}

/**
 * Add a new value entry to the tail of kv_answer_t list
 * @param a kv_answer_t container
 * @param ptr The added content pointer.
 * @param ptrlen The length of pointer to the content.
 */
void kv_answer_add_value_tail(kv_answer_t *a, const void *ptr, unsigned long ptrlen)
{
        kv_answer_value_t *value;
        if (!a || !ptr || ptrlen == 0) return;

        value = kv_answer_value_create(ptr, ptrlen);
        if (a->head == NULL && a->tail == NULL) {
                a->head = a->tail = value;
        } else {
                a->tail->next = value;
                value->prev = a->tail;
                a->tail = value;
        }
        a->count++;
}

/**
 * Add a new value entry to the head of kv_answer_t list
 * @param a kv_answer_t container
 * @param ptr The added content pointer.
 * @param ptrlen The length of pointer to the content.
 */
void kv_answer_add_value_head(kv_answer_t *a, const void *ptr, unsigned long ptrlen)
{
        kv_answer_value_t *value;
        if (!a || !ptr || ptrlen == 0) return;

        value = kv_answer_value_create(ptr, ptrlen);
        if (a->head == NULL && a->tail == NULL) {
                a->head = a->tail = value;
        } else {
                a->head->prev = value;
                value->next = a->head;
                a->head = value;
        }
        a->count++;
}

kv_answer_iter_t *kv_answer_get_iter(kv_answer_t *a, int direction)
{
        if (!a) return NULL;
    
        kv_answer_iter_t *iter = zmalloc(sizeof(kv_answer_iter_t));
        iter->direction = direction;
        if (iter->direction == ANSWER_HEAD) {
                iter->next = a->head;
        } else {
                iter->next = a->tail;
        }
        return iter;
}

void kv_answer_rewind_iter(kv_answer_t *a, kv_answer_iter_t *iter)
{
        if (iter->direction == ANSWER_HEAD) {
                iter->next = a->head;
        } else {
                iter->next = a->tail;
        }
}

void kv_answer_release_iter(kv_answer_iter_t *iter)
{
        if (iter)
                zfree(iter);
}

kv_answer_value_t *kv_answer_next(kv_answer_iter_t *iter)
{
        kv_answer_value_t *cur = iter->next;

        if (!cur) return NULL;
        if (iter->direction == ANSWER_HEAD) {
                iter->next = cur->next;
        } else {
                iter->next = cur->prev;
        }

        return cur;
}

kv_answer_value_t *kv_answer_first_value(kv_answer_t *ans)
{
        return ans == NULL ? NULL : ans->head;
}

kv_answer_value_t *kv_answer_last_value(kv_answer_t *ans)
{
        return ans == NULL ? NULL : ans->tail;
}

char* kv_answer_value_to_string(kv_answer_value_t *value)
{
        if (value == NULL || value->ptr == NULL) return NULL;

        value->ptr = zrealloc(value->ptr, value->ptrlen + 1);
        ((char*)value->ptr)[value->ptrlen] = '\0';
        value->ptrlen += 1;
        return (char*)value->ptr;
}




/**
 * create reply instance for every call.
 */
reply_t* reply_create()
{
        reply_t* c = zmalloc(sizeof(*c));
        if (!c) return NULL;

        c->argc = 0;
        c->argv = NULL;
        c->cmd = NULL;
        c->proto_text = sdsempty();
        c->errnum = ERR_NONE;
        c->err = get_err(c->errnum);
        c->result = NULL;
        return c;
}

void reply_free(reply_t* c)
{
        int i;

        for (i = 0; i < c->argc; i++) {
                decrRefCount(c->argv[i]);
        }
        zfree(c->argv);
        sdsfree(c->proto_text);
        reply_free_result(c);
        zfree(c);
}

void reply_free_result(reply_t* c)
{
        if (c->result != NULL)
                listRelease(c->result);
}

void reply_set_err(reply_t* c, int errnum)
{
        c->errnum = errnum;
        c->err = get_err(errnum);
}

void reply_add_result(reply_t *c, robj *o)
{
        if (c == NULL || o == NULL) return;
        if (c->result == NULL) {
                c->result = listCreate();
                listSetFreeMethod(c->result, decrRefCountVoid); 
        }
        listAddNodeTail(c->result, o);
}

void reply_add_result_long(reply_t *c, long value)
{
        robj *result;
        result = createObject(REDIS_STRING, (void*)value);
        result->encoding = REDIS_ENCODING_INT;
        reply_add_result(c, result);
}

void reply_add_result_str(reply_t *c, const char* str)
{
        robj *result;

        if (!str || !str[0]) return;
        result = createStringObject((char*)str, strlen(str));
        reply_add_result(c, result);
}

int reply_start(struct kv_handler *handler)
{
        reply_t *c = handler->reply;

        if (processProto(c) == KV_ERR) return KV_ERR;
        c->cmd = lookupCommand(handler, c->argv[0]->ptr);
        if (!c->cmd) {
                reply_set_err(c, ERR_CMD);
                return KV_ERR;
        }

        if ((c->cmd->arity > 0 && c->cmd->arity != c->argc) ||
            (c->argc < -c->cmd->arity)) {
                reply_set_err(c, ERR_ARGUMENTS);
                return KV_ERR;
        }

        c->cmd->proc(handler);
        return ERR_NONE;
        
}

/* convert cmd words to protocol */
static int cmd_to_proto(const char* cmd, size_t cmdlen, reply_t* c)
{
        list* cmd_words = split_word(cmd, cmdlen, ' ');
        listIter* iter;
        listNode* node;
        char buf[64];

        if (!cmd_words) return KV_ERR;
        if (cmd_words->len == 0) {
                listRelease(cmd_words);
                return KV_ERR;
        }
        
        sdsclear(c->proto_text);
        snprintf(buf, sizeof(buf), "*%ld\r\n", cmd_words->len);

        c->proto_text = sdscat(c->proto_text, buf);
        
        iter = listGetIterator(cmd_words, AL_START_HEAD);
        while((node = listNext(iter))) {
                snprintf(buf, sizeof(buf), "$%ld\r\n", sdslen(node->value));
                c->proto_text = sdscat(c->proto_text, buf);
                c->proto_text = sdscat(c->proto_text, node->value);
                c->proto_text = sdscat(c->proto_text, "\r\n");
        }

        listReleaseIterator(iter);
        listRelease(cmd_words);

        return ERR_NONE;
}

static void stringTypeValue(robj *o, kv_answer_t *a)
{
        int bytes;
        char *lbuf;

        logicErrorExpr(o->type == REDIS_STRING, "Never happend!");

        switch(o->encoding) {
        case REDIS_ENCODING_RAW:
        case REDIS_ENCODING_EMBSTR:
                bytes = sdslen(o->ptr);
                lbuf = zmalloc(bytes);
                memcpy(lbuf, o->ptr, bytes);
                kv_answer_add_value_tail(a, lbuf, bytes);
                break;
        case REDIS_ENCODING_INT:
                lbuf = zmalloc(128);
                logicErrorExpr((bytes = ll2string(lbuf, 128, (long long)o->ptr)) != 0, "Convert string to ll error!");
                kv_answer_add_value_tail(a, lbuf, bytes);
                break;
        default:
                printf("=== forget to handle with type belong to StringType\n");
        }
}

#if 0
static void listTypeValue(robj *o, kv_answer_t *a)
{
        logicErrorExpr(o->type == REDIS_LIST, "Never happend!");
        switch(o->encoding) {
        case REDIS_ENCODING_ZIPLIST:
        case REDIS_ENCODING_LINKEDLIST:
                break;
        default:
                printf("=== to handle with type belong to ListType\n");
        }
        
}

static void setTypeValue(robj *o, kv_answer_t *a)
{
        logicError(o->type == REDIS_SET);
        switch(o->encoding) {

        default:
                printf("=== to handle with type belong to SetType\n");
        }
}

static void zsetTypeValue(robj *o, kv_answer_t *a)
{
        logicErrorExpr(o->type == REDIS_ZSET, "Never happend!");

        switch(o->encoding) {
        default:
                printf("=== forget to handle with type belong to ZsetType\n");

        }
}

static void hashTypeValue(robj *o, kv_answer_t *a)
{
        logicErrorExpr(o->type == REDIS_HASH, "Never happend!");
        switch(o->encoding) {
        default:
                printf("=== to handle with type belong to HashType\n");
        }
}
#endif

void reply_to_answer(reply_t* c, kv_answer_t* ans)
{
        list *l;
        listIter *iter;
        listNode *node;
        robj *tmp;

        if (!c || !ans) return;
        ans->errnum = c->errnum;
        ans->err = c->err;
        if (!c->result) return;
        l = c->result;
        iter = listGetIterator(l, 0/*head*/);

        while((node = listNext(iter)) != NULL) {
                tmp = node->value;
                switch(tmp->type) {
                case REDIS_STRING:
                        stringTypeValue(tmp,ans);break;
                case REDIS_LIST:
//			listTypeValue(tmp,ans);break;
                case REDIS_SET:
//                        setTypeValue(tmp,ans);break;
                case REDIS_ZSET:
//                        zsetTypeValue(tmp,ans);break;
                case REDIS_HASH:
//                       	hashTypeValue(tmp,ans);break;
                        logicErrorExpr(0, "Never happend!");
                default:
                        logicError("Unknown object type!");
                }
        }
        listReleaseIterator(iter);
}

int processProto(reply_t *c)
{
        char* newline = NULL;
        int pos=0, ok;
        long long ll;
        long long linecount = 0;
        int bulklen = -1;

        /* get line count*/
        newline = strchr(c->proto_text, '\r');
        if (newline == NULL) {
                reply_set_err(c, ERR_PROTOCOL);
                return KV_ERR;
        }

        /* Buffer should also contain \n */
        if (newline-(c->proto_text) > ((signed)sdslen(c->proto_text)-2)) {
                reply_set_err(c, ERR_PROTOCOL);
                return KV_ERR;
        }

        ok = string2ll(c->proto_text+1, newline-(c->proto_text+1),&ll);
        if (!ok || ll > 1024*1024) {
                reply_set_err(c, ERR_PROTOCOL);
                return KV_ERR;
        }

        pos = (newline-c->proto_text)+2;
        if (ll <= 0) { // not exists return ?
                sdsrange(c->proto_text, pos, -1);
                return ERR_NONE;
        }
        linecount = ll;
        if (c->argv) zfree(c->argv);
        c->argv = zmalloc(sizeof(robj*)*linecount);

        while(linecount) {
                if (bulklen == -1) {
                        newline = strchr(c->proto_text+pos, '\r');
                        if (newline == NULL) {
                                reply_set_err(c, ERR_PROTOCOL);
                                return KV_ERR;
                        }
      
                        if (newline-(c->proto_text) > ((signed)sdslen(c->proto_text)-2))
                                break;

                        if (c->proto_text[pos] != '$') {
                                reply_set_err(c, ERR_PROTOCOL);
                                return KV_ERR;
                        }

                        ok = string2ll(c->proto_text+pos+1,newline-(c->proto_text+pos+1),&ll);
                        if (!ok || ll < 0 || ll > 512*1024*1024) {
                                reply_set_err(c, ERR_PROTOCOL);
                                return REDIS_ERR;
                        }

                        pos += newline-(c->proto_text+pos)+2;
                        if (ll >= REDIS_MBULK_BIG_ARG) {
                                size_t qblen;
                                sdsrange(c->proto_text,pos,-1);
                                pos = 0;
                                qblen = sdslen(c->proto_text);

                                if (qblen < (size_t)ll+2)
                                        c->proto_text = sdsMakeRoomFor(c->proto_text,ll+2-qblen);
                        }
                        bulklen = ll;
                }

                if (sdslen(c->proto_text)-pos < (unsigned)(bulklen+2)) {
                        break;
                } else {
                        if (pos == 0 &&
                            bulklen >= REDIS_MBULK_BIG_ARG &&
                            (signed) sdslen(c->proto_text) == bulklen+2)
                        {
                                c->argv[c->argc++] = createObject(REDIS_STRING,c->proto_text);
                                sdsIncrLen(c->proto_text,-2); /* remove CRLF */
                                c->proto_text = sdsempty();
                                c->proto_text = sdsMakeRoomFor(c->proto_text,bulklen+2);
                                pos = 0;
                        } else {
                                c->argv[c->argc++] =
                                        createStringObject(c->proto_text+pos,bulklen);
                                pos += bulklen+2;
                        }
                        bulklen = -1;
                        linecount--;
                }
        }

        if (pos) sdsrange(c->proto_text,pos,-1);
        if (linecount == 0) return ERR_NONE;

        reply_set_err(c, ERR_PROTOCOL);
        return KV_ERR;
}

kv_answer_t* kv_executor(struct kv_handler *handler, const char* cmd, unsigned int cmdlen)
{
        kv_answer_t* ans = NULL;

        handler_mutex_lock(handler); /** start reply mutex lock */

        ans = kv_answer_create(handler);
        handler->reply = reply_create();
        selectDb(handler, handler->dbindex);

        if (cmd_to_proto(cmd, cmdlen, handler->reply) == KV_ERR) {
                ans->errnum = ERR_ARGUMENTS;
                ans->err = get_err(ERR_ARGUMENTS);
                reply_free(handler->reply);
                return ans;
        }
 
        reply_start(handler);
        reply_to_answer(handler->reply, ans);
        reply_free(handler->reply);

        handler_mutex_unlock(handler);
        return ans;
}

/**
 * Config list support:
 *           maxmemory
 *           locale
 *           persistence
 *           oom callback
 */
struct kv_handler* kv_create(const char* param1, ...)
{
        va_list ap;
        struct kv_handler *handler;
        
        va_start(ap, param1);
        handler = kv_handler_create(param1, ap);
        va_end(ap);

        return handler;
}

void kv_destroy(struct kv_handler *handler)
{
        kv_handler_release(handler);
}

kv_answer_t* kv_ask(struct kv_handler *handler, const char *cmd, unsigned int cmdlen)
{
        if (!(handler->state & STATE_IDLE)) {
                return kv_answer_create(handler);
        } else if (!cmd || cmdlen == 0) { /* invalid parameter passed */
                kv_answer_t *a = kv_answer_create(handler);
                a->errnum = ERR_ARGUMENTS;
                a->err = get_err(ERR_ARGUMENTS);
                return a;
        }
        return kv_executor(handler, cmd, cmdlen);
}

unsigned int kv_get_used_memory()
{
        return zmalloc_used_memory();
}


