#ifndef __DNFS_LOG_H_
#define __DNFS_LOG_H_
#include <stdio.h>

//  macro to write structs fields .
#define writel_struct(st, field, format, typecast) \
  writel_msg("    " #field " = " #format "\n", typecast st->field)

FILE *writel_open(void);
void writel_msg(const char *format, ...);
void writel_conn(struct fuse_conn_info *conn);
int writel_error(char *func);
void writel_fi(struct fuse_file_info *fi);
void writel_fuse_context(struct fuse_context *context);
void writel_retstat(char *func, int retstat);
void writel_stat(struct stat *si);
void writel_statvfs(struct statvfs *sv);
int  writel_syscall(char *func, int retstat, int min_ret);
void writel_utime(struct utimbuf *buf);

#endif
