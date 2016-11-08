#pragma once

/**
 * @return Redis protocol string, it's your responsibility to free it when needed.
 *         NULL if invalid command(paramter) passed.
 */
int cmd_to_proto(char **cmd, const char *fmt, ...);

