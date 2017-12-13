/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  There are a couple of symbols that need to be #defined before
  #including all the headers.
*/

#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 29
#define FUSE_USE_VERSION 29

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

#include <limits.h>
#include <stdio.h>

#define NPHFS_DATA ((struct nphfuse_state *) fuse_get_context()->private_data)
#include "nphfuse_extra.h"
#endif

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"
int nphfuse_getattr(const char *path, struct stat *statbuf);
int nphfuse_readlink(const char *path, char *link, size_t size);
int nphfuse_mknod(const char *path, mode_t mode, dev_t dev);
int nphfuse_mkdir(const char *path, mode_t mode);
int nphfuse_unlink(const char *path);
int nphfuse_rmdir(const char *path);
int nphfuse_symlink(const char *path, const char *link);
int nphfuse_rename(const char *path, const char *newpath);
int nphfuse_link(const char *path, const char *newpath);
int nphfuse_chmod(const char *path, mode_t mode);
int nphfuse_chown(const char *path, uid_t uid, gid_t gid);
int nphfuse_truncate(const char *path, off_t newsize);
int nphfuse_utime(const char *path, struct utimbuf *ubuf);
int nphfuse_open(const char *path, struct fuse_file_info *fi);
int nphfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int nphfuse_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi);
int nphfuse_statfs(const char *path, struct statvfs *statv);
int nphfuse_flush(const char *path, struct fuse_file_info *fi);
int nphfuse_release(const char *path, struct fuse_file_info *fi);
int nphfuse_fsync(const char *path, int datasync, struct fuse_file_info *fi);
#ifdef HAVE_SYS_XATTR_H
/** Set extended attributes */
int nphfuse_setxattr(const char *path, const char *name, const char *value, size_t size, int flags);
int nphfuse_getxattr(const char *path, const char *name, char *value, size_t size);
int nphfuse_listxattr(const char *path, char *list, size_t size);
int nphfuse_removexattr(const char *path, const char *name);
#endif
int nphfuse_opendir(const char *path, struct fuse_file_info *fi);
int nphfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int nphfuse_releasedir(const char *path, struct fuse_file_info *fi);
int nphfuse_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi);
void *nphfuse_init(struct fuse_conn_info *conn);
void nphfuse_destroy(void *userdata);
int nphfuse_access(const char *path, int mask);
int nphfuse_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi);
int nphfuse_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi);
