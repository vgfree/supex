#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>		/* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <assert.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "load_cfg.h"
#include "zmq_api.h"

#include "ldb.h"
extern struct _leveldb_stuff *g_supex_ldbs;

static bool                     g_loop_flag = true;
static struct loghit_cfg_list   g_loghit_cfg_list = {};
static char                     *g_unique_data = NULL;

int file_fetch(char *name)
{
	FILE *fp;

	fp = fopen(name, "r");

	if (fp) {
		fseek(fp, 0L, SEEK_END);
		long size = ftell(fp);
		g_unique_data = (char *)malloc(size + 1);
		fseek(fp, 0L, SEEK_SET);
		fread(g_unique_data, size, 1, fp);
		g_unique_data[size] = 0;
		fclose(fp);
		return 0;
	}

	return -1;
}

int file_flows(char *name, off_t offset, size_t count)
{
	// TODO send 1.zmq 2.zerosend and async ev
	int     sz = 0;
	int     fd = open(name, O_RDONLY);

	if (fd != -1) {
		void *temp = calloc(sizeof(char), count);
		assert(temp != NULL);
		ssize_t nbyte = pread(fd, temp, count, offset);
		assert(nbyte != -1);

		struct skt_device devc = {};
		zmqmsg_start(&devc, NULL, 3);
		zmqmsg_spill(&devc, g_unique_data, strlen(g_unique_data), false);
		zmqmsg_spill(&devc, name, strlen(name), false);
		zmqmsg_spill(&devc, temp, nbyte, false);
		zmqmsg_flush(&devc);
		zmqmsg_close(&devc);
		free(temp);
		sz = nbyte;
		close(fd);
	}

	return sz;
}

void file_watch(char *name, struct stat *f_stat, struct store_node *node)
{
	bool flag = false;

	// 文件有修改
	if ((node->sn_ino != f_stat->st_ino)
		|| (node->sn_size != f_stat->st_size)
		|| (node->sn_atime != f_stat->st_atime)
		|| (node->sn_mtime != f_stat->st_mtime)
		|| (node->sn_ctime != f_stat->st_ctime)) {
		// 如果文件重新生成了，重置store_node
		if (node->sn_size > f_stat->st_size) {
			memset(node, 0, sizeof(struct store_node));
		}

		node->sn_ino = f_stat->st_ino;
		node->sn_size = f_stat->st_size;
		node->sn_atime = f_stat->st_atime;
		node->sn_mtime = f_stat->st_mtime;
		node->sn_ctime = f_stat->st_ctime;
		flag = true;
	}

	// 但是没有完全读取完毕
	if (node->sn_done != node->sn_size) {
		int     moresz = node->sn_size - node->sn_done;
		int     needsz = (moresz > (MAX_TRANSIT_SIZE)) ? (MAX_TRANSIT_SIZE) : moresz;
		int     size = file_flows(name, node->sn_done, needsz);

		if (size > 0) {
			node->sn_done += size;
			flag = true;
			// printf("\tAdd %d\n", size);
		}
	}

	if (flag == true) {
		// 重置store_node
		int ok = ldb_push(g_supex_ldbs, name, strlen(name), (const char *)node, sizeof(struct store_node));

		if (0 != ok) {
			x_printf(E, "ldb_push failed!\n");
		}
	}
}

static void signal_callback(int sig)
{
	switch (sig)
	{
		case SIGINT:
		case SIGTERM:
		case SIGUSR1:
			g_loop_flag = false;
			break;

		default:
			break;
	}
}

static void register_signals()
{
	signal(SIGPIPE, SIG_IGN);

	signal(SIGINT, signal_callback);	// Ctrl-C
	signal(SIGTERM, signal_callback);	// kill
	signal(SIGUSR1, signal_callback);	// user defined signal.
}

// exe shell clean log
void *cleanlog(void *args)
{
	struct follow_node      *file_list = NULL;
	char                    *name = NULL;
	char                    *cleanshell = "cleanlog.sh";
	char                    shell_cmd_str[1024] = { 0 };

	while (true) {
		for (file_list = g_loghit_cfg_list.file_info.list; file_list; file_list = file_list->next) {
			name = file_list->name;
			struct stat     f_stat = {};
			int             ok = stat(name, &f_stat);

			if (0 == ok) {
				if ((f_stat.st_mode & S_IFMT) == S_IFDIR) {
					snprintf(shell_cmd_str, 1023, "./%s %s", cleanshell, name);

					// printf(">>>>>>>>>>>>>In thread  %s\n",shell_cmd_str);
					/* Check for existence */
					if ((access(cleanshell, 0)) != -1) {
						// printf( "File cleanlog.sh exists\n" );
						/* Check for write permission */
						if ((access(cleanshell, 1)) != -1) {
							// printf( "File cleanlog.sh has exe permission\n" );
							system(shell_cmd_str);
						}
					}
				}
			}
		}

		sleep(86400);
	}

	return (void *)0;
}

int main(int argc, char **argv)
{
	/* register signals. */
	register_signals();

	load_loghit_cfg_argv(&g_loghit_cfg_list.argv_info, argc, argv);
	load_loghit_cfg_file(&g_loghit_cfg_list.file_info, g_loghit_cfg_list.argv_info.conf_name);

	file_fetch(g_loghit_cfg_list.file_info.unique_file);

	zmqskt_init(g_loghit_cfg_list.file_info.host, g_loghit_cfg_list.file_info.port);

	// 初始化本地缓存
	char loghitDB_path_str[256] = { 0 };
	snprintf(loghitDB_path_str, 255, "%s/%s", g_loghit_cfg_list.file_info.loghitDB_path, HIT_LDB_NAME);

	g_supex_ldbs = ldb_initialize(loghitDB_path_str, HIT_LDB_BLK_SIZE,
			HIT_LDB_WB_SIZE, HIT_LDB_LRU_SIZE, HIT_LDB_BLOOM_SIZE);
	assert(g_supex_ldbs);

	// create thread for clean log
	pthread_t       tid;
	pthread_attr_t  attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&tid, &attr, cleanlog, NULL) != 0) {
		fprintf(stderr, "pthread_create failed\n");
		return -1;
	}

	// continue main
	struct store_node       f_node = {};
	char                    f_name[MAX_ABSOLUTE_PATH] = {};

	struct follow_node      *temp = NULL;
	struct dirent           *dp = NULL;
	DIR                     *dir = NULL;
	char                    shell_cmd_str[256] = { 0 };
	snprintf(shell_cmd_str, 255, "usleep %d", g_loghit_cfg_list.file_info.space_usleep);

	while (g_loop_flag) {
		system(shell_cmd_str);

		for (temp = g_loghit_cfg_list.file_info.list; temp; temp = temp->next) {
			usleep(USLEEP_TIME_APART);
			char *name = temp->name;

			struct stat     f_stat = {};
			int             ok = stat(name, &f_stat);

			// printf("-------------------\nok = %d\n", ok);
			if (0 == ok) {
				switch (f_stat.st_mode & S_IFMT)
				{
					case S_IFDIR:
						// printf("directory\n");

						dir = opendir(name);

						// 遍历目录下每个文件
						while ((dp = readdir(dir)) != NULL) {
							// printf("--------------\n");
							memset(f_name, 0, MAX_ABSOLUTE_PATH);
							snprintf(f_name, MAX_ABSOLUTE_PATH - 1, "%s/%s", name, dp->d_name);
							// printf("file is %s\n", f_name);

							ok = stat(f_name, &f_stat);

							// printf("ok = %d\n", ok);
							if (0 == ok) {
								if (!S_ISREG(f_stat.st_mode)) {
									continue;
								}

								// 取上次操作后文件信息
								memset(&f_node, 0, sizeof(struct store_node));
								ok = ldb_pull(g_supex_ldbs, f_name, strlen(f_name), (char *)&f_node, sizeof(struct store_node));
								file_watch(f_name, &f_stat, &f_node);	// FIXME
								// printf("file is %s\n", f_name);
							}
						}

						closedir(dir);
						break;

					case S_IFREG:
						// printf("regular file\n");

						memset(&f_node, 0, sizeof(struct store_node));
						ok = ldb_pull(g_supex_ldbs, name, strlen(name), (char *)&f_node, sizeof(struct store_node));
						file_watch(name, &f_stat, &f_node);
						break;

					default:
						// printf("other file\n");
						break;
				}
			}
		}
	}

	zmqskt_exit();
	return 0;
}

