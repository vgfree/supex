/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file    libkv.h
 * @detail  libkv API header file.
 *
 * @author  shishengjie
 * @date    2015-05-12
 *
 *
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
 *             	      Returns message..
 *             Return value
 *             	      success:Bulk string reply.
 *             	      failed :error code.
 *
 *           - <b>llen key</b>
 *             Description
 *             	      Returns the length of the list stored at key.
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
 *            	      Sets the given keys to their respective values.
 *             Return value
 *                    success:always OK since MSET can't fail.
 *                    failed :error code.
 *           
 *	     - <b>rpop key</b>
 *             Description
 *            	      Removes and returns the first element of the list stored at key.
 *             Return value
 *                    success:the value of the last element, or nil when key does not exist.
 *                    failed :error code.
 *	 
 *	     - <b>lpop key</b>
 *             Description
 *            	      Removes and returns a list of key elements in the head.
 *             Return value
 *                    success:the value of the first element, or nil when key does not exist.
 *                    failed :error code.
 *
 * 	     - <b>type key</b>
 *             Description
 *             	      Get the type of the key(string,list,set,zset,hash,none).
 *             Return value
 *                    success:Return the type of the key.
 *                    failed :error code.
 *
 *           - <b>select dbindex</b>
 *             Description
 *             	      Switch current db to the specified according to the index. 
 *             Return value
 *                    success:Return "OK".
 *                    failed :error code.
 *
 *           - <b>flushall</b>
 *             Description
 *             	      Remove all data from all databases.
 *             Return value
 *                    success:Return "OK".
 *                    failed :error code.
 *
 *           - <b>hset key field value</b>
 *             Description
 *             	      Set the string value of a hash field.
 *             Return value
 *                    success:Return 1 if new field is set, otherwise update return 0.
 *                    failed :error code.
 *
 *           - <b>hget key field</b>
 *             Description
 *             	      Get the string value of a hash field.
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
 *             	      Removes the specified fields from the hash stored at key.
 *             Return value
 *                    success:the number of fields that were removed from the hash, not including specified but non existing fields.
 *                    failed :error code.
 *         
 *           - <b>sismember key member</b>
 *             Description
 *             	      Determine if a given value is a member of a set.
 *             Return value
 *                    success:Return 1 if the member is a member of the set, otherwise return 0.
 *                    failed :error code.
 *
 *           - <b>scard key</b>
 *             Description
 *             	      Get the number of key in the set.
 *             Return value
 *                    success:Return the number of key in the set. If the key not exist, 0 is returned.
 *                    failed :error code.
 *
 *           - <b>srandmember key [count]</b>
 *             Description
 *             	      Get one or multiple random members from a set.
 *             Return value
 *                    success:Return the number of random from a set. 1 is returned by default, otherwise return the number relative to
 *                            the parameter count.
 *                    failed :error code.
 *
 *           - <b>srem key member[member ...]</b>
 *             Description
 *             	      Remove the specified members from the set stored at key.
 *             Return value
 *                    success:the number of members that were removed from the set, not including non existing members.
 *                    failed :error code.
 *
 *           </pre>
 */

#ifndef LIBKV_H_
#define LIBKV_H_



#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup group_error_code Error code
 * @{
 * User level error code.
 */
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
/**
 * @}
 */
        
/**
 * @defgroup group_result_container Result container
 * Contain command execution results.
 * @{
 */

/** result list iterator from head */
#define ANSWER_HEAD 0
/** result list iterator from tail */
#define ANSWER_TAIL 1


        typedef struct kv_handler kv_handler_t;

        /**
         * kv_answer_t container node entry.
         */
        typedef struct kv_answer_value {
                void *ptr;
                unsigned long ptrlen;
                struct kv_answer_value* prev;
                struct kv_answer_value* next;
        }kv_answer_value_t;

        /**
         * kv_answer_t container list iterator. 
         */
        typedef struct kv_answer_iter {
                kv_answer_value_t *next;
                int direction;
        }kv_answer_iter_t;

        /**
         * answer_t container(list).
         */
        typedef struct kv_answer {
                int errnum;
                const char* err;
                unsigned long count; 
                struct kv_answer_value* head, *tail;
        }kv_answer_t;
    

        /**
         * Free kv_answer_t structure, per node->ptr and per node.
         * @param a kv_answer_t pointer to free.
         */
        void kv_answer_release(kv_answer_t *a);

        /**
         * Get the value number of list in answer.
         * @param a Which answer to get from.
         * @return Value number in list contain.
         */
        unsigned long kv_answer_length(kv_answer_t *a);

        /**
         * Get the iterator of kv_answer_t. Overall nodes through it.
         * @param a kv_answer_t where iterator from.
         * @param direction Ensure direction start from head or tail.
         * @return A new malloc iterator of a's. 
         */
        kv_answer_iter_t* kv_answer_get_iter(kv_answer_t *a, int direction);

        /**
         * Rewind iterator to start position.
         * @param a kv_answer_t where iterator from.
         * @param iter Reset iterator to start position.
         */
        void kv_answer_rewind_iter(kv_answer_t *a, kv_answer_iter_t *iter);

        /**
         * Free iterator.
         * @param iter Iterator to be freed.
         */
        void kv_answer_release_iter(kv_answer_iter_t *iter);

        /**
         * Get current value entry and move iterator to the next.
         * @param iter Iterator where value from.
         * @return return current iterator's value pointer.
         */
        kv_answer_value_t* kv_answer_next(kv_answer_iter_t *iter);

        /**
         * Get first value entry pointer.
         * @param ans kv_answer_t where get from.
         * @return First value entry pointer or NULL if empty list.
         */
        kv_answer_value_t* kv_answer_first_value(kv_answer_t *ans);

        /**
         * Get last value entry pointer.
         * @param ans kv_answer_t where get from.
         * @return Last value entry pointer or NULL if empty list.
         */
        kv_answer_value_t* kv_answer_last_value(kv_answer_t *ans);

        /**
         * Convert value->ptr to a null-terminated string,
         * append '\0' character to value->ptr.
         * @param value Value entry pointer to be converted.
         * @return Force to convert to char* type and return.
         */
        char* kv_answer_value_to_string(kv_answer_value_t *value);
/**
 *@}
 */

        
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
        kv_handler_t* kv_create(const char *param1, ...);

        /**
         * Free libkv handler resources if you want to do nothing next with the handler.
         */
        void kv_destroy(kv_handler_t *handler);

        /**
         * Command executes interface.
         * @param  handler where the resource from.
         * @param  cmd Specified length command bytes array.
         * @param  cmdlen The length(bytes) of command bytes array.
         * @return kv_answer_t The structure contains command execution result.
         */
        kv_answer_t* kv_ask(kv_handler_t *handler, const char *cmd, unsigned int cmdlen);

        /**
         * Get libkv version.
         * @return libkv version string
         */
        const char* kv_version();

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

        

#ifdef __cplusplus
}
#endif


#endif
