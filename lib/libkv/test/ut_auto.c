#include "ut_public.h"






/** version */
void case_version();
/** init */
void case_init();
void case_init_after_used();
void case_init_stress();
/** set */
void case_cmd_set_normal();
void case_cmd_set_invalid_param();
void case_cmd_set_invalid_type();
void case_cmd_set_multi_handler();
void case_cmd_set_stress();
/** get */
void case_cmd_get();
void case_cmd_get_invalid_type();
void case_cmd_get_nil();
void case_cmd_get_stress();
/** del */
void case_cmd_del();
void case_cmd_del_multi_handler();
void case_cmd_del_stress();
/** dbsize */
void case_cmd_dbsize();
void case_cmd_dbsize_expire();
void case_cmd_dbsize_multi_handler();
void case_cmd_dbsize_stress();
/** flushdb */
void case_cmd_flushdb();
void case_cmd_flushdb_multi_handler();
void case_cmd_flushdb_stress();
/** incr */
void case_cmd_incr();
void case_cmd_incr_multi_handler();
/** incrby */
void case_cmd_incrby();
void case_cmd_incrby_multi_handler();
/** decr */
void case_cmd_decr();
void case_cmd_decr_multi_handler();
/** decrby */
void case_cmd_decrby();
void case_cmd_decrby_multi_handler();
/** used memory */
void case_used_memory();
/** lpush */
void case_cmd_lpush_add_few_bytes();
void case_cmd_lpush_add_lot_bytes();
void case_cmd_lpush_del();
void case_cmd_lpush_invalid_param();
void case_cmd_lpush_multi_handler();
void case_cmd_lpush_stress();
/** exists */
void case_cmd_exists_normal();
void case_cmd_exists_invalid_cmd();
void case_cmd_exists_invalid_param();
void case_cmd_exists_multi_handler();
void case_cmd_exists_stress();
/** lrange */
void case_cmd_lrange_without_found();
void case_cmd_lrange_found();
void case_cmd_lrange_invalid_param();
void case_cmd_lrange_multi_db();
void case_cmd_lrange_multi_handler();
void case_cmd_lrange_stress();
/** sadd */
void cmd_case_sadd_normal();
void cmd_case_sadd_invalid_cmd();
void cmd_case_sadd_invalid_param();
void cmd_case_sadd_invalid_type();
void cmd_case_sadd_multi_handler();
void cmd_case_sadd_stress();
/** arem */
void cmd_case_srem();
void cmd_case_srem_invalid_cmd();
void cmd_case_srem_invalid_param();
void cmd_case_srem_invalid_type();
void cmd_case_srem_multi_db();
void cmd_case_srem_multi_handler();
void cmd_case_srem_stress();
/** smembers */
void cmd_case_smembers_without_members();
void cmd_case_smembers_with_members();
void cmd_case_smembers_invalid_cmd();
void cmd_case_smembers_invalid_param();
void cmd_case_smembers_invalid_type();
void cmd_case_smembers_multi_handler();
void cmd_case_smembers_stress();
/** expire */
void case_cmd_expire();
void case_cmd_expire_get();
void case_cmd_expire_list();
void case_cmd_expire_set();
void case_cmd_expire_without_key();
void case_cmd_expire_invalid_cmd();
void case_cmd_expire_invalid_param();
void case_cmd_expire_multi_handler();
void case_cmd_expire_negative_param();
void case_cmd_expire_stress();
/** expireat */
void case_cmd_expireat_get();
void case_cmd_expireat_list();
void case_cmd_expireat_set();
void case_cmd_expireat_without_key();
void case_cmd_expireat_invalid_cmd();
void case_cmd_expireat_invalid_param();
void case_cmd_expireat_multi_handler();
void case_cmd_expireat_stress();
/** pexpire */
void case_cmd_pexpire_get();
void case_cmd_pexpire_list();
void case_cmd_pexpire_set();
void case_cmd_pexpire_without_key();
void case_cmd_pexpire_invalid_cmd();
void case_cmd_pexpire_invalid_param();
void case_cmd_pexpire_multi_handler();
void case_cmd_pexpire_stress();
/** pexpireat */
void case_cmd_pexpireat_get();
void case_cmd_pexpireat_list();
void case_cmd_pexpireat_set();
void case_cmd_pexpireat_without_key();
void case_cmd_pexpireat_invalid_cmd();
void case_cmd_pexpireat_invalid_param();
void case_cmd_pexpireat_multi_handler();
void case_cmd_pexpireat_stress();
/** echo */
void case_cmd_echo();
void case_cmd_echo_invalid_param();
void case_cmd_echo_invalid_cmd();
void case_cmd_echo_multi_db();
void case_cmd_echo_multi_handler();
void case_cmd_echo_stress();
/** llen */
void case_cmd_llen();
void case_cmd_llen_invalid_param();
void case_cmd_llen_invalid_type();
void case_cmd_llen_invalid_cmd();
void case_cmd_llen_nil();
void case_cmd_llen_expire();
void case_cmd_llen_multi_handler();
void case_cmd_llen_stress();
void case_cmd_llen_multi_db();
/** type */
void case_cmd_type();
void case_cmd_type_invalid_cmd();
void case_cmd_type_invalid_param();
void case_cmd_type_multi_handler();
/** select */
void case_cmd_select();
void case_cmd_select_invalid_index();
void case_cmd_select_one_handler();
void case_cmd_select_multi_handler();
/** mset */
void case_cmd_mset();
void case_cmd_mset_invalid_param();
void case_cmd_mset_invalid_cmd();
void case_cmd_mset_multi_handler();
void case_cmd_mset_multi_db();
void case_cmd_mset_stress();
/** rpush */
void case_cmd_rpush_add_few_bytes();
void case_cmd_rpush_add_lot_bytes();
void case_cmd_rpush_del();
void case_cmd_rpush_invalid_param();
void case_cmd_rpush_multi_handler();
void case_cmd_rpush_stress();
/** flushall */
void case_cmd_flushall();
/** lpop  */
void case_cmd_lpop();
void case_cmd_lpop_invalid_param();
void case_cmd_lpop_invalid_type();
void case_cmd_lpop_invalid_cmd();
void case_cmd_lpop_nil();
void case_cmd_lpop_expire();
void case_cmd_lpop_multi_db();
void case_cmd_lpop_multi_handler();
void case_cmd_lpop_stress();
/** rpop  */
void case_cmd_rpop();
void case_cmd_rpop_invalid_param();
void case_cmd_rpop_invalid_type();
void case_cmd_rpop_invalid_cmd();
void case_cmd_rpop_nil();
void case_cmd_rpop_expire();
void case_cmd_rpop_multi_db();
void case_cmd_rpop_multi_handler();
void case_cmd_rpop_stress();
/** hset */
void case_cmd_hset();
void case_cmd_hset_invalid_cmd();
void case_cmd_hset_invalid_param();
void case_cmd_hset_multi_handler();
void case_cmd_hset_stress();
/** hget */
void case_cmd_hget();
void case_cmd_hget_invalid_cmd();
void case_cmd_hget_invalid_param();
void case_cmd_hget_multi_handler();
void case_cmd_hget_stress();
/** hmset */
void case_cmd_hmset();
void case_cmd_hmset_invalid_cmd();
void case_cmd_hmset_invalid_param();
void case_cmd_hmset_multi_handler();
void case_cmd_hmset_multi_db();
void case_cmd_hmset_stress();
/** hmget */
void case_cmd_hmget();
void case_cmd_hmget_invalid_param();
void case_cmd_hmget_invalid_cmd();
void case_cmd_hmget_multi_handler();
void case_cmd_hmget_multi_db();
void case_cmd_hmget_stress();
/** hgetall */
void case_cmd_hgetall();
void case_cmd_hgetall_invalid_cmd();
void case_cmd_hgetall_invalid_param();
void case_cmd_hgetall_multi_db();
void case_cmd_hgetall_multi_handler();
void case_cmd_hgetall_stress();
/** hdel */
void case_cmd_hdel();
void case_cmd_hdel_invalid_param();
void case_cmd_hdel_invalid_cmd();
void case_cmd_hdel_multi_handler();
void case_cmd_hdel_multi_db();
void case_cmd_hdel_stress();
/** sismember */
void case_cmd_sismember();
void case_cmd_sismember_invalid_cmd();
void case_cmd_sismember_invalid_param();
void case_cmd_sismember_invalid_type();
void case_cmd_sismember_multi_handler();
void case_cmd_sismember_db();
/** scard */
void case_cmd_scard();
void case_cmd_scard_invalid_cmd();
void case_cmd_scard_invalid_param();
void case_cmd_scard_invalid_type();
void case_cmd_scard_db();
void case_cmd_scard_multi_handler();
/** srandmember */
void case_cmd_srandmember();
void case_cmd_srandmember_invalid_cmd();
void case_cmd_srandmember_invalid_param();
void case_cmd_srandmember_invalid_type();
void case_cmd_srandmember_db();
void case_cmd_srandmember_multi_handler();






#ifdef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();

        /** version */
        UTIL_ADD_CASE_SHORT(case_version);
        /** init */
        UTIL_ADD_CASE_SHORT(case_init);
        UTIL_ADD_CASE_SHORT(case_init_after_used);
        UTIL_ADD_CASE_SHORT(case_init_stress);
        /** set */
        UTIL_ADD_CASE_SHORT(case_cmd_set_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_set_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_set_stress);
        /** get */
        UTIL_ADD_CASE_SHORT(case_cmd_get);
        UTIL_ADD_CASE_SHORT(case_cmd_get_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_get_nil);
        UTIL_ADD_CASE_SHORT(case_cmd_get_stress);
        /** del */
        UTIL_ADD_CASE_SHORT(case_cmd_del);
        UTIL_ADD_CASE_SHORT(case_cmd_del_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_del_stress);
        /** dbsize */
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize);
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize_expire);
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize_stress);
        /** flushdb */
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb);
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb_stress); 
        /** incr */
        UTIL_ADD_CASE_SHORT(case_cmd_incr);
        UTIL_ADD_CASE_SHORT(case_cmd_incr_multi_handler);
        /** incrby */
        UTIL_ADD_CASE_SHORT(case_cmd_incrby);
        UTIL_ADD_CASE_SHORT(case_cmd_incrby_multi_handler);
        /** decr */
        UTIL_ADD_CASE_SHORT(case_cmd_decr);
        UTIL_ADD_CASE_SHORT(case_cmd_decr_multi_handler);
        /** decrby */
        UTIL_ADD_CASE_SHORT(case_cmd_decrby);
        UTIL_ADD_CASE_SHORT(case_cmd_decrby_multi_handler);
        /** lpush */
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_add_few_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_add_lot_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_del);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_stress);
        /** exists */
        UTIL_ADD_CASE_SHORT(case_cmd_exists_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_stress);
        /** lrange */
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_without_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_lrange_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_stress);
        /** sadd */
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_normal);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_cmd);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_param);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_type);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_multi_handler);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_stress);
        /** srem */
	UTIL_ADD_CASE_SHORT(cmd_case_srem);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_invalid_cmd);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_invalid_param);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_invalid_type);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_multi_db);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_multi_handler);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_stress);
	/** smembers */
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_without_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_with_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_cmd);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_param);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_type);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_multi_handler);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_stress);
        /** expire */
        UTIL_ADD_CASE_SHORT(case_cmd_expire);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_get);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_list);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_set);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_expire_negative_param);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_stress);
        /** expireat */
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_get);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_list);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_set);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_stress);
        /** pexpire */
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_get);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_list);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_set);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_stress);
        /** pexpireat */
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_get);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_list);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_set);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_stress);
	/** echo */
	UTIL_ADD_CASE_SHORT(case_cmd_echo);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_stress);        
	/** llen */	
	UTIL_ADD_CASE_SHORT(case_cmd_llen);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_invalid_type);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_nil);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_expire);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_stress);
  	UTIL_ADD_CASE_SHORT(case_cmd_llen_multi_db);
        /** type */
        UTIL_ADD_CASE_SHORT(case_cmd_type);
        UTIL_ADD_CASE_SHORT(case_cmd_type_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_type_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_type_multi_handler);
        /** select */
        UTIL_ADD_CASE_SHORT(case_cmd_select);
        UTIL_ADD_CASE_SHORT(case_cmd_select_invalid_index);
        UTIL_ADD_CASE_SHORT(case_cmd_select_one_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_select_multi_handler);
	/** mset */
 	UTIL_ADD_CASE_SHORT(case_cmd_mset);
 	UTIL_ADD_CASE_SHORT(case_cmd_mset_invalid_param);
 	UTIL_ADD_CASE_SHORT(case_cmd_mset_invalid_cmd);
 	UTIL_ADD_CASE_SHORT(case_cmd_mset_multi_handler);
 	UTIL_ADD_CASE_SHORT(case_cmd_mset_multi_db);
 	UTIL_ADD_CASE_SHORT(case_cmd_mset_stress);
        /** rpush */
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_add_few_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_add_lot_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_del);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_stress);
        /** flushall */
        UTIL_ADD_CASE_SHORT(case_cmd_flushall);
	/** lpop */
	UTIL_ADD_CASE_SHORT(case_cmd_lpop);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_invalid_type);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_nil);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_expire);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_multi_handler); 
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_stress); 
	/** rpop */
	UTIL_ADD_CASE_SHORT(case_cmd_rpop);
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_invalid_type);
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_nil);
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_expire);
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_multi_handler); 
	UTIL_ADD_CASE_SHORT(case_cmd_rpop_stress);
        /** hset */
        UTIL_ADD_CASE_SHORT(case_cmd_hset);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_stress);
        /** hget */
        UTIL_ADD_CASE_SHORT(case_cmd_hget);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_stress);
	/** hmset */
	UTIL_ADD_CASE_SHORT(case_cmd_hmset);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_stress);
	/** hmget */
	UTIL_ADD_CASE_SHORT(case_cmd_hmget);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_stress);
	/** hgetall */
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_stress);
	/** hdel */
	UTIL_ADD_CASE_SHORT(case_cmd_hdel);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_stress);
        /** sismember */
        UTIL_ADD_CASE_SHORT(case_cmd_sismember);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_db);
        /** scard */
        UTIL_ADD_CASE_SHORT(case_cmd_scard);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_db);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_multi_handler);
        /** srandmember */
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_db);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_multi_handler);

        
	UTIL_RUN();
        UTIL_UNINIT();
  
        return 0;
}
#endif
