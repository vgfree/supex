#include <string.h>
#include <stdlib.h>

#include "libevcs.h"
#include "redis_parse.h"
#include <ctype.h>

/* Calculate the number of bytes needed to represent an integer as string. */
static int intlen(int i)
{
	int len = 0;

	if (i < 0) {
		len++;
		i = -i;
	}

	do {
		len++;
		i /= 10;
	} while (i);
	return len;
}

/* Helper that calculates the bulk length given a certain string length. */
static size_t bulklen(size_t len)
{
	return 1 + intlen(len) + 2 + len + 2;
}

static int _cmd_to_proto(char **target, const char *format, va_list ap)
{
	const char      *c = format;
	char            *cmd = NULL;	/* final command */
	int             pos;		/* position in final command */
	sstr            curarg, newarg;
	int             touched = 0;	/* was the current argument touched? */
	char            **curargv = NULL, **newargv = NULL;
	int             argc = 0;
	int             totlen = 0;
	int             j;

	/* Abort if there is not target to set */
	if (target == NULL) {
		return -1;
	}

	/* Build the command string accordingly to protocol */
	curarg = sstr_empty();

	if (curarg == NULL) {
		return -1;
	}

	while (*c != '\0') {
		if ((*c != '%') || (c[1] == '\0')) {
			if (*c == ' ') {
				if (touched) {
					newargv = realloc(curargv, sizeof(char *) * (argc + 1));

					if (newargv == NULL) {
						goto err;
					}

					curargv = newargv;
					curargv[argc++] = curarg;
					totlen += bulklen(sstr_len(curarg));

					/* curarg is put in argv so it can be overwritten. */
					curarg = sstr_empty();

					if (curarg == NULL) {
						goto err;
					}

					touched = 0;
				}
			} else {
				newarg = sstr_catlen(curarg, c, 1);

				if (newarg == NULL) {
					goto err;
				}

				curarg = newarg;
				touched = 1;
			}
		} else {
			char    *arg;
			size_t  size;

			/* Set newarg so it can be checked even if it is not touched. */
			newarg = curarg;

			switch (c[1])
			{
				case 's':
					arg = va_arg(ap, char *);
					size = strlen(arg);

					if (size > 0) {
						newarg = sstr_catlen(curarg, arg, size);
					}

					break;

				case 'b':
					arg = va_arg(ap, char *);
					size = va_arg(ap, size_t);

					if (size > 0) {
						newarg = sstr_catlen(curarg, arg, size);
					}

					break;

				case '%':
					newarg = sstr_cat(curarg, "%");
					break;

				default:
					/* Try to detect printf format */
				{
					static const char       intfmts[] = "diouxX";
					char                    _format[16];
					const char              *_p = c + 1;
					size_t                  _l = 0;
					va_list                 _cpy;

					/* Flags */
					if ((*_p != '\0') && (*_p == '#')) {
						_p++;
					}

					if ((*_p != '\0') && (*_p == '0')) {
						_p++;
					}

					if ((*_p != '\0') && (*_p == '-')) {
						_p++;
					}

					if ((*_p != '\0') && (*_p == ' ')) {
						_p++;
					}

					if ((*_p != '\0') && (*_p == '+')) {
						_p++;
					}

					/* Field width */
					while (*_p != '\0' && isdigit(*_p)) {
						_p++;
					}

					/* Precision */
					if (*_p == '.') {
						_p++;

						while (*_p != '\0' && isdigit(*_p)) {
							_p++;
						}
					}

					/* Copy va_list before consuming with va_arg */
					va_copy(_cpy, ap);

					/* Integer conversion (without modifiers) */
					if (strchr(intfmts, *_p) != NULL) {
						va_arg(ap, int);
						goto fmt_valid;
					}

					/* Double conversion (without modifiers) */
					if (strchr("eEfFgGaA", *_p) != NULL) {
						va_arg(ap, double);
						goto fmt_valid;
					}

					/* Size: char */
					if ((_p[0] == 'h') && (_p[1] == 'h')) {
						_p += 2;

						if ((*_p != '\0') && (strchr(intfmts, *_p) != NULL)) {
							va_arg(ap, int);/* char gets promoted to int */
							goto fmt_valid;
						}

						goto fmt_invalid;
					}

					/* Size: short */
					if (_p[0] == 'h') {
						_p += 1;

						if ((*_p != '\0') && (strchr(intfmts, *_p) != NULL)) {
							va_arg(ap, int);/* short gets promoted to int */
							goto fmt_valid;
						}

						goto fmt_invalid;
					}

					/* Size: long long */
					if ((_p[0] == 'l') && (_p[1] == 'l')) {
						_p += 2;

						if ((*_p != '\0') && (strchr(intfmts, *_p) != NULL)) {
							va_arg(ap, long long);
							goto fmt_valid;
						}

						goto fmt_invalid;
					}

					/* Size: long */
					if (_p[0] == 'l') {
						_p += 1;

						if ((*_p != '\0') && (strchr(intfmts, *_p) != NULL)) {
							va_arg(ap, long);
							goto fmt_valid;
						}

						goto fmt_invalid;
					}

fmt_invalid:
					va_end(_cpy);
					goto err;

fmt_valid:
					_l = (_p + 1) - c;

					if (_l < sizeof(_format) - 2) {
						memcpy(_format, c, _l);
						_format[_l] = '\0';
						newarg = sstr_catvprintf(curarg, _format, _cpy);

						/* Update current position (note: outer blocks
						 * increment c twice so compensate here) */
						c = _p - 1;
					}

					va_end(_cpy);
					break;
				}
			}

			if (newarg == NULL) {
				goto err;
			}

			curarg = newarg;

			touched = 1;
			c++;
		}

		c++;
	}

	/* Add the last argument if needed */
	if (touched) {
		newargv = realloc(curargv, sizeof(char *) * (argc + 1));

		if (newargv == NULL) {
			goto err;
		}

		curargv = newargv;
		curargv[argc++] = curarg;
		totlen += bulklen(sstr_len(curarg));
	} else {
		sstr_free(curarg);
	}

	/* Clear curarg because it was put in curargv or was free'd. */
	curarg = NULL;

	/* Add bytes needed to hold multi bulk count */
	totlen += 1 + intlen(argc) + 2;

	/* Build the command at protocol level */
	cmd = malloc(totlen + 1);

	if (cmd == NULL) {
		goto err;
	}

	pos = sprintf(cmd, "*%d\r\n", argc);

	for (j = 0; j < argc; j++) {
		pos += sprintf(cmd + pos, "$%zu\r\n", sstr_len(curargv[j]));
		memcpy(cmd + pos, curargv[j], sstr_len(curargv[j]));
		pos += sstr_len(curargv[j]);
		sstr_free(curargv[j]);
		cmd[pos++] = '\r';
		cmd[pos++] = '\n';
	}

	assert(pos == totlen);
	cmd[pos] = '\0';

	free(curargv);
	*target = cmd;
	return totlen;

err:

	while (argc--) {
		sstr_free(curargv[argc]);
	}

	free(curargv);

	if (curarg != NULL) {
		sstr_free(curarg);
	}

	/* No need to check cmd since it is the last statement that can fail,
	 * but do it anyway to be as defensive as possible. */
	if (cmd != NULL) {
		free(cmd);
	}

	return -1;
}

/**
 * @param  proto out parameter holds protocol string, it's your responsibility to free it.
 * @return -1 if failed
 *         >0 indicates protocol bytes count converted.
 */
int cmd_to_proto(char **proto, const char *fmt, ...)
{
	int     ret;
	va_list ap;

	va_start(ap, fmt);
	ret = _cmd_to_proto(proto, fmt, ap);
	va_end(ap);

	return ret;
}

