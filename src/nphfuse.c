/*
  Key Value System
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
  A copy of that code is included in the file fuse.h
  
  The point of this FUSE filesystem is to provide an introduction to
  FUSE.  It was my first FUSE filesystem as I got to know the
  software; hopefully, the comments in this code will help people who
  follow later to get a gentler introduction.

*/
#include "nphfuse.h"
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
//#define HAVE_SYS_XATTR_H 1
#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

struct nphfuse_state *nphfuse_data;

struct fuse_operations nphfuse_oper = {
  .getattr = nphfuse_getattr,
  .readlink = nphfuse_readlink,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = nphfuse_mknod,
  .mkdir = nphfuse_mkdir,
  .unlink = nphfuse_unlink,
  .rmdir = nphfuse_rmdir,
  .symlink = nphfuse_symlink,
  .rename = nphfuse_rename,
  .link = nphfuse_link,
  .chmod = nphfuse_chmod,
  .chown = nphfuse_chown,
  .truncate = nphfuse_truncate,
  .utime = nphfuse_utime,
  .open = nphfuse_open,
  .read = nphfuse_read,
  .write = nphfuse_write,
  .statfs = nphfuse_statfs,
  /** Just a placeholder, don't set */ // huh???
  .flush = nphfuse_flush,
  .release = nphfuse_release,
  .fsync = nphfuse_fsync,
  
#ifdef HAVE_SYS_XATTR_H
  .setxattr = nphfuse_setxattr,
  .getxattr = nphfuse_getxattr,
  .listxattr = nphfuse_listxattr,
  .removexattr = nphfuse_removexattr,
#endif
  
  .opendir = nphfuse_opendir,
  .readdir = nphfuse_readdir,
  .releasedir = nphfuse_releasedir,
  .fsyncdir = nphfuse_fsyncdir,
  .init = nphfuse_init,
  .destroy = nphfuse_destroy,
  .access = nphfuse_access,
  .ftruncate = nphfuse_ftruncate,
  .fgetattr = nphfuse_fgetattr
};


void nphfuse_usage()
{
    fprintf(stderr, "usage:  nphfuse [FUSE and mount options] npheap_device_name mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;

    // NPHeapFS doesn't do any access checking on its own (the comment
    // blocks in fuse.h mention some of the functions that need
    // accesses checked -- but note there are other functions, like
    // chown(), that also need checking!).  Since running NPHeapFS as root
    // will therefore open Metrodome-sized holes in the system
    // security, we'll check if root is trying to mount the filesystem
    // and refuse if it is.  The somewhat smaller hole of an ordinary
    // user doing it with the allow_other flag is still there because
    // I don't want to parse the options string.
    if ((getuid() == 0) || (geteuid() == 0)) {
	fprintf(stderr, "Running NPHeapFS as root opens unnacceptable security holes\n");
	return 1;
    }

    // See which version of fuse we're running
    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    
    // Perform some sanity checking on the command line:  make sure
    // there are enough arguments, and that neither of the last two
    // start with a hyphen (this will break if you actually have a
    // rootpoint or mountpoint whose name starts with a hyphen, but so
    // will a zillion other programs)
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	nphfuse_usage();

    nphfuse_data = (struct nphfuse_state *)malloc(sizeof(struct nphfuse_state));
    if (nphfuse_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the rootdir out of the argument list and save it in my
    // internal data
    nphfuse_data->device_name = (char *)malloc((strlen(argv[argc-2])+1)*sizeof(char));
    strcpy(nphfuse_data->device_name,argv[argc-2]);
    nphfuse_data->devfd = open(nphfuse_data->device_name,O_RDWR);
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    // You can output to a log file for debugging if you would like to.
    nphfuse_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &nphfuse_oper, nphfuse_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
