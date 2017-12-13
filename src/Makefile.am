bin_PROGRAMS = nphfuse
nphfuse_SOURCES = nphfuse.c log.c log.h  nphfuse_extra.h nphfuse.h nphfuse_functions.c
AM_CFLAGS = @FUSE_CFLAGS@
LDADD = @FUSE_LIBS@ -lnpheap -lpthread
