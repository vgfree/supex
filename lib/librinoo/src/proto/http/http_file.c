/**
 * @file   http_file.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Feb 16 23:36:42 2011
 *
 * @brief  HTTP Server file sending
 *
 *
 */

#include "rinoo/proto/http/module.h"

int rinoo_http_send_dir(t_http *http, const char *path)
{
	int ret;
	int flag;
	DIR *dir;
	char *hl;
	char *de;
	t_buffer *result;
	struct stat stats;
	struct dirent *curentry;

	XASSERT(http != NULL, -1);
	XASSERT(path != NULL, -1);

	if (stat(path, &stats) != 0) {
		return -1;
	}
	if (S_ISDIR(stats.st_mode) == 0) {
		return -1;
	}
	dir = opendir(path);
	if (dir == NULL) {
		return -1;
	}
	result = buffer_create(NULL);
	if (result == NULL) {
		closedir(dir);
		errno = ENOMEM;
		return -1;
	}
	buffer_print(result,
		     "<html>\n"
		     "  <head>\n"
		     "    <title>Directory listing</title>\n"
		     "    <style>\n"
		     "      body { font-family: Monospace; color: #666; font-size: 14px; }\n"
		     "      #shadow { width: 800px; border-radius: 100% / 33px; box-shadow: 0 8px 3px -5px rgba(0, 0, 0, .2); margin: 20px auto; }\n"
		     "      #dirlist { border-radius: 5px; background: -webkit-linear-gradient(top, rgba(249, 249, 249, 1) 50%, rgba(240, 240, 240, 1) 100%); box-shadow: 0 -3px 1px rgba(255, 255, 255, .6), inset 0 1px 1px rgba(255, 255, 255, .97), 0 0 3px 0px rgba(0, 0, 0, .7); padding: 0px; }\n"
		     "      #dirlist ul { padding: 10px; margin: 0; list-style: none; }\n"
		     "      #dirlist li { padding: 0; margin: 0; }\n"
		     "      #dirlist li a { display: block; padding: 2px 10px; height: 15px; border-radius: 5px;  margin: 3px 0; }\n"
		     "      a { margin: 0px; text-decoration: none; color: #666; }\n"
		     "      .dl_title { padding: 10px; border-bottom: 1px solid #eee; box-shadow: 0 1px 1px rgba(255, 255, 255, .6); text-shadow: 0 1px 1px rgba(255, 255, 255, .6); font-size: 16px; font-weight: bold; }\n"
		     "      .dl_en { float: left; width: 500px; }\n"
		     "      .dl_ed { float: left; width: 130px; text-align: center; }\n"
		     "      .dl_es { float: left; width: 120px; text-align: right; }\n"
		     "      .dl_hl { box-shadow: 0 0px 2px 0 rgba(0, 0, 0, .2); background-color: #fff; }\n"
		     "    </style>\n"
		     "  </head>\n"
		     "  <body>\n"
		     "    <div id=\"shadow\">\n"
		     "      <div id=\"dirlist\">\n"
		     "        <div class=\"dl_title\">Directory listing</div>\n"
		     "        <ul>\n"
		     "<li>\n"
		     "  <a href=\"../\" class=\"dl_hl\">\n"
		     "    <div class=\"dl_en\">../</div>\n"
		     "    <div class=\"dl_ed\">-</div>\n"
		     "    <div class=\"dl_es\">-</div>\n"
		     "  </a>\n"
		     "</li>\n");
	flag = 1;
	while ((curentry = readdir(dir)) != NULL) {
		if (strcmp(curentry->d_name, ".") != 0 && strcmp(curentry->d_name, "..") != 0) {
			if (flag == 0) {
				hl = " class=\"dl_hl\"";
			} else {
				hl = "";
			}
			if (curentry->d_type == DT_DIR) {
				de = "/";
			} else {
				de = "";
			}
			buffer_print(result,
				     "<li>\n"
				     "  <a href=\"%s%s\"%s>\n"
				     "    <div class=\"dl_en\">%s%s</div>\n"
				     "    <div class=\"dl_ed\">-</div>\n"
				     "    <div class=\"dl_es\">-</div>\n"
				     "  </a>\n"
				     "</li>\n",
				     curentry->d_name,
				     de,
				     hl,
				     curentry->d_name,
				     de);
			flag = !flag;
		}
	}
	buffer_print(result,
		     "        </ul>\n"
		     "      </div>\n"
		     "    </div>\n"
		     "  </body>\n"
		     "</html>\n");
	closedir(dir);

	http->response.code = 200;
	ret = rinoo_http_response_send(http, result);
	buffer_destroy(result);
	return ret;
}

int rinoo_http_send_file(t_http *http, const char *path)
{
	int ret;
	int fd;
	void *ptr;
	t_buffer dummy;
	struct stat stats;

	XASSERT(http != NULL, -1);
	XASSERT(path != NULL, -1);

	if (stat(path, &stats) != 0) {
		return -1;
	}
	if (S_ISDIR(stats.st_mode)) {
		return rinoo_http_send_dir(http, path);
	}
	if (S_ISREG(stats.st_mode) == 0) {
		return -1;
	}
	if (stats.st_size == 0) {
		http->response.code = 200;
		return rinoo_http_response_send(http, NULL);
	}
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		return -1;
	}
	ptr = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) {
		ret = errno;
		close(fd);
		errno = ret;
		return -1;
	}
	http->response.code = 200;
	buffer_static(&dummy, ptr, stats.st_size);
	ret = rinoo_http_response_send(http, &dummy);
	munmap(ptr, stats.st_size);
	close(fd);
	return ret;
}
