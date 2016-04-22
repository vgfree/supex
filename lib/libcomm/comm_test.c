
static bool _write_event(struct comm_data *commdata, int fd)
{
	assert(commctx && fda);
	int bytes = 0;
	int flag = false;
	int size = commdata->send_buff.size;
	int n = 0;
	struct comm_data *commdata = NULL;
	log("_write_event fd:%d\n", fd);
	while (1) {
		//bytes = write(fd, &commdata->send_buff.cache[commdata->send_buff.start], commdata->send_buff.size);
		bytes = write(fd, &commdata->send_buff.cache[commdata->send_buff.start], COMM_WRITE_MIOU);
		if (unlikely(bytes < 0)) {
			log("write failed\n");
			if (likely(errno == EAGAIN || errno == EWOULDBLOCK)) {
				/* 写缓冲区列队已满 */
				flag = false ;
				_package_data(commdata);
				break ; 
			} else if (likely(errno == EINTR)) {
				/* 写操作被信号中断 可继续写 */
				continue ;
			} else {
				/* 其他错误，退出 */
				flag = false;
				break ;
			}

		} else {
			log("write successed\n");
			/* 数据成功发送完成 */
			flag = true;
			commdata->send_buff.start += bytes;
			commdata->send_buff.size -= bytes;
			size -= bytes;
			if (size == bytes) {
				/* 发送完毕，退出循环 */
				log("write successed and ready to break ");
				commcache_clean(&commdata->send_buff); /* 待定中 */
				/* 接收完数据就打包 */
				_package_data(commdata);
				log("write successed and break ");
				break ;
			}
			log("write successed but can't break while(1)");
		}
	}
	if (commdata->finishedcb.callback) {
		commdata->finishedcb.callback(commdata->commctx, fd, FD_WRITE, commdata->finishedcb.usr);
	}

	return flag;
}


/*关闭父进程所有打开的文件描述符， pfd:此描述符不关闭*/
static bool  _close_all_fd(int pfd)
{
	int		retval = 0;
	int		rewind = 0;
	int		fd = 0;
	DIR*		dir = NULL;
	struct dirent	*entry, _entry;

	if (unlikely(!(dir = opendir("/dev/fd")))) {
		return false;
	}

	while (1) {

		retval = readdir_r(dir, &_entry, &entry);
		if (unlikely(retval != 0)) {
			closedir(dir);
			return false;
		}
		if (entry == NULL) {
			if (!rewind) {
				break;
			}
			rewinddir(dir);
			rewind = 0;
			continue;
		}
		if (entry->d_name[0] == '.') {
			continue;
		}
		fd = atoi(entry->d_name);
		if (unlikely(dirfd(dir) == fd || fd == 1 || fd == 0 ||fd == 2 || fd == pfd)) {
			continue;
		}
		close(fd);
		rewind = 1;
	}
	closedir(dir);
	return true;
}
