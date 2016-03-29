#include "jtt_package.h"
#include "pkg_sec.h"
#include "pkg_crc.h"
#include "pkg_escape.h"
#include "log.h"

int pkg_encode(PKG_USER *user, unsigned char *pkg, int *plen, PKG_BODY body)
{
	JTT_HEAD        big_head = {};
	unsigned char   *tmp_pkg = NULL;
	int             i = 0;
	int             j = 0;
	int             k = 0;
	int             n = 0;
	unsigned short  crc = 0;
	unsigned        key = 0;

	key = sec_key_get();

	big_head.head_flag = user->head_flag;
	big_head.msg_length = htonl(sizeof(big_head) + body.blen + 3);
	big_head.msg_sz = htonl(user->msg_sz++);
	big_head.msg_id = htons(body.msg_id);
	big_head.msg_enterid = htonl(user->msg_enterid);
	big_head.encrypt = user->encrypt;
	big_head.key = htonl(key);
	memcpy(big_head.ver, user->version, 3);

	i = sizeof(JTT_HEAD);
	tmp_pkg = (unsigned char *)calloc(1, sizeof(big_head) + body.blen + 3);

	if (!tmp_pkg) {
		log_info(LOG_E, "内存分配错\n");
		return -1;
	}

	memcpy(tmp_pkg, &big_head, i);

	if (body.body && (body.blen > 0)) {
		memcpy(tmp_pkg + i, body.body, body.blen);

		if (big_head.encrypt == 1) {
			sec_encrypt(key, (unsigned char *)(tmp_pkg + i), body.blen);
		}

		i += body.blen;
	}

	crc = crc_get(tmp_pkg + 1, i - 1);
	tmp_pkg[i++] = (char)(crc >> 8);
	tmp_pkg[i++] = (char)crc;

	tmp_pkg[i] = user->end_flag;

	pkg[0] = tmp_pkg[0];

	for (j = 1, k = 1; j < i; j++) {
		n = escape(pkg + k, tmp_pkg[j]);

		if (k + n >= *plen) {
			log_info(LOG_E, "转义后数据超长\n");
			free(tmp_pkg);
			return -1;
		}

		k += n;
	}

	pkg[k] = tmp_pkg[i];

	*plen = k + 1;

	free(tmp_pkg);
	return 0;
}

int pkg_decode(PKG_USER user, unsigned char *pkg, int *pkg_len)
{
	int             len = 1;
	unsigned short  crc;
	unsigned short  org_crc;
	JTT_HEAD        *head = NULL;
	unsigned        key;

	if ((pkg[0] != user.head_flag) || (pkg[*pkg_len - 1] != user.end_flag)) {
		log_info(LOG_E, "数据包格式错误head[%c],end[%c]\n", pkg[0], pkg[*pkg_len - 1]);
		return -1;
	}

	len = 1;
	len += unescape(pkg + 1, pkg + 1, *pkg_len - 2);
	pkg[len++] = pkg[*pkg_len - 1];
	*pkg_len = len;

	crc = crc_get(pkg + 1, len - 4);
	org_crc = *((unsigned short *)(pkg + len - 3));
	org_crc = htons(org_crc);

	if (crc != org_crc) {
		log_info(LOG_E, "校验错误\n");
		return -1;
	}

	head = (JTT_HEAD *)pkg;

	if (head->encrypt == 1) {
		key = htonl(head->key);
		sec_decrypt(key, pkg + sizeof(JTT_HEAD), len - sizeof(JTT_HEAD) - 3);
	}

	return 0;
}

