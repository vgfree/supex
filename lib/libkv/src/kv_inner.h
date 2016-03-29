#ifndef _KV_H_
#define _KV_H_




#ifndef KV_OK
#define KV_ERR -1
#endif

#include "fmacros.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>

#include "sds.h"     /* Dynamic safe strings */
#include "dict.h"    /* Hash tables */
#include "adlist.h"  /* Linked lists */
#include "zmalloc.h" /* total memory usage aware version of malloc/free */
#include "ziplist.h" /* Compact list data structure */
#include "intset.h"  /* Compact integer set structure */
#include "util.h"    /* Misc functions useful in many places */
#include "kv_rbtree.h"


typedef long long mstime_t; /* millisecond time type. */

typedef pthread_mutex_t mutex_lock_t;
typedef pthread_spinlock_t fast_lock_t;

#define LOOP_WAIT(condition) while((condition)) 


#define REDIS_LRU_BITS 24
#define REDIS_LRU_CLOCK_MAX ((1<<REDIS_LRU_BITS)-1) /* Max value of obj->lru */
#define REDIS_LRU_CLOCK_RESOLUTION 1000 /* LRU clock resolution in ms */
typedef struct redisObject {
        unsigned type:4;
        unsigned encoding:4;
        unsigned lru:REDIS_LRU_BITS; /* lru time (relative to server.lruclock) */
        int refcount;
        void *ptr;
} robj;


/* Error codes */
#define REDIS_OK                0
#define REDIS_ERR               -1



#define REDIS_SHARED_SELECT_CMDS 10
#define REDIS_SHARED_INTEGERS 10000

#define REDIS_SHARED_BULKHDR_LEN 32
#define REDIS_DEFAULT_MAXMEMORY 0
#define REDIS_DEFAULT_MAXMEMORY_SAMPLES 5
#define REDIS_DEFAULT_ACTIVE_REHASHING 1
#define KV_DEFAULT_TIMER_INTERVAL 1000 /** microseconds */


/* Instantaneous metrics tracking. */
#define REDIS_METRIC_SAMPLES 16     /* Number of samples per metric. */
#define REDIS_METRIC_COMMAND 0      /* Number of commands executed. */
#define REDIS_METRIC_NET_INPUT 1    /* Bytes read to network .*/
#define REDIS_METRIC_NET_OUTPUT 2   /* Bytes written to network. */
#define REDIS_METRIC_COUNT 3

/* Protocol and I/O related defines */
#define REDIS_MAX_QUERYBUF_LEN  (1024*1024*1024) /* 1GB max query buffer. */
#define REDIS_IOBUF_LEN         (1024*16)  /* Generic I/O buffer size */
#define REDIS_REPLY_CHUNK_BYTES (16*1024) /* 16k output buffer */
#define REDIS_INLINE_MAX_SIZE   (1024*64) /* Max size of inline reads */
#define REDIS_MBULK_BIG_ARG     (1024*32)
#define REDIS_LONGSTR_SIZE      21          /* Bytes needed for long -> str */

/* When configuring the Redis eventloop, we setup it so that the total number
 * of file descriptors we can handle are server.maxclients + RESERVED_FDS + FDSET_INCR
 * that is our safety margin. */
#define REDIS_EVENTLOOP_FDSET_INCR (REDIS_MIN_RESERVED_FDS+96)

/* Hash table parameters */
#define REDIS_HT_MINFILL        10      /* Minimal hash table fill 10% */

/* Command flags. Please check the command table defined in the redis.c file
 * for more information about the meaning of every flag. */
#define REDIS_CMD_WRITE 1                   /* "w" flag */
#define REDIS_CMD_READONLY 2                /* "r" flag */
#define REDIS_CMD_DENYOOM 4                 /* "m" flag */
#define REDIS_CMD_NOT_USED_1 8              /* no longer used flag */
#define REDIS_CMD_ADMIN 16                  /* "a" flag */
#define REDIS_CMD_PUBSUB 32                 /* "p" flag */
#define REDIS_CMD_NOSCRIPT  64              /* "s" flag */
#define REDIS_CMD_RANDOM 128                /* "R" flag */
#define REDIS_CMD_SORT_FOR_SCRIPT 256       /* "S" flag */
#define REDIS_CMD_LOADING 512               /* "l" flag */
#define REDIS_CMD_STALE 1024                /* "t" flag */
#define REDIS_CMD_SKIP_MONITOR 2048         /* "M" flag */
#define REDIS_CMD_ASKING 4096               /* "k" flag */
#define REDIS_CMD_FAST 8192                 /* "F" flag */

/* Object types */
#define REDIS_STRING 0
#define REDIS_LIST 1
#define REDIS_SET 2
#define REDIS_ZSET 3
#define REDIS_HASH 4

/* Objects encoding. Some kind of objects like Strings and Hashes can be
 * internally represented in multiple ways. The 'encoding' field of the object
 * is set to one of this fields for this object. */
#define REDIS_ENCODING_RAW 0     /* Raw representation */
#define REDIS_ENCODING_INT 1     /* Encoded as integer */
#define REDIS_ENCODING_HT 2      /* Encoded as hash table */
#define REDIS_ENCODING_ZIPMAP 3  /* Encoded as zipmap */
#define REDIS_ENCODING_LINKEDLIST 4 /* Encoded as regular linked list */
#define REDIS_ENCODING_ZIPLIST 5 /* Encoded as ziplist */
#define REDIS_ENCODING_INTSET 6  /* Encoded as intset */
#define REDIS_ENCODING_SKIPLIST 7  /* Encoded as skiplist */
#define REDIS_ENCODING_EMBSTR 8  /* Embedded sds string encoding */

#define REDIS_CLIENT_TYPE_COUNT 3

/* List related stuff */
#define REDIS_HEAD 0
#define REDIS_TAIL 1

/* Sort operations */
#define REDIS_SORT_GET 0
#define REDIS_SORT_ASC 1
#define REDIS_SORT_DESC 2
#define REDIS_SORTKEY_MAX 1024

/* Anti-warning macro... */
#define REDIS_NOTUSED(V) ((void) V)

#define ZSKIPLIST_MAXLEVEL 32 /* Should be enough for 2^32 elements */
#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */

/* Zip structure related defaults */
#define REDIS_HASH_MAX_ZIPLIST_ENTRIES 512
#define REDIS_HASH_MAX_ZIPLIST_VALUE 64
#define REDIS_LIST_MAX_ZIPLIST_ENTRIES 512
#define REDIS_LIST_MAX_ZIPLIST_VALUE 64
#define REDIS_SET_MAX_INTSET_ENTRIES 512
#define REDIS_ZSET_MAX_ZIPLIST_ENTRIES 128
#define REDIS_ZSET_MAX_ZIPLIST_VALUE 64

/* HyperLogLog defines */
#define REDIS_DEFAULT_HLL_SPARSE_MAX_BYTES 3000

/* Sets operations codes */
#define REDIS_OP_UNION 0
#define REDIS_OP_DIFF 1
#define REDIS_OP_INTER 2

/* Redis maxmemory strategies */
#define REDIS_MAXMEMORY_VOLATILE_LRU 0
#define REDIS_MAXMEMORY_VOLATILE_TTL 1
#define REDIS_MAXMEMORY_VOLATILE_RANDOM 2
#define REDIS_MAXMEMORY_ALLKEYS_LRU 3
#define REDIS_MAXMEMORY_ALLKEYS_RANDOM 4
#define REDIS_MAXMEMORY_NO_EVICTION 5
#define REDIS_DEFAULT_MAXMEMORY_POLICY REDIS_MAXMEMORY_NO_EVICTION

/* Units */
#define UNIT_SECONDS 0
#define UNIT_MILLISECONDS 1



void logicError(const char* fmt, ...);
void logicErrorExpr(int expr, const char* fmt, ...);


/* error code */
#define ERR_NONE           0
#define ERR_TYPE           1
#define ERR_NO_KEY         2
#define ERR_SYNTAX         3
#define ERR_SAME_OBJECT    4
#define ERR_OUT_OF_RANGE   5
#define ERR_NIL            6
#define ERR_NOT_INIT       7
#define ERR_BUSY           8
#define ERR_ARGUMENTS      9
#define ERR_PROTOCOL       10
#define ERR_VALUE          11
#define ERR_DB_INDEX       12
#define ERR_CMD            13

const char* get_err(int errnum);


/* Macro used to obtain the current LRU clock.
 * If the current resolution is lower than the frequency we refresh the
 * LRU clock (as it should be in production servers) we return the
 * precomputed value, otherwise we need to resort to a function call. */
#define LRU_CLOCK() ((1000/10 <= REDIS_LRU_CLOCK_RESOLUTION) ? /*server.lruclock*/getLRUClock() : getLRUClock())

/* Macro used to initialize a Redis object allocated on the stack.
 * Note that this macro is taken near the structure definition to make sure
 * we'll update it when the structure is changed, to avoid bugs like
 * bug #85 introduced exactly in this way. */
#define initStaticStringObject(_var,_ptr) do {          \
                _var.refcount = 1;                      \
                _var.type = REDIS_STRING;               \
                _var.encoding = REDIS_ENCODING_RAW;     \
                _var.ptr = _ptr;                        \
        } while(0);

/* To improve the quality of the LRU approximation we take a set of keys
 * that are good candidate for eviction across freeMemoryIfNeeded() calls.
 *
 * Entries inside the eviciton pool are taken ordered by idle time, putting
 * greater idle times to the right (ascending order).
 *
 * Empty entries have the key pointer set to NULL. */
#define REDIS_EVICTION_POOL_SIZE 16
struct evictionPoolEntry {
        unsigned long long idle;    /* Object idle time. */
        sds key;                    /* Key name. */
};


typedef struct kv_expire_manager {
        struct kv_rbtree* ex_tree;

}kv_expire_manager_t;


/* Redis database representation. There are multiple databases identified
 * by integers from 0 (the default database) up to the max configured
 * database. The database number is the 'id' field in the structure. */
typedef struct redisDb {
        dict *dict;                        /* The keyspace for this DB */
        dict *expires;                     /* Timeout of keys with a timeout set */
        kv_expire_manager_t *expire_manager; /** Manage All the keys that are setted by expire command */ 
        dict *blocking_keys;               /* Keys with clients waiting for data (BLPOP) */
        dict *ready_keys;                  /* Blocked keys that received a PUSH */
        dict *watched_keys;                /* WATCHED keys for MULTI/EXEC CAS */
        struct evictionPoolEntry *eviction_pool;    /* Eviction pool of keys */
        int id;                            /* Database ID */
        long long avg_ttl;                 /* Average TTL, just for stats */
} redisDb;

/* Client MULTI/EXEC state */
typedef struct multiCmd {
        robj **argv;
        int argc;
        struct redisCommand *cmd;
} multiCmd;

typedef struct multiState {
        multiCmd *commands;     /* Array of MULTI commands */
        int count;              /* Total number of MULTI commands */
        int minreplicas;        /* MINREPLICAS for synchronous replication */
        time_t minreplicas_timeout; /* MINREPLICAS timeout as unixtime. */
} multiState;

/* The following structure represents a node in the server.ready_keys list,
 * where we accumulate all the keys that had clients blocked with a blocking
 * operation such as B[LR]POP, but received new data in the context of the
 * last executed command.
 *
 * After the execution of every command or script, we run this list to check
 * if as a result we should serve data to clients blocked, unblocking them.
 * Note that server.ready_keys will not have duplicates as there dictionary
 * also called ready_keys in every structure representing a Redis database,
 * where we make sure to remember if a given key was already added in the
 * server.ready_keys list. */
typedef struct readyList {
        redisDb *db;
        robj *key;
} readyList;


struct saveparam {
        time_t seconds;
        int changes;
};

struct sharedObjectsStruct {
        robj *crlf, *err, *emptybulk, *czero, *cone, *cnegone, *pong, *space,
                *colon, *nullbulk, *nullmultibulk, *queued,
                *emptymultibulk, *nokeyerr, *syntaxerr, *sameobjecterr,
                *outofrangeerr, *noscripterr, *loadingerr, *slowscripterr, *bgsaveerr,
                *masterdownerr, *roslaveerr, *execaborterr, *noautherr, *noreplicaserr,
                *busykeyerr, *oomerr, *plus, *messagebulk, *pmessagebulk, *subscribebulk,
                *unsubscribebulk, *psubscribebulk, *punsubscribebulk, *del, *rpop, *lpop,
                *lpush, *emptyscan, *minstring, *maxstring,
                *select[REDIS_SHARED_SELECT_CMDS],
                *mbulkhdr[REDIS_SHARED_BULKHDR_LEN], /* "*<value>\r\n" */
                *bulkhdr[REDIS_SHARED_BULKHDR_LEN];  /* "$<value>\r\n" */

    
        robj *ok, *wrongtypeerr, *integers[REDIS_SHARED_INTEGERS];
};


/* ZSETs use a specialized version of Skiplists */
typedef struct zskiplistNode {
        robj *obj;
        double score;
        struct zskiplistNode *backward;
        struct zskiplistLevel {
                struct zskiplistNode *forward;
                unsigned int span;
        } level[];
} zskiplistNode;

typedef struct zskiplist {
        struct zskiplistNode *header, *tail;
        unsigned long length;
        int level;
} zskiplist;

typedef struct zset {
        dict *dict;
        zskiplist *zsl;
} zset;

typedef struct clientBufferLimitsConfig {
        unsigned long long hard_limit_bytes;
        unsigned long long soft_limit_bytes;
        time_t soft_limit_seconds;
} clientBufferLimitsConfig;

extern clientBufferLimitsConfig clientBufferLimitsDefaults[REDIS_CLIENT_TYPE_COUNT];

/* The redisOp structure defines a Redis Operation, that is an instance of
 * a command with an argument vector, database ID, propagation target
 * (REDIS_PROPAGATE_*), and command pointer.
 *
 * Currently only used to additionally propagate more commands to AOF/Replication
 * after the propagation of the executed command. */
typedef struct redisOp {
        robj **argv;
        int argc, dbid, target;
        struct redisCommand *cmd;
} redisOp;

/* Defines an array of Redis operations. There is an API to add to this
 * structure in a easy way.
 *
 * redisOpArrayInit();
 * redisOpArrayAppend();
 * redisOpArrayFree();
 */
typedef struct redisOpArray {
        redisOp *ops;
        int numops;
} redisOpArray;


#define handler_mutex_lock_init(h)    pthread_mutex_init(&h->reply_lock, NULL)
#define handler_mutex_lock_destroy(h) pthread_mutex_destroy(&h->reply_lock)
#define handler_mutex_lock(h)         pthread_mutex_lock(&h->reply_lock)
#define handler_mutex_unlock(h)       pthread_mutex_unlock(&h->reply_lock)
#define handler_mutex_trylock(h)      pthread_mutex_trylock(&h->reply_lock)

/** state define */
#define STATE_PENDING 0X00
#define STATE_LOADING 0X01
#define STATE_IDLE    0X02
#define STATE_RUNNING 0X04


typedef struct reply{
        int argc;
        robj** argv;
        struct redisCommand* cmd;
        redisDb* db;
        sds proto_text;
        int errnum;
        const char* err;
        list* result; // robj list
}reply_t;


struct kv_handler {
        int state;                  /** current state  */
        redisDb *db;
        unsigned int dbindex;       /* current db index */
        int dbnum;                  /* Total number of configured DBs */
        dict *commands;             /* Command table */
        struct sharedObjectsStruct shared;
        /* Fields used only for stats */
        long long stat_expiredkeys;     /* Number of expired keys */
        long long stat_keyspace_hits;   /* Number of successful lookups of keys */
        long long stat_keyspace_misses; /* Number of failed lookups of keys */

        long long dirty;                /* Changes to DB from the last save */
        /* Limits */
        unsigned long long maxmemory;   /* Max number of memory bytes to use */
        int maxmemory_policy;           /* Policy for key eviction */
        /* Sort parameters - qsort_r() is only available under BSD so we
         * have to take this state global, in order to pass it to sortCompare() */
        int sort_desc;
        int sort_alpha;
        int sort_bypattern;
        int sort_store;
        /* Zip structure config, see redis.conf for more information  */
        size_t hash_max_ziplist_entries;
        size_t hash_max_ziplist_value;
        size_t list_max_ziplist_entries;
        size_t list_max_ziplist_value;
        size_t set_max_intset_entries;
        size_t zset_max_ziplist_entries;
        size_t zset_max_ziplist_value;
        size_t hll_sparse_max_bytes;
        time_t unixtime;        /* Unix time sampled every cron cycle. */
        long long mstime;       /* Like 'unixtime' but with milliseconds resolution. */
        reply_t *reply;
        mutex_lock_t reply_lock;      /** reply mutex lock */
        u_int64_t reply_count;
};




/** handler */
struct kv_handler* kv_handler_create();
void kv_handler_release(struct kv_handler *handler);

/** reply */
reply_t* reply_create();
void reply_free_result(reply_t* c);
void reply_free(reply_t* c);
void reply_set_err(reply_t* c, int errnum);
void reply_add_result(reply_t *c, robj *o);
void reply_add_result_long(reply_t *c, long value);
void reply_add_result_str(reply_t *c, const char* str);


typedef void redisCommandProc(struct kv_handler *handler);
typedef int *redisGetKeysProc(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);
struct redisCommand {
        char *name;
        redisCommandProc *proc;
        int arity;
        char *sflags; /* Flags as string representation, one char per flag. */
        int flags;    /* The actual flags, obtained from the 'sflags' field. */
        /* Use a function to determine keys arguments in a command line.
         * Used for Redis Cluster redirect. */
        redisGetKeysProc *getkeys_proc;
        /* What keys should be loaded in background when calling this command? */
        int firstkey; /* The first argument that's a key (0 = no keys) */
        int lastkey;  /* The last argument that's a key */
        int keystep;  /* The step between first and last key */
        long long microseconds, calls;
};

struct redisFunctionSym {
        char *name;
        unsigned long pointer;
};

typedef struct _redisSortObject {
        robj *obj;
        union {
                double score;
                robj *cmpobj;
        } u;
} redisSortObject;

typedef struct _redisSortOperation {
        int type;
        robj *pattern;
} redisSortOperation;

/* Structure to hold list iteration abstraction. */
typedef struct {
        robj *subject;
        unsigned char encoding;
        unsigned char direction; /* Iteration direction */
        unsigned char *zi;
        listNode *ln;
} listTypeIterator;

/* Structure for an entry while iterating over a list. */
typedef struct {
        listTypeIterator *li;
        unsigned char *zi;  /* Entry in ziplist */
        listNode *ln;       /* Entry in linked list */
} listTypeEntry;

/* Structure to hold set iteration abstraction. */
typedef struct {
        robj *subject;
        int encoding;
        int ii; /* intset iterator */
        dictIterator *di;
} setTypeIterator;

/* Structure to hold hash iteration abstraction. Note that iteration over
 * hashes involves both fields and values. Because it is possible that
 * not both are required, store pointers in the iterator to avoid
 * unnecessary memory allocation for fields/values. */
typedef struct {
        robj *subject;
        int encoding;

        unsigned char *fptr, *vptr;

        dictIterator *di;
        dictEntry *de;
} hashTypeIterator;

#define REDIS_HASH_KEY 1
#define REDIS_HASH_VALUE 2

/*-----------------------------------------------------------------------------
 * Extern declarations
 *----------------------------------------------------------------------------*/

extern dictType setDictType;
extern dictType zsetDictType;
extern dictType clusterNodesDictType;
extern dictType clusterNodesBlackListDictType;
extern dictType dbDictType;
extern dictType shaScriptObjectDictType;
extern double R_Zero, R_PosInf, R_NegInf, R_Nan;
extern dictType hashDictType;
extern dictType replScriptCacheDictType;

/*-----------------------------------------------------------------------------
 * Functions prototypes
 *----------------------------------------------------------------------------*/

/* libkv */
int processProto(reply_t *c);


/* Utils */
long long ustime();
long long mstime();
void getRandomHexChars(char *p, unsigned int len);
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);
size_t redisPopcount(void *s, long count);


int dictSdsKeyCompare(void *privdata, const void *key1, const void *key2);

/* List data type */
void listTypeTryConversion(struct kv_handler *handler, robj *subject, robj *value);
void listTypePush(struct kv_handler *handler, robj *subject, robj *value, int where);
unsigned long listTypeLength(robj *subject);
listTypeIterator *listTypeInitIterator(robj *subject, long index, unsigned char direction);
void listTypeReleaseIterator(listTypeIterator *li);
int listTypeNext(listTypeIterator *li, listTypeEntry *entry);
robj *listTypeGet(struct kv_handler *handler, listTypeEntry *entry);
void listTypeInsert(listTypeEntry *entry, robj *value, int where);
int listTypeEqual(listTypeEntry *entry, robj *o);
void listTypeDelete(listTypeEntry *entry);
void listTypeConvert(struct kv_handler *handler, robj *subject, int enc);
void handleClientsBlockedOnLists(void);
void signalListAsReady(redisDb *db, robj *key);

/* MULTI/EXEC/WATCH... */
void touchWatchedKey(redisDb *db, robj *key);
void touchWatchedKeysOnFlush(int dbid);

/* Redis object implementation */
void decrRefCount(robj *o);
void decrRefCountVoid(void *o);
void incrRefCount(robj *o);
robj *resetRefCount(robj *obj);
void freeStringObject(robj *o);
void freeListObject(robj *o);
void freeSetObject(robj *o);
void freeZsetObject(robj *o);
void freeHashObject(robj *o);
robj *createObject(int type, void *ptr);
robj *createStringObject(char *ptr, size_t len);
robj *createRawStringObject(char *ptr, size_t len);
robj *createEmbeddedStringObject(char *ptr, size_t len);
robj *dupStringObject(robj *o);
int isObjectRepresentableAsLongLong(robj *o, long long *llongval);
robj *tryObjectEncoding(struct kv_handler *handler, robj *o);
robj *getDecodedObject(robj *o);
size_t stringObjectLen(robj *o);
robj *createStringObjectFromLongLong(struct kv_handler *handler, long long value);
robj *createStringObjectFromLongDouble(long double value, int humanfriendly);
robj *createListObject(void);
robj *createZiplistObject(void);
robj *createSetObject(void);
robj *createIntsetObject(void);
robj *createHashObject(void);
robj *createZsetObject(void);
robj *createZsetZiplistObject(void);
int getDoubleFromObj(robj *o, double *target);
int getLongLongFromObject(robj *o, long long *target);
int getLongDoubleFromObject(robj *o, long double *target);
char *strEncoding(int encoding);
int compareStringObjects(robj *a, robj *b);
int collateStringObjects(robj *a, robj *b);
int equalStringObjects(robj *a, robj *b);
unsigned long long estimateObjectIdleTime(robj *o);
#define sdsEncodedObject(objptr) (objptr->encoding == REDIS_ENCODING_RAW || objptr->encoding == REDIS_ENCODING_EMBSTR)


/* Struct to hold a inclusive/exclusive range spec by score comparison. */
typedef struct {
    double min, max;
    int minex, maxex; /* are min or max exclusive? */
} zrangespec;

/* Struct to hold an inclusive/exclusive range spec by lexicographic comparison. */
typedef struct {
    robj *min, *max;  /* May be set to shared.(minstring|maxstring) */
    int minex, maxex; /* are min or max exclusive? */
} zlexrangespec;

zskiplist *zslCreate(void);
void zslFree(zskiplist *zsl);
zskiplistNode *zslInsert(zskiplist *zsl, double score, robj *obj);
unsigned char *zzlInsert(unsigned char *zl, robj *ele, double score);
int zslDelete(zskiplist *zsl, double score, robj *obj);
zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range);
zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range);
double zzlGetScore(unsigned char *sptr);
void zzlNext(unsigned char *zl, unsigned char **eptr, unsigned char **sptr);
void zzlPrev(unsigned char *zl, unsigned char **eptr, unsigned char **sptr);
unsigned int zsetLength(robj *zobj);
void zsetConvert(robj *zobj, int encoding);
unsigned long zslGetRank(zskiplist *zsl, double score, robj *o);

/* Core functions */
struct redisCommand *lookupCommand(struct kv_handler *handler, sds name);
void propagate(struct redisCommand *cmd, int dbid, robj **argv, int argc, int flags);
void alsoPropagate(struct redisCommand *cmd, int dbid, robj **argv, int argc, int target);
void updateDictResizePolicy(void);
int htNeedsResize(dict *dict);
void oom(const char *msg);
void resetCommandTableStats(void);
void adjustOpenFilesLimit(void);
void closeListeningSockets(int unlink_unix_socket);
void updateCachedTime(void);
void resetServerStats(void);
unsigned int getLRUClock(void);

/* Set data type */
robj *setTypeCreate(robj *value);
int setTypeAdd(struct kv_handler *handler, robj *subject, robj *value);
int setTypeRemove(robj *subject, robj *value);
int setTypeIsMember(robj *subject, robj *value);
setTypeIterator *setTypeInitIterator(robj *subject);
void setTypeReleaseIterator(setTypeIterator *si);
int setTypeNext(setTypeIterator *si, robj **objele, int64_t *llele);
robj *setTypeNextObject(struct kv_handler *handler, setTypeIterator *si);
int setTypeRandomElement(robj *setobj, robj **objele, int64_t *llele);
unsigned long setTypeSize(robj *subject);
void setTypeConvert(struct kv_handler *handler, robj *subject, int enc);

/* Hash data type */
void hashTypeConvert(struct kv_handler *handler, robj *o, int enc);
void hashTypeTryConversion(struct kv_handler *handler, robj *subject, robj **argv, int start, int end);
void hashTypeTryObjectEncoding(struct kv_handler *handler, robj *subject, robj **o1, robj **o2);
robj *hashTypeGetObject(struct kv_handler *handler, robj *o, robj *key);
int hashTypeExists(robj *o, robj *key);
int hashTypeSet(struct kv_handler *handler, robj *o, robj *key, robj *value);
int hashTypeDelete(robj *o, robj *key);
unsigned long hashTypeLength(robj *o);
hashTypeIterator *hashTypeInitIterator(robj *subject);
void hashTypeReleaseIterator(hashTypeIterator *hi);
int hashTypeNext(hashTypeIterator *hi);
void hashTypeCurrentFromZiplist(hashTypeIterator *hi, int what,
                                unsigned char **vstr,
                                unsigned int *vlen,
                                long long *vll);
void hashTypeCurrentFromHashTable(hashTypeIterator *hi, int what, robj **dst);
robj *hashTypeCurrentObject(struct kv_handler *handler, hashTypeIterator *hi, int what);

/* Pub / Sub */
void freePubsubPattern(void *p);
int listMatchPubsubPattern(void *a, void *b);
int pubsubPublishMessage(robj *channel, robj *message);

/* db.c -- Keyspace access API */
robj *lookupKey(redisDb *db, robj *key);
robj *lookupKeyRead(struct kv_handler *handler, redisDb *db, robj *key);
robj *lookupKeyWrite(struct kv_handler *handler, redisDb *db, robj *key);
void dbAdd(redisDb *db, robj *key, robj *val);
void dbOverwrite(redisDb *db, robj *key, robj *val);
void setKey(struct kv_handler *handler, redisDb *db, robj *key, robj *val);
int dbExists(redisDb *db, robj *key);
int dbDelete(redisDb *db, robj *key);
robj *dbUnshareStringValue(redisDb *db, robj *key, robj *o);
long long emptyDb(void(callback)(void*));
int selectDb(struct kv_handler *handler, int id);
void signalModifiedKey(redisDb *db, robj *key);
void signalFlushedDb(int dbid);
int verifyClusterConfigWithData(void);

/** expire */
void setExpire(struct kv_handler *handler, redisDb *db, robj *key, long long when);
int expireIfNeeded(struct kv_handler *handler, redisDb *db, robj *key);
int removeExpire(redisDb *db, robj *key);

/* API to get key arguments from commands */
int *getKeysFromCommand(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);
void getKeysFreeResult(int *result);
int *zunionInterGetKeys(struct redisCommand *cmd,robj **argv, int argc, int *numkeys);
int *evalGetKeys(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);
int *sortGetKeys(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);

/* Scripting */
void scriptingInit(void);

/* Git SHA1 */
char *redisGitSHA1(void);
char *redisGitDirty(void);
uint64_t redisBuildId(void);


/** kv expire manager */
kv_expire_manager_t* kv_expire_manager_create();
void kv_expire_manager_destroy(struct kv_expire_manager *manager);
void kv_expire_manager_add(struct kv_handler *handler, long long key, void *value);
int kv_expire_manager_remove(struct kv_handler *handler, long long count);
int kv_expire_manager_remove_complete(struct kv_handler *handler, long long key, void *value);
void kv_expire_manager_clear(struct kv_handler *handler, struct kv_expire_manager *manager);


/* Commands prototypes */
void getCommand(struct kv_handler *handler);
void setCommand(struct kv_handler *handler);
void delCommand(struct kv_handler *handler);
void incrCommand(struct kv_handler *handler);
void incrbyCommand(struct kv_handler *handler);
void decrCommand(struct kv_handler *handler);
void decrbyCommand(struct kv_handler *handler);
void lpushCommand(struct kv_handler *handler);
void lrangeCommand(struct kv_handler *handler);
void saddCommand(struct kv_handler *handler);
void sremCommand(struct kv_handler *handler);
void smembersCommand(struct kv_handler *handler);
void zaddCommand(struct kv_handler *handler);
void zrangeCommand(struct kv_handler *handler);
void hsetCommand(struct kv_handler *handler);
void hgetCommand(struct kv_handler *handler);
void hmsetCommand(struct kv_handler *handler);
void hmgetCommand(struct kv_handler *handler);
void hgetallCommand(struct kv_handler *handler);
void hdelCommand(struct kv_handler *handler);
void existsCommand(struct kv_handler *handler);
void scardCommand(struct kv_handler *handler);
void selectCommand(struct kv_handler *handler);
void sismemberCommand(struct kv_handler *handler);
void srandmemberCommand(struct kv_handler *handler);
void typeCommand(struct kv_handler *handler);
void zcardCommand(struct kv_handler *handler);
void dbsizeCommand(struct kv_handler *handler);
void flushdbCommand(struct kv_handler *handler);
void flushallCommand(struct kv_handler *handler);
void expireCommand(struct kv_handler *handler);
void expireatCommand(struct kv_handler *handler);
void pexpireCommand(struct kv_handler *handler);
void pexpireatCommand(struct kv_handler *handler);
void echoCommand(struct kv_handler *handler);
void llenCommand(struct kv_handler *handler);
void msetCommand(struct kv_handler *handler);
void rpushCommand(struct kv_handler *handler);
void lpopCommand(struct kv_handler *handler);
void rpopCommand(struct kv_handler *handler);

#define KV_DEFAULT_DBNUM     16


#endif
