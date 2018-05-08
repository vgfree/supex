#pragma once

#include <utime.h>
#include <sys/statvfs.h>

#include "libevcoro.h"

#ifndef off64_t
typedef __off64_t off64_t;
#endif

//#define USE_LIBEIO

#ifdef USE_LIBEIO
void create_eio_domain(void);
#else
void create_dio_domain(void);
#endif

int evcoro_dskio_open(struct evcoro_scheduler *scheduler, const char *pathname, int flags, mode_t mode);

int evcoro_dskio_close(struct evcoro_scheduler *scheduler, int fd);

ssize_t evcoro_dskio_read(struct evcoro_scheduler *scheduler, int fd, void *buf, size_t count);

ssize_t evcoro_dskio_pread(struct evcoro_scheduler *scheduler, int fd, void *buf, size_t count, off_t offset);

ssize_t evcoro_dskio_write(struct evcoro_scheduler *scheduler, int fd, const void *buf, size_t count);

ssize_t evcoro_dskio_pwrite(struct evcoro_scheduler *scheduler, int fd, const void *buf, size_t count, off_t offset);




void evcoro_dskio_sync(struct evcoro_scheduler *scheduler);

int evcoro_dskio_fsync(struct evcoro_scheduler *scheduler, int fd);

int evcoro_dskio_fdatasync(struct evcoro_scheduler *scheduler, int fd);

int evcoro_dskio_syncfs(struct evcoro_scheduler *scheduler, int fd);

int evcoro_dskio_msync(struct evcoro_scheduler *scheduler, void *addr, size_t length, int flags);

int evcoro_dskio_mlock(struct evcoro_scheduler *scheduler, const void *addr, size_t len);

int evcoro_dskio_mlockall(struct evcoro_scheduler *scheduler, int flags);

int evcoro_dskio_sync_file_range(struct evcoro_scheduler *scheduler, int fd, off64_t offset, off64_t nbytes, unsigned int flags);

int evcoro_dskio_fallocate(struct evcoro_scheduler *scheduler, int fd, int mode, off_t offset, off_t len);

ssize_t evcoro_dskio_readahead(struct evcoro_scheduler *scheduler, int fd, off64_t offset, size_t count);

off_t evcoro_dskio_lseek(struct evcoro_scheduler *scheduler, int fd, off_t offset, int whence);

int evcoro_dskio_fstat(struct evcoro_scheduler *scheduler, int fd, struct stat *buf);

int evcoro_dskio_fstatvfs(struct evcoro_scheduler *scheduler, int fd, struct statvfs *buf);

int evcoro_dskio_ftruncate(struct evcoro_scheduler *scheduler, int fd, off_t length);

int evcoro_dskio_fchmod(struct evcoro_scheduler *scheduler, int fd, mode_t mode);

int evcoro_dskio_fchown(struct evcoro_scheduler *scheduler, int fd, uid_t owner, gid_t group);

int evcoro_dskio_dup2(struct evcoro_scheduler *scheduler, int oldfd, int newfd);

int evcoro_dskio_utime(struct evcoro_scheduler *scheduler, const char *filename, const struct utimbuf *times);

int evcoro_dskio_truncate(struct evcoro_scheduler *scheduler, const char *path, off_t length);

int evcoro_dskio_chown(struct evcoro_scheduler *scheduler, const char *pathname, uid_t owner, gid_t group);

int evcoro_dskio_chmod(struct evcoro_scheduler *scheduler, const char *pathname, mode_t mode);

int evcoro_dskio_mkdir(struct evcoro_scheduler *scheduler, const char *pathname, mode_t mode);

int evcoro_dskio_rmdir(struct evcoro_scheduler *scheduler, const char *pathname);

int evcoro_dskio_unlink(struct evcoro_scheduler *scheduler, const char *pathname);

ssize_t evcoro_dskio_readlink(struct evcoro_scheduler *scheduler, const char *pathname, char *buf, size_t bufsiz);

int evcoro_dskio_stat(struct evcoro_scheduler *scheduler, const char *pathname, struct stat *buf);

int evcoro_dskio_lstat(struct evcoro_scheduler *scheduler, const char *pathname, struct stat *buf);

int evcoro_dskio_statvfs(struct evcoro_scheduler *scheduler, const char *path, struct statvfs *buf);

int evcoro_dskio_mknod(struct evcoro_scheduler *scheduler, const char *pathname, mode_t mode, dev_t dev);

int evcoro_dskio_link(struct evcoro_scheduler *scheduler, const char *oldpath, const char *newpath);

int evcoro_dskio_symlink(struct evcoro_scheduler *scheduler, const char *target, const char *linkpath);

int evcoro_dskio_rename(struct evcoro_scheduler *scheduler, const char *oldpath, const char *newpath);

#if 0
  #define evcoro_dskio_exf(name, ...)	evcoro_dskio_##name(evcoro_get_default_scheduler(), ##__VA_ARGS__)
  #define open(...)		evcoro_dskio_exf(open,		##__VA_ARGS__)
  #define close(...)		evcoro_dskio_exf(close,		##__VA_ARGS__)
  #define pread(...)		evcoro_dskio_exf(pread,		##__VA_ARGS__)
  #define read(...)		evcoro_dskio_exf(read,		##__VA_ARGS__)
  #define pwrite(...)		evcoro_dskio_exf(pwrite,	##__VA_ARGS__)
  #define write(...)		evcoro_dskio_exf(write,		##__VA_ARGS__)
  #define sync(...)		evcoro_dskio_exf(sync,		##__VA_ARGS__)
  #define fsync(...)		evcoro_dskio_exf(fsync,		##__VA_ARGS__)
  #define fdatasync(...)	evcoro_dskio_exf(fdatasync,	##__VA_ARGS__)
  #define syncfs(...)		evcoro_dskio_exf(syncfs,	##__VA_ARGS__)
  #define msync(...)		evcoro_dskio_exf(msync,		##__VA_ARGS__)
  #define mlock(...)		evcoro_dskio_exf(mlock,		##__VA_ARGS__)
  #define mlockall(...)		evcoro_dskio_exf(mlockall,	##__VA_ARGS__)
  #define sync_file_range(...)	evcoro_dskio_exf(sync_file_range, ##__VA_ARGS__)
  #define fallocate(...)	evcoro_dskio_exf(fallocate,	##__VA_ARGS__)
  #define readahead(...)	evcoro_dskio_exf(readahead,	##__VA_ARGS__)
  #define lseek(...)		evcoro_dskio_exf(lseek,	   	##__VA_ARGS__)
  #define fstat(...)		evcoro_dskio_exf(fstat,	   	##__VA_ARGS__)
  #define fstatvfs(...)		evcoro_dskio_exf(fstatvfs, 	##__VA_ARGS__)
  #define ftruncate(...)	evcoro_dskio_exf(ftruncate,	##__VA_ARGS__)
  #define fchmod(...)		evcoro_dskio_exf(fchmod,   	##__VA_ARGS__)
  #define fchown(...)		evcoro_dskio_exf(fchown,   	##__VA_ARGS__)
  #define dup2(...)		evcoro_dskio_exf(dup2,	   	##__VA_ARGS__)
  #define utime(...)		evcoro_dskio_exf(utime,	   	##__VA_ARGS__)
  #define truncate(...)		evcoro_dskio_exf(truncate, 	##__VA_ARGS__)
  #define chown(...)		evcoro_dskio_exf(chown,	   	##__VA_ARGS__)
  #define chmod(...)		evcoro_dskio_exf(chmod,	   	##__VA_ARGS__)
  #define mkdir(...)		evcoro_dskio_exf(mkdir,	   	##__VA_ARGS__)
  #define rmdir(...)		evcoro_dskio_exf(rmdir,	   	##__VA_ARGS__)
  #define unlink(...)		evcoro_dskio_exf(unlink,   	##__VA_ARGS__)
  #define readlink(...)		evcoro_dskio_exf(readlink, 	##__VA_ARGS__)
  #define stat(...)		evcoro_dskio_exf(stat,	   	##__VA_ARGS__)
  #define lstat(...)		evcoro_dskio_exf(lstat,	   	##__VA_ARGS__)
  #define statvfs(...)		evcoro_dskio_exf(statvfs,  	##__VA_ARGS__)
  #define mknod(...)		evcoro_dskio_exf(mknod,	   	##__VA_ARGS__)
  #define link(...)		evcoro_dskio_exf(link,	   	##__VA_ARGS__)
  #define symlink(...)		evcoro_dskio_exf(symlink,  	##__VA_ARGS__)
  #define rename(...)		evcoro_dskio_exf(rename,   	##__VA_ARGS__)
#endif
