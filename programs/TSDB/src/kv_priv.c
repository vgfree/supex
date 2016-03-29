#include "libkv.h"
#include "kv_inner.h"

extern kv_answer_t *kv_answer_create(struct kv_handler *handler);

extern reply_t *reply_create();

extern void reply_free(reply_t *c);

extern int reply_start(struct kv_handler *handler);

extern void reply_to_answer(reply_t *c, kv_answer_t *ans);

static int reply_attach_proto(reply_t *c, const char *proto, unsigned int protolen)
{
	sdsclear(c->proto_text);
	c->proto_text = sdsnewlen(proto, protolen);

	return 0;
}

static kv_answer_t *kv_executor_proto(struct kv_handler *handler, const char *proto, unsigned int protolen)
{
	kv_answer_t *ans = NULL;

	handler_mutex_lock(handler);	/** start reply mutex lock */

	ans = kv_answer_create(handler);
	handler->reply = reply_create();
	selectDb(handler, handler->dbindex);

	reply_attach_proto(handler->reply, proto, protolen);

	reply_start(handler);
	reply_to_answer(handler->reply, ans);
	reply_free(handler->reply);

	handler_mutex_unlock(handler);
	return ans;
}

kv_answer_t *kv_ask_proto(struct kv_handler *handler, const char *proto, unsigned int protolen)
{
	if (!(handler->state & STATE_IDLE)) {
		return kv_answer_create(handler);
	} else if (!proto || (protolen == 0)) {	/* invalid parameter passed */
		kv_answer_t *a = kv_answer_create(handler);
		a->errnum = ERR_ARGUMENTS;
		a->err = get_err(ERR_ARGUMENTS);
		return a;
	}

	return kv_executor_proto(handler, proto, protolen);
}

