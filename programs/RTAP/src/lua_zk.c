#include "lua_zk.h"

#include "utils.h"
#include "smart_api.h"
#include "smart_evcb.h"

static zk_ctx_t *s_zc = NULL;

static int _free_dn(lua_State **L, int last, struct smart_task_node *task, long S)
{
	char            host[128] = { 0 };
	data_node_t     *dn = NULL;

	dn = (data_node_t *)task->data;

	if (NULL == dn) {
		x_printf(E, "dn == NULL");
		return -1;
	}

	snprintf(host, sizeof(host), "%s:%d", dn->ip, (int)dn->r_port);

	lua_getglobal(*L, "del_dn_from_pool");
	lua_pushstring(*L, host);
	return lua_pcall(*L, 1, 0, 0);
}

static int free_dn_task(void *user, void *task, int step)
{
	return smart_for_batch_vm(user, task, step, _free_dn);
}

static void free_dn_hook(void *ctx, data_node_t *dn)
{
	struct smart_task_node task = {
		.id     = 0,
		.sfd    = 0,
		.type   = BIT8_TASK_TYPE_WHOLE,
		.origin = BIT8_TASK_ORIGIN_TIME,
		.func   = (SUPEX_TASK_CALLBACK)free_dn_task,
		.index  = 0,
		.data   = (void *)dn
	};

	smart_all_task_hit(&task, true, 0);
}

int zk_init(char *zk_host, const char *zk_rnode)
{
	if (NULL != s_zc) {
		return 0;
	}

	s_zc = open_zkhandler(zk_host, NULL, (free_dn_hook_f)free_dn_hook, NULL);

	if (NULL == s_zc) {
		x_printf(F, "open_zkhandler failed!");
		return -1;
	}

	if (register_zkcache(s_zc, zk_rnode) < 0) {
		x_printf(F, "register_zkcache failed!");
		close_zkhandler(s_zc);
		s_zc = NULL;
		return -1;
	}

	return 0;
}

void zk_close(void)
{
	if (NULL == s_zc) {
		return;
	}

	unregister_zkcache(s_zc);
	close_zkhandler(s_zc);
	s_zc = NULL;
}

int zk_get_read_dn(lua_State *L)
{
	data_set_t      *ds = NULL;
	data_node_t     *dn = NULL;

	uint64_t        key = luaL_checkinteger(L, 1);
	uint64_t        tmstamp = luaL_checkinteger(L, 2);

	if (NULL == s_zc) {
		lua_pushboolean(L, 0);
		return 1;
	}

	if (get_read_dataset(s_zc, key, tmstamp, (const data_set_t **)&ds) < 0) {
		lua_pushboolean(L, 0);
		return 1;
	}

	dn = &(ds->data_node[(ds->dn_idx++) % ds->dn_cnt]);

	lua_pushboolean(L, 1);
	lua_pushstring(L, dn->ip);
	lua_pushinteger(L, dn->r_port);
	return 3;
}

