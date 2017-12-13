// Project 3: Jithin John, jjohn3; Abhash Jain, ajain28; Bhavya Bansal, bbansal;
/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  You may extend this file if necessary  
*/

#include <sys/stat.h>
#include <dirent.h>

#define ROOT_INODE 1
#define BLOCK_SIZE 8192
#define DIRENTSIZE sizeof(struct dirent)
#define DIRENTNUM (BLOCK_SIZE/DIRENTSIZE - 1)
#define ROOT_DIR "/"

struct nphfuse_state {
    FILE *logfile;
    char *device_name;
    int devfd;
};

struct nphfuse_dir {
    struct stat myStat;
    struct dirent direntList[DIRENTNUM];
};
