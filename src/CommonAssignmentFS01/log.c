
#include "p.h"

#include <errno.h>
#include <fuse.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "log.h"

FILE *writel_open()
{
    
    // open log file or quit.
    FILE * logfd = fopen("dnfs.log", "w");
    if (logfd == NULL) {
	perror("writel_open");
	exit(EXIT_FAILURE);
    }
    
    // set logfd to line buffering
    setvbuf(logfd, NULL, _IOLBF, 0);

    return logfd;
}

void writel_msg(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vfprintf(DNFS_DATA->logfile, format, ap);
}

// Report errors to logfile and give -errno to caller
int writel_error(char *func)
{
  // do this save because the writel_msg can eventually change errno
    int ret = -errno;
    
    writel_msg("    ERROR %s: %s\n", func, strerror(errno));
    
    return ret;
}

// fuse context
void writel_fuse_context(struct fuse_context *context)
{
    writel_msg("    context:\n");
    
    /** Pointer to the fuse object */
    //	struct fuse *fuse;
    writel_struct(context, fuse, %08x, );

    /** User ID of the calling process */
    //	uid_t uid;
    writel_struct(context, uid, %d, );

    /** Group ID of the calling process */
    //	gid_t gid;
    writel_struct(context, gid, %d, );

    /** Thread ID of the calling process */
    //	pid_t pid;
    writel_struct(context, pid, %d, );

    /** Private filesystem data */
    //	void *private_data;
    writel_struct(context, private_data, %08x, );
    writel_struct(((struct dnfs_state *)context->private_data), logfile, %08x, );
    writel_struct(((struct dnfs_state *)context->private_data), rootdir, %s, );
	
    /** Umask of the calling process (introduced in version 2.8) */
    //	mode_t umask;
    writel_struct(context, umask, %05o, );
}

// fuse uses socket for communication
//we do not use here ... but logging is simple
void writel_conn(struct fuse_conn_info *conn)
{
    writel_msg("    conn:\n");
    
    /** Major version of the protocol (read-only) */
    // unsigned proto_major;
    writel_struct(conn, proto_major, %d, );

    /** Minor version of the protocol (read-only) */
    // unsigned proto_minor;
    writel_struct(conn, proto_minor, %d, );

    /** Is asynchronous read supported (read-write) */
    // unsigned async_read;
    writel_struct(conn, async_read, %d, );

    /** Maximum size of the write buffer */
    // unsigned max_write;
    writel_struct(conn, max_write, %d, );
    
    /** Maximum readahead */
    // unsigned max_readahead;
    writel_struct(conn, max_readahead, %d, );
    
    /** Capability flags, that the kernel supports */
    // unsigned capable;
    writel_struct(conn, capable, %08x, );
    
    /** Capability flags, that the filesystem wants to enable */
    // unsigned want;
    writel_struct(conn, want, %08x, );
    
    /** Maximum number of backgrounded requests */
    // unsigned max_background;
    writel_struct(conn, max_background, %d, );
    
    /** Kernel congestion threshold parameter */
    // unsigned congestion_threshold;
    writel_struct(conn, congestion_threshold, %d, );
    
    /** For future use. */
    // unsigned reserved[23];
}
    
// fuse_file_info stores information about files .
// The struct is in  /usr/include/fuse/fuse_common.h
void writel_fi (struct fuse_file_info *fi)
{
    writel_msg("    fi:\n");
    
    /** Open flags.  Available in open() and release() */
    //	int flags;
	writel_struct(fi, flags, 0x%08x, );
	
    /** Old file handle, don't use */
    //	unsigned long fh_old;	
	writel_struct(fi, fh_old, 0x%08lx,  );

    /** In case of a write operation indicates if this was caused by a
        writepage */
    //	int writepage;
	writel_struct(fi, writepage, %d, );

    /** Can be filled in by open, to use direct I/O on this file.
        Introduced in version 2.4 */
    //	unsigned int keep_cache : 1;
	writel_struct(fi, direct_io, %d, );

    /** Can be filled in by open, to indicate, that cached file data
        need not be invalidated.  Introduced in version 2.4 */
    //	unsigned int flush : 1;
	writel_struct(fi, keep_cache, %d, );

    /** Padding.  Do not use*/
    //	unsigned int padding : 29;

    /** File handle.  May be filled in by filesystem in open().
        Available in all other file operations */
    //	uint64_t fh;
	writel_struct(fi, fh, 0x%016llx,  );
	
    /** Lock owner id.  Available in locking operations and flush */
    //  uint64_t lock_owner;
	writel_struct(fi, lock_owner, 0x%016llx, );
}

void writel_retstat(char *func, int retstat)
{
    int errsave = errno;
    writel_msg("    %s returned %d\n", func, retstat);
    errno = errsave;
}
      
// make a system call, checking (and reporting) return status and
// possibly logging error
int writel_syscall(char *func, int retstat, int min_ret)
{
    writel_retstat(func, retstat);

    if (retstat < min_ret) {
	writel_error(func);
	retstat = -errno;
    }

    return retstat;
}

// stat is defined in
// <bits/stat.h>; this is indirectly included from <fcntl.h>
void writel_stat(struct stat *si)
{
    writel_msg("    si:\n");
    
    //  dev_t     st_dev;     /* ID of device containing file */
	writel_struct(si, st_dev, %lld, );
	
    //  ino_t     st_ino;     /* inode number */
	writel_struct(si, st_ino, %lld, );
	
    //  mode_t    st_mode;    /* protection */
	writel_struct(si, st_mode, 0%o, );
	
    //  nlink_t   st_nlink;   /* number of hard links */
	writel_struct(si, st_nlink, %d, );
	
    //  uid_t     st_uid;     /* user ID of owner */
	writel_struct(si, st_uid, %d, );
	
    //  gid_t     st_gid;     /* group ID of owner */
	writel_struct(si, st_gid, %d, );
	
    //  dev_t     st_rdev;    /* device ID (if special file) */
	writel_struct(si, st_rdev, %lld,  );
	
    //  off_t     st_size;    /* total size, in bytes */
	writel_struct(si, st_size, %lld,  );
	
    //  blksize_t st_blksize; /* blocksize for filesystem I/O */
	writel_struct(si, st_blksize, %ld,  );
	
    //  blkcnt_t  st_blocks;  /* number of blocks allocated */
	writel_struct(si, st_blocks, %lld,  );

    //  time_t    st_atime;   /* time of last access */
	writel_struct(si, st_atime, 0x%08lx, );

    //  time_t    st_mtime;   /* time of last modification */
	writel_struct(si, st_mtime, 0x%08lx, );

    //  time_t    st_ctime;   /* time of last status change */
	writel_struct(si, st_ctime, 0x%08lx, );
	
}

void writel_statvfs(struct statvfs *sv)
{
    writel_msg("    sv:\n");
    
    //  unsigned long  f_bsize;    /* file system block size */
	writel_struct(sv, f_bsize, %ld, );
	
    //  unsigned long  f_frsize;   /* fragment size */
	writel_struct(sv, f_frsize, %ld, );
	
    //  fsblkcnt_t     f_blocks;   /* size of fs in f_frsize units */
	writel_struct(sv, f_blocks, %lld, );
	
    //  fsblkcnt_t     f_bfree;    /* # free blocks */
	writel_struct(sv, f_bfree, %lld, );
	
    //  fsblkcnt_t     f_bavail;   /* # free blocks for non-root */
	writel_struct(sv, f_bavail, %lld, );
	
    //  fsfilcnt_t     f_files;    /* # inodes */
	writel_struct(sv, f_files, %lld, );
	
    //  fsfilcnt_t     f_ffree;    /* # free inodes */
	writel_struct(sv, f_ffree, %lld, );
	
    //  fsfilcnt_t     f_favail;   /* # free inodes for non-root */
	writel_struct(sv, f_favail, %lld, );
	
    //  unsigned long  f_fsid;     /* file system ID */
	writel_struct(sv, f_fsid, %ld, );
	
    //  unsigned long  f_flag;     /* mount flags */
	writel_struct(sv, f_flag, 0x%08lx, );
	
    //  unsigned long  f_namemax;  /* maximum filename length */
	writel_struct(sv, f_namemax, %ld, );
	
}

void writel_utime(struct utimbuf *buf)
{
    writel_msg("    buf:\n");
    
    //    time_t actime;
    writel_struct(buf, actime, 0x%08lx, );
	
    //    time_t modtime;
    writel_struct(buf, modtime, 0x%08lx, );
}

