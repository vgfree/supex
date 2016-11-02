#ifndef LIBKV_H_
#define LIBKV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @defgroup group_error_code Error code
 * @{
 * User level error code.
 */
#define ERROR_MAP(XX)								   \
	XX(ERR_NONE, "no error")						   \
	XX(ERR_TYPE, "-ERR wrong type")						   \
	XX(ERR_NO_KEY, "-ERR no such key")					   \
	XX(ERR_SYNTAX, "-ERR syntax error")					   \
	XX(ERR_SAME_OBJECT, "-ERR source and destination objects are the same")	   \
	XX(ERR_OUT_OF_RANGE, "-ERR value is not an integer or index out of range") \
	XX(ERR_NIL, "-ERR nil")							   \
	XX(ERR_EMPTY, "-ERR empty list or set")							   \
	XX(ERR_NOT_INIT, "-ERR not init")					   \
	XX(ERR_BUSY, "-ERR busy")						   \
	XX(ERR_ARGUMENTS, "-ERR wrong number of arguments")			   \
	XX(ERR_PROTOCOL, "-ERR protocol error")					   \
	XX(ERR_VALUE, "-ERR value invalid")					   \
	XX(ERR_DB_INDEX, "-ERR invalid DB index")				   \
	XX(ERR_CMD, "-ERR unknown command")

typedef enum __error_type_t
{
#define XX(name, _) name,
	ERROR_MAP(XX)
#undef XX
	ERR_EOF,
} error_type_t;

static inline
const char *error_getname(int type)
{
#define ERROR_NAME_GEN(name, _)	\
	case name:		\
		return #name;

	switch (type)
	{
		ERROR_MAP(ERROR_NAME_GEN)
		default:
			return NULL;
	}

#undef ERROR_NAME_GEN
}

static inline
const char *error_getinfo(int type)
{
#define ERROR_INFO_GEN(name, info) \
	case name:		   \
		return info;

	switch (type)
	{
		ERROR_MAP(ERROR_INFO_GEN)
		default:
			return "未知信息";
	}

#undef ERROR_INFO_GEN
}

static inline
void error_print(void)
{
#define ERROR_PRINT_GEN(name, info) printf("value:%d, name:%s, info:%s\n", name,#name, info);
	ERROR_MAP(ERROR_PRINT_GEN)
#undef ERROR_PRINT_GEN
}

/**
 * @}
 */

/**
 * @defgroup group_result_container Result container
 * Contain command execution results.
 * @{
 */

/**
 * Global domain structure.
 */
/** result list iterator from head */
#define ANSWER_HEAD     0
/** result list iterator from tail */
#define ANSWER_TAIL     1


enum value_type
{
	VALUE_TYPE_NIL = -1,
	VALUE_TYPE_STAR = 0,
	VALUE_TYPE_INTPTR,
	VALUE_TYPE_INT,
	VALUE_TYPE_DOUBLE,
	VALUE_TYPE_CHAR,
};

typedef struct _value
{
	enum value_type type;
	union
	{
		void            *_star;
		intptr_t        _intptr;
		int             _int;
		double          _double;
		char            _char;
	};
	unsigned long   size;
} _value_t;

#define _VALUE_INIT(val)		      \
	do {				      \
		(val)->type = VALUE_TYPE_NIL; \
		(val)->_star = NULL;	      \
		(val)->size = 0;	      \
	} while (0);

#define _VALUE_ZERO(val)						  \
	do {								  \
		if (((val)->type == VALUE_TYPE_STAR) && ((val)->_star)) { \
			zfree((val)->_star);				  \
		}							  \
		_VALUE_INIT(val);					  \
	} while (0);

#define _VALUE_LOOK_ADDR(val)   ((char *)(((val)->type == VALUE_TYPE_STAR) ? (val)->_star : &(val)->_star))
#define _VALUE_LOOK_SIZE(val)   ((val)->size)
#define _VALUE_LOOK_TYPE(val)   ((val)->type)

void value_copy_string(struct _value *val, char *str, size_t len);
#define _VALUE_COPY_STRING(val, str, len) value_copy_string(val, str, len)

#define _VALUE_LOOK_STRING(val) ((val)->_star)

#define _VALUE_COPY_INTPTR(val, _intptr_)	     \
	do {					     \
		(val)->type = VALUE_TYPE_INTPTR;     \
		(val)->_intptr = (intptr_t)_intptr_; \
		(val)->size = sizeof(intptr_t);	     \
	} while (0);
#define _VALUE_LOOK_INTPTR(val) ((val)->_intptr)

#define _VALUE_COPY_INT(val, _int_)	      \
	do {				      \
		(val)->type = VALUE_TYPE_INT; \
		(val)->_int = (int)_int_;     \
		(val)->size = sizeof(int);    \
	} while (0);
#define _VALUE_LOOK_INT(val)    ((val)->_int)

#define _VALUE_COPY_DOUBLE(val, _double_)	   \
	do {					   \
		(val)->type = VALUE_TYPE_DOUBLE;   \
		(val)->_double = (double)_double_; \
		(val)->size = sizeof(double);	   \
	} while (0);
#define _VALUE_LOOK_DOUBLE(val) ((val)->_double)

#define _VALUE_COPY_CHAR(val, _char_)	       \
	do {				       \
		(val)->type = VALUE_TYPE_CHAR; \
		(val)->_char = (char)_char_;   \
		(val)->size = sizeof(char);    \
	} while (0);
#define _VALUE_LOOK_CHAR(val)   ((val)->_char)

#define _VALUE_PRINT(val)								      \
	do {										      \
		switch ((val)->type)							      \
		{									      \
			case VALUE_TYPE_NIL:						      \
				printf("[nil]\n");					      \
				break;							      \
			case VALUE_TYPE_STAR:						      \
				printf("[string]");					      \
				size_t i;						      \
				for (i = 0; i < _VALUE_LOOK_SIZE(val); i++) {		      \
					printf("%c", ((char *)(_VALUE_LOOK_STRING(val)))[i]); \
				}							      \
				printf("\n");						      \
				break;							      \
			case VALUE_TYPE_INTPTR:						      \
				printf("[intptr_t]%ld\n", _VALUE_LOOK_INTPTR(val));	      \
				break;							      \
			case VALUE_TYPE_INT:						      \
				printf("[int]%d\n", _VALUE_LOOK_INT(val));		      \
				break;							      \
			case VALUE_TYPE_DOUBLE:						      \
				printf("[double]%f\n", _VALUE_LOOK_DOUBLE(val));	      \
				break;							      \
			case VALUE_TYPE_CHAR:						      \
				printf("[char]%c\n", _VALUE_LOOK_CHAR(val));		      \
				break;							      \
			default:							      \
				break;							      \
		}									      \
	} while (0);

/****************************************************************
*			answer					*
****************************************************************/

/**
 * kv_answer_t container node entry.
 */
typedef struct kv_answer_value
{
	int                     errnum;
	struct _value           data;
	struct kv_answer_value  *prev;
	struct kv_answer_value  *next;
} kv_answer_value_t;

/**
 * kv_answer_t container list iterator.
 */
typedef struct kv_answer_iter
{
	kv_answer_value_t       *cursor;
	int                     direction;
} kv_answer_iter_t;

/**
 * answer_t container(list).
 */
typedef struct kv_answer
{
	int                     errnum;
	unsigned long           count;
	struct kv_answer_value  *head, *tail;
} kv_answer_t;

#define answer_value_look_addr(value)   _VALUE_LOOK_ADDR( &(value)->data)
#define answer_value_look_size(value)   _VALUE_LOOK_SIZE( &(value)->data)
#define answer_value_look_type(value)   _VALUE_LOOK_TYPE( &(value)->data)
#define answer_value_look_string(value) _VALUE_LOOK_STRING( &(value)->data)
#define answer_value_look_intptr(value) _VALUE_LOOK_INTPTR( &(value)->data)
#define answer_value_look_int(value)    _VALUE_LOOK_INT( &(value)->data)
#define answer_value_look_double(value) _VALUE_LOOK_DOUBLE( &(value)->data)
#define answer_value_look_char(value)   _VALUE_LOOK_CHAR( &(value)->data)
#define answer_value_print(value)       _VALUE_PRINT( &(value)->data)
char *answer_value_to_string(kv_answer_value_t *value);

#define answer_length(ans)              (((ans) == NULL) ? 0 : (ans)->count)
#define answer_head_value(ans)          (((ans) == NULL) ? NULL : (ans)->head)
#define answer_last_value(ans)          (((ans) == NULL) ? NULL : (ans)->tail)
kv_answer_value_t *answer_get_index_value(kv_answer_t *a, size_t index);

kv_answer_iter_t *answer_iter_make(kv_answer_t *a, int direction);

void answer_iter_free(kv_answer_iter_t *iter);

kv_answer_value_t *answer_iter_next(kv_answer_iter_t *iter);

/****************************************************************
*			asking					*
****************************************************************/
typedef struct kv_asking_value
{
	struct _value           data;
	struct kv_asking_value  *prev;
	struct kv_asking_value  *next;
} kv_asking_value_t;

/**
 * kv_asking_t container list iterator.
 */
typedef struct kv_asking_iter
{
	kv_asking_value_t       *cursor;
	int                     direction;
} kv_asking_iter_t;

typedef struct kv_asking
{
	uint8_t                 id;/* current select db */

	char                    cmd[16];

	unsigned long           count;
	kv_asking_value_t       *head;
	kv_asking_value_t       *tail;
} kv_asking_t;

/****************************************************************
*			all					*
****************************************************************/
void kv_init(void);

typedef struct kv_config
{
	bool    open_on_disk;
	bool    data_is_pure;
	long    time_to_stay;
} kv_config_t;

int kv_load(kv_config_t *cfg, char *ident);

/**
 * Free handler resources and intialise NULL.
 * @note kv_destroy(void)
 */
void kv_destroy(void);

/**
 * cmd handler entry.
 */
typedef struct kv_handler
{
	kv_asking_t     asking;
	kv_answer_t     answer;
} kv_handler_t;

/**
 * Command executes interface.
 * @param  handler where the resource from.
 * @param  cmd Specified length command bytes array.
 * @param  cmdlen The length(bytes) of command bytes array.
 * @return kv_answer_t The structure contains command execution result.
 */
enum ask_proto_type
{
	REDIS_PROTO_TYPE = 1,
	SPLIT_PROTO_TYPE
};

kv_handler_t *_kv_ask(int dbindex, enum ask_proto_type type, const char *cmd_data, unsigned int cmd_len);

#define kv_rds(dbindex, cmd_data, cmd_len)      _kv_ask(dbindex, REDIS_PROTO_TYPE, cmd_data, cmd_len)
#define kv_spl(dbindex, cmd_data, cmd_len)      _kv_ask(dbindex, SPLIT_PROTO_TYPE, cmd_data, cmd_len)

kv_handler_t *_kv_opt(int dbindex, const char *cmd_head, ...);

#define kv_opt(dbindex, cmd_head, ...) _kv_opt(dbindex, cmd_head, ##__VA_ARGS__, NULL)

struct kv_argv {
	void *ptr;
	size_t len;
};
kv_handler_t *kv_arg(int dbindex, const char *cmd_head, int argc, struct kv_argv args[]);

void kv_handler_release(kv_handler_t *handler);

/**
 * Clone a new handler.
 * @return new handler the same as old.
 */
kv_handler_t *kv_clone(kv_handler_t *old);

/**
 * Get libkv version.
 * @return libkv version string
 */
const char *kv_version();

/**
 * Get libkv version numeric. Format [xxyyzz]->[xx-major yy-minor zz-revision]
 * @return A long type number indicates version.
 */
long kv_version_numeric();

/**
 * Get current memory used by libkv.
 * @return Bytes used by libkv.
 */
unsigned int kv_get_used_memory();

/**
 * Create a new libkv handler. You can pass param-value pairs or a single parameter like:
 *        a_new_handler = kv_create("enable_rdb",
 *                                  "rdb_file", "./dump.rdb",
 *                                  "oom_cb", oom_cb,
 *                                  ...
 *                                  );
 *
 * @param  param1,... Parameters list(not support currently, just pass NULL instead).
 * @return A new handler returned, otherwise oom callback triggered.
 */

/**
 * Config parameters:
 *       "dispatcher",dispatcher_func_addr,
 *
 */
// kv_handler_t *kv_create(const char *param1, ...);



/* ------------------------------export------------------------------------- */

/**
 * 创建对象池
 */
void *kv_pool_create(size_t peak, kv_config_t *cfg);

/**
 * 在C层已句柄名称调用，查找或创建对应的句柄，并返回该句柄
 */
int kv_pool_search(void *root, const char *name);

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
#endif	/* ifndef LIBKV_H_ */

/*
 * @mainpage Libkv references
 *           <pre>
 * <b>libkv command list</b>
 *
 *           - <b>set key value</b>
 *             Description
 *                    set string value of a key.
 *             Return value
 *                    success:"OK".
 *                    failed :error code.
 *
 *           - <b>del key [key ...]</b>
 *             Description
 *                    Delete a key
 *             Return value
 *                    success:number of deleted.
 *                    failed :error code.
 *
 *           - <b>get key</b>
 *             Description
 *                    Get string value of a key.
 *             Return value
 *                    success:the value of key.
 *                    failed :error code.
 *
 *           - <b>dbsize</b>
 *             Description
 *                    Get the number of keys in the select db.
 *             Return value
 *                    success:the number of keys in the select db.
 *                    failed :error code.
 *
 *           - <b>flushdb</b>
 *             Description
 *                    Remove all keys from the current database.
 *             Return value
 *                    success:"OK"
 *                    failed :error code.
 *
 *           - <b>incr key</b>
 *             Description
 *                    Increment the integer value of a key by one.
 *             Return value
 *                    success:the value of key after the increment.
 *                    failed :error code.
 *
 *           - <b>incrby key increment</b>
 *             Description
 *                    Increment the integer value of a key by the given amount.
 *             Return value
 *                    success:the value of key after the increment.
 *                    failed :error code.
 *
 *           - <b>decr key</b>
 *             Description
 *                    Decrement integer value of key by one.
 *             Return value
 *                    success:the value of key after decrement.
 *                    failed :error code.
 *
 *           - <b>decrby key decrement</b>
 *             Description
 *                    Decrement the integer value of a key by the given number.
 *             Return value
 *                    success:the value of key after the decrement.
 *                    failed :error code.
 *
 *           - <b>lpush key value [value ...]</b>
 *             Description
 *                    Prepend one or multiple values to a list.
 *             Return value
 *                    success:the length of the list after the push operations.
 *                    failed :error code.
 *
 *           - <b>exists key</b>
 *             Description
 *                    Determine if a key exists
 *             Return value
 *                    success:return 1 if exist, otherwise 0.
 *                    failed :error code.
 *
 *           - <b>lrange key start stop</b>
 *             Description
 *                    Get a range of elements from a list.
 *             Return value
 *                    success:An elements array which in the specified range.
 *                    failed :error code.
 *
 *           - <b>sadd key [key ...]</b>
 *             Description
 *                    Add one or more members to a set.
 *             Return value
 *                    success:The number of elements that were added to the set.
 *                    failed :error code.
 *
 *           - <b>smembers key</b>
 *             Description
 *                    Get all elements from a set.
 *             Return value
 *                    success:return all elements of the specified set(key).
 *                    failed :error code.
 *
 *           - <b>expire key seconds</b>
 *             Description
 *                    Set a key's time to live in seconds
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           - <b>expireat key timestamp</b>
 *             Description
 *                    Set a key's time to live in timestamp seconds.
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           - <b>pexpire key milliseconds</b>
 *             Description
 *                    Set a key's time to live in milliseconds.
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           - <b>pexpireat key milliseconds-timestamp</b>
 *             Description
 *                    Set a key's time to live in milliseconds-timestamp.
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           - <b>echo message</b>
 *             Description
 *                    Returns message..
 *             Return value
 *                    success:Bulk string reply.
 *                    failed :error code.
 *
 *           - <b>llen key</b>
 *             Description
 *                    Returns the length of the list stored at key.
 *             Return value
 *                    success:the length of the list at key.
 *                    failed :error code.
 *
 *	     - <b>rpush key value [value ...]</b>
 *             Description
 *                    Insert all the specified values at the tail of the list stored at key..
 *             Return value
 *                    success:the length of the list after the push operation..
 *                    failed :error code.
 *
 *	     - <b>mset key value [key value ...]</b>
 *             Description
 *                    Sets the given keys to their respective values.
 *             Return value
 *                    success:always OK since MSET can't fail.
 *                    failed :error code.
 *
 *	     - <b>rpop key</b>
 *             Description
 *                    Removes and returns the first element of the list stored at key.
 *             Return value
 *                    success:the value of the last element, or nil when key does not exist.
 *                    failed :error code.
 *
 *	     - <b>lpop key</b>
 *             Description
 *                    Removes and returns a list of key elements in the head.
 *             Return value
 *                    success:the value of the first element, or nil when key does not exist.
 *                    failed :error code.
 *
 *           - <b>type key</b>
 *             Description
 *                    Get the type of the key(string,list,set,zset,hash,none).
 *             Return value
 *                    success:Return the type of the key.
 *                    failed :error code.
 *
 *           - <b>select dbindex</b>
 *             Description
 *                    Switch current db to the specified according to the index.
 *             Return value
 *                    success:Return "OK".
 *                    failed :error code.
 *
 *           - <b>flushall</b>
 *             Description
 *                    Remove all data from all databases.
 *             Return value
 *                    success:Return "OK".
 *                    failed :error code.
 *
 *           - <b>hset key field value</b>
 *             Description
 *                    Set the string value of a hash field.
 *             Return value
 *                    success:Return 1 if new field is set, otherwise update return 0.
 *                    failed :error code.
 *
 *           - <b>hget key field</b>
 *             Description
 *                    Get the string value of a hash field.
 *             Return value
 *                    success:Return the value from the field of key.
 *                    failed :error code.
 *
 *           - <b>hmset key field value [field value ...]</b>
 *             Description
 *                    Sets the specified fields to their respective values in the hash stored at key. .
 *             Return value
 *                    success:Simple string reply
 *                    failed :error code.
 *
 *           - <b>hmget key field [field ...]</b>
 *             Description
 *                    Returns the values associated with the specified fields in the hash stored at key.
 *             Return value
 *                    success:list of values associated with the given fields, in the same order as they are requested.
 *                    failed :error code.
 *
 *           - <b>hgetall key</b>
 *             Description
 *                    Returns all fields and values of the hash stored at key.
 *             Return value
 *                    success:list of fields and their values stored in the hash, or an empty list when key does not exist.
 *                    failed :error code.
 *
 *           - <b>hdel key field [field ...]</b>
 *             Description
 *                    Removes the specified fields from the hash stored at key.
 *             Return value
 *                    success:the number of fields that were removed from the hash, not including specified but non existing fields.
 *                    failed :error code.
 *
 *           - <b>sismember key member</b>
 *             Description
 *                    Determine if a given value is a member of a set.
 *             Return value
 *                    success:Return 1 if the member is a member of the set, otherwise return 0.
 *                    failed :error code.
 *
 *           - <b>scard key</b>
 *             Description
 *                    Get the number of key in the set.
 *             Return value
 *                    success:Return the number of key in the set. If the key not exist, 0 is returned.
 *                    failed :error code.
 *
 *           - <b>srandmember key [count]</b>
 *             Description
 *                    Get one or multiple random members from a set.
 *             Return value
 *                    success:Return the number of random from a set. 1 is returned by default, otherwise return the number relative to
 *                            the parameter count.
 *                    failed :error code.
 *
 *           - <b>srem key member[member ...]</b>
 *             Description
 *                    Remove the specified members from the set stored at key.
 *             Return value
 *                    success:the number of members that were removed from the set, not including non existing members.
 *                    failed :error code.
 *
 *           - <b>zadd key score member[[score member][score member]...]</b>
 *             Description
 *                    Adds all the specified members with the specified scores to the sorted set stored at key.
 *             Return value
 *                    success:The number of elements added to the sorted sets, not including elements already existing for which the score was updated.
 *                    failed :error code.
 *
 *           - <b>zrem key member[member ...]</b>
 *             Description
 *                    Removes the specified members from the sorted set stored at key.
 *             Return value
 *                    success:The number of members removed from the sorted set, not including non existing members.
 *                    failed :error code.
 *
 *           - <b>zrange key start stop[withscores]</b>
 *             Description
 *                    Returns the specified range of elements in the sorted set stored at key.
 *             Return value
 *                    success:list of elements in the specified range (optionally with their scores).
 *                    failed :error code.
 *
 *            - <b>zcard key</b>
 *             Description
 *                    Returns the sorted set cardinality (number of elements) of the sorted set stored at key.
 *             Return value
 *                    success:the cardinality (number of elements) of the sorted set, or 0 if key does not exist.
 *                    failed :error code.
 *
 *             - <b>zincrby key increment member</b>
 *             Description
 *                    Increments the score of member in the sorted set stored at key by increment. .
 *             Return value
 *                    success:the new score of member (a double precision floating point number), represented as string.
 *                    failed :error code.
 *
 *             - <b>zscore key member</b>
 *             Description
 *                    Returns the score of member in the sorted set at key.
 *             Return value
 *                    success:the score of member (a double precision floating point number), represented as string.
 *                    failed :error code.
 *
 *             - <b>sinter key[key...]</b>
 *             Description
 *                    Intersect multiple sets.
 *             Return value
 *                    success:Return elements array of intersect sets.
 *                    failed :error code.
 *
 *             - <b>sdiffstore destination_key [key...]</b>
 *             Description
 *                    Subtract multiple sets and store the resulting set in destination_key.
 *             Return value
 *                    success:Return the number of elements in the resulting set.
 *                    failed :error code.
 *
 *             - <b>info</b>
 *             Description
 *                    Return some infomations about libkv.
 *             Return value
 *                    success:Return some infomations about libkv, such as:
 *			      version:
 *		              total handlers:
 *			      total_keys: (number of all keys)
 *			      expires: (number of expire keys)
 *		              expired:
 *		              used memory:
 *		              request: (number of kv_ask() have been requeseted)
 *                    failed :error code.
 *
 *	       - <b>setex</b>
 *	       Description
 *		      set string value of a key, and set the key's time to live in seconds.
 *	       Return value
 *		      success:"OK"
 *		      fail:error code
 *
 *	       - <b>psetex</b>
 *	       Description
 *	              set string value of a key, and set the key's time to live in milliseconds.
 *             Return value
 *                    success:"OK"
 *                    fail:error code
 *
 *           </pre>
 *
 */

