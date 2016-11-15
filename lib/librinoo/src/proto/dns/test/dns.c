/**
 * @file   dns.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Feb 25 14:48:20 2013
 *
 * @brief  DNS unit test
 *
 *
 */

#include "rinoo/rinoo.h"

void dns_test(void *arg)
{
	t_ip ip;

	XTEST(rinoo_dns_ip_get(arg, "google.com", &ip) == 0);
	rinoo_log("IP: %s", inet_ntoa(ip.v4.sin_addr));
}

/**
 * Main function for this unit test.
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_sched *sched;

	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_start(sched, dns_test, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XPASS();
}
