#ifndef daemon_h
#define daemon_h

int daemon_init(const char *pidfile);

int daemon_exit(const char *pidfile);
#endif

