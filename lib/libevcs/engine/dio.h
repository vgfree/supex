#pragma once

#include <unistd.h>
#include <stdbool.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <utime.h>

struct dio_req; 
typedef int (*dio_cb)(struct dio_req *req);
  
struct dio_req {
	ssize_t result;  /* result of syscall, e.g. result = read (... */
	off_t offs;      /* read, write, truncate, readahead, sync_file_range, fallocate: file offset, mknod: dev_t */
	size_t size;     /* read, write, readahead, sendfile, msync, mlock, sync_file_range, fallocate: length */
	void *ptr1;      /* all applicable requests: pathname, old name, readdir: optional dio_dirents */
	void *ptr2;      /* all applicable requests: new name or memory buffer; readdir: name strings */

	int int1;        /* all applicable requests: file descriptor; sendfile: output fd; open, msync, mlockall, readdir: flags */
	long int2;       /* chown, fchown: uid; sendfile: input fd; open, chmod, mkdir, mknod: file mode, seek: whence, fcntl, ioctl: request, sync_file_range, fallocate: flags */
	long int3;       /* chown, fchown: gid; rename, link: working directory of new name */
	int errorno;     /* errno value on syscall return */

	signed char type;/* DIO_xxx constant ETP */
	signed char pri;     /* the priority ETP */

	void *data;
	dio_cb finish;
};


void start_dio_domain(void);

bool dio_open (const char *path, int flags, mode_t mode, int pri, dio_cb cb, void *data);
bool dio_close (int fd, int pri, dio_cb cb, void *data);

bool dio_read (int fd, void *buf, size_t length, int pri, dio_cb cb, void *data);
bool dio_pread (int fd, void *buf, size_t length, off_t offset, int pri, dio_cb cb, void *data);

bool dio_write (int fd, void *buf, size_t length, int pri, dio_cb cb, void *data);
bool dio_pwrite (int fd, void *buf, size_t length, off_t offset, int pri, dio_cb cb, void *data);



bool dio_sync (int pri, dio_cb cb, void *data);

bool dio_fsync (int fd, int pri, dio_cb cb, void *data);

bool dio_fdatasync (int fd, int pri, dio_cb cb, void *data);

bool dio_syncfs (int fd, int pri, dio_cb cb, void *data);

bool dio_msync (void *addr, size_t length, int flags, int pri, dio_cb cb, void *data);

bool dio_mlock (void *addr, size_t length, int pri, dio_cb cb, void *data);

bool dio_mlockall (int flags, int pri, dio_cb cb, void *data);

bool dio_sync_file_range (int fd, off_t offset, size_t nbytes, unsigned int flags, int pri, dio_cb cb, void *data);

bool dio_fallocate (int fd, int mode, off_t offset, size_t len, int pri, dio_cb cb, void *data);

bool dio_readahead (int fd, off_t offset, size_t length, int pri, dio_cb cb, void *data);

bool dio_lseek (int fd, off_t offset, int whence, int pri, dio_cb cb, void *data);

bool dio_fstat (int fd, struct stat *buf, int pri, dio_cb cb, void *data);

bool dio_fstatvfs (int fd, struct statvfs *buf, int pri, dio_cb cb, void *data);

bool dio_ftruncate (int fd, off_t offset, int pri, dio_cb cb, void *data);

bool dio_fchmod (int fd, mode_t mode, int pri, dio_cb cb, void *data);

bool dio_fchown (int fd, uid_t uid, gid_t gid, int pri, dio_cb cb, void *data);

bool dio_dup2 (int fd, int fd2, int pri, dio_cb cb, void *data);

bool dio_utime (const char *path, const struct utimbuf *times, int pri, dio_cb cb, void *data);

bool dio_truncate (const char *path, off_t offset, int pri, dio_cb cb, void *data);

bool dio_chown (const char *path, uid_t uid, gid_t gid, int pri, dio_cb cb, void *data);

bool dio_chmod (const char *path, mode_t mode, int pri, dio_cb cb, void *data);

bool dio_mkdir (const char *path, mode_t mode, int pri, dio_cb cb, void *data);

bool dio_rmdir (const char *path, int pri, dio_cb cb, void *data);

bool dio_unlink (const char *path, int pri, dio_cb cb, void *data);

bool dio_readlink (const char *path, char *buf, size_t bufsiz, int pri, dio_cb cb, void *data);

bool dio_stat (const char *path, struct stat *buf, int pri, dio_cb cb, void *data);

bool dio_lstat (const char *path, struct stat *buf, int pri, dio_cb cb, void *data);

bool dio_statvfs (const char *path, struct statvfs *buf, int pri, dio_cb cb, void *data);

bool dio_mknod (const char *path, mode_t mode, dev_t dev, int pri, dio_cb cb, void *data);

bool dio_link (const char *path, const char *new_path, int pri, dio_cb cb, void *data);

bool dio_symlink (const char *path, const char *new_path, int pri, dio_cb cb, void *data);

bool dio_rename (const char *path, const char *new_path, int pri, dio_cb cb, void *data);
