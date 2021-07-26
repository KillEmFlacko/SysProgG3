#include "CommonAssignmentFS01/p.h"

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

#include "CommonAssignmentFS01/log.h"

//  All the paths are relative to the root of the mounted
//  filesystem.  paths have to be completed by mountpoint
static void dnfs_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, DNFS_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // truncates long paths

    writel_msg("    dnfs_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
	    DNFS_DATA->rootdir, path, fpath);
}

//8<==============================================>8
// 
// next prototypes are in /usr/include/fuse.h
//
//8<==============================================>8


/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int dnfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat;
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);
    dnfs_fullpath(fpath, path);

    retstat = writel_syscall("lstat", lstat(fpath, statbuf), 0);
    
    writel_stat(statbuf);
    
    return retstat;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must be one
// less than the size passed to dnfs_readlink()
// dnfs_readlink() 
int dnfs_readlink(const char *path, char *link, size_t size)
{
    int retstat;
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
	  path, link, size);
    dnfs_fullpath(fpath, path);

    retstat = writel_syscall("readlink", readlink(fpath, link, size - 1), 0);
    if (retstat >= 0) {
	link[retstat] = '\0';
	retstat = 0;
	writel_msg("    link=\"%s\"\n", link);
    }
    
    return retstat;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
int dnfs_mknod(const char *path, mode_t mode, dev_t dev)
{
    int retstat;
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n",
	  path, mode, dev);
    dnfs_fullpath(fpath, path);
    
    // On Linux this could just be 'mknod(path, mode, dev)' but this
    // tries to be be more portable by honoring the quote in the Linux
    // mknod man page stating the only portable use of mknod() is to
    // make a fifo, but saying it should never actually be used for
    // that.
    if (S_ISREG(mode)) {
	retstat = writel_syscall("open", open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode), 0);
	if (retstat >= 0)
	    retstat = writel_syscall("close", close(retstat), 0);
    } else
	if (S_ISFIFO(mode))
	    retstat = writel_syscall("mkfifo", mkfifo(fpath, mode), 0);
	else
	    retstat = writel_syscall("mknod", mknod(fpath, mode, dev), 0);
    
    return retstat;
}

/** Create a directory */
int dnfs_mkdir(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
    dnfs_fullpath(fpath, path);

    return writel_syscall("mkdir", mkdir(fpath, mode), 0);
}

/** Remove a file */
int dnfs_unlink(const char *path)
{
    char fpath[PATH_MAX];
    
    writel_msg("dnfs_unlink(path=\"%s\")\n",
	    path);
    dnfs_fullpath(fpath, path);

    return writel_syscall("unlink", unlink(fpath), 0);
}

/** Remove a directory */
int dnfs_rmdir(const char *path)
{
    char fpath[PATH_MAX];
    
    writel_msg("dnfs_rmdir(path=\"%s\")\n",
	    path);
    dnfs_fullpath(fpath, path);

    return writel_syscall("rmdir", rmdir(fpath), 0);
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int dnfs_symlink(const char *path, const char *link)
{
    char flink[PATH_MAX];
    
    writel_msg("\ndnfs_symlink(path=\"%s\", link=\"%s\")\n",
	    path, link);
    dnfs_fullpath(flink, link);

    return writel_syscall("symlink", symlink(path, flink), 0);
}

/** Rename a file */
// both path and newpath are fs-relative
int dnfs_rename(const char *path, const char *newpath)
{
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];
    
    writel_msg("\ndnfs_rename(fpath=\"%s\", newpath=\"%s\")\n",
	    path, newpath);
    dnfs_fullpath(fpath, path);
    dnfs_fullpath(fnewpath, newpath);

    return writel_syscall("rename", rename(fpath, fnewpath), 0);
}

/** Create a hard link to a file */
int dnfs_link(const char *path, const char *newpath)
{
    char fpath[PATH_MAX], fnewpath[PATH_MAX];
    
    writel_msg("\ndnfs_link(path=\"%s\", newpath=\"%s\")\n",
	    path, newpath);
    dnfs_fullpath(fpath, path);
    dnfs_fullpath(fnewpath, newpath);

    return writel_syscall("link", link(fpath, fnewpath), 0);
}

/** Change the permission bits of a file */
int dnfs_chmod(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_chmod(fpath=\"%s\", mode=0%03o)\n",
	    path, mode);
    dnfs_fullpath(fpath, path);

    return writel_syscall("chmod", chmod(fpath, mode), 0);
}

/** Change the owner and group of a file */
int dnfs_chown(const char *path, uid_t uid, gid_t gid)
  
{
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_chown(path=\"%s\", uid=%d, gid=%d)\n",
	    path, uid, gid);
    dnfs_fullpath(fpath, path);

    return writel_syscall("chown", chown(fpath, uid, gid), 0);
}

/** Change the size of a file */
int dnfs_truncate(const char *path, off_t newsize)
{
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_truncate(path=\"%s\", newsize=%lld)\n",
	    path, newsize);
    dnfs_fullpath(fpath, path);

    return writel_syscall("truncate", truncate(fpath, newsize), 0);
}

/** Change the access and/or modification times of a file */
/* note -- To Update */
int dnfs_utime(const char *path, struct utimbuf *ubuf)
{
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_utime(path=\"%s\", ubuf=0x%08x)\n",
	    path, ubuf);
    dnfs_fullpath(fpath, path);

    return writel_syscall("utime", utime(fpath, ubuf), 0);
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 */
int dnfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    int fd;
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_open(path\"%s\", fi=0x%08x)\n",
	    path, fi);
    dnfs_fullpath(fpath, path);
    
    // if the open call succeeds, my retstat is the file descriptor,
    // else it's -errno.  I'm making sure that in that case the saved
    // file descriptor is exactly -1.
    fd = writel_syscall("open", open(fpath, fi->flags), 0);
    if (fd < 0)
	retstat = writel_error("open");
	
    fi->fh = fd;

    writel_fi(fi);
    
    return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 */

int dnfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    writel_msg("\ndnfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    writel_fi(fi);

    return writel_syscall("pread", pread(fi->fh, buf, size, offset), 0);
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 */
int dnfs_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;
    
    writel_msg("\ndnfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi
	    );
    // no need to get fpath on this one, since I work from fi->fh not the path
    writel_fi(fi);

    return writel_syscall("pwrite", pwrite(fi->fh, buf, size, offset), 0);
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * uses parameters in  'struct statvfs' 
 */
int dnfs_statfs(const char *path, struct statvfs *statv)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_statfs(path=\"%s\", statv=0x%08x)\n",
	    path, statv);
    dnfs_fullpath(fpath, path);
    
    // get stats for underlying filesystem
    retstat = writel_syscall("statvfs", statvfs(fpath, statv), 0);
    
    writel_statvfs(statv);
    
    return retstat;
}

/** Possibly flush cached data
 *
 * NOTICE!!!: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 *
 */
// this is a no-op in DNFS.  It just logs the call and returns success
int dnfs_flush(const char *path, struct fuse_file_info *fi)
{
    writel_msg("\ndnfs_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    writel_fi(fi);
	
    return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * 
 */
int dnfs_release(const char *path, struct fuse_file_info *fi)
{
    writel_msg("\ndnfs_release(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    writel_fi(fi);

    // We need to close the file.  Had we allocated any resources
    // (buffers etc) we'd need to free them here as well.
    return writel_syscall("close", close(fi->fh), 0);
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 */
int dnfs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    writel_msg("\ndnfs_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
	    path, datasync, fi);
    writel_fi(fi);
    
 
#ifdef HAVE_FDATASYNC
    if (datasync)
	return writel_syscall("fdatasync", fdatasync(fi->fh), 0);
    else
#endif	
	return writel_syscall("fsync", fsync(fi->fh), 0);
}

#ifdef HAVE_SYS_XATTR_H

/** Note that XXXxattr functions use
    the 'l-' versions of the functions (eg dnfs_setxattr() calls
    lsetxattr() not setxattr(), etc).  This is because it appears any
    symbolic links are resolved before the actual call takes place, so
    I only need to use the system-provided calls that don't follow
    them */

/** Set extended attributes */
int dnfs_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_setxattr(path=\"%s\", name=\"%s\", value=\"%s\", size=%d, flags=0x%08x)\n",
	    path, name, value, size, flags);
    dnfs_fullpath(fpath, path);

    return writel_syscall("lsetxattr", lsetxattr(fpath, name, value, size, flags), 0);
}

/** Get extended attributes */
int dnfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_getxattr(path = \"%s\", name = \"%s\", value = 0x%08x, size = %d)\n",
	    path, name, value, size);
    dnfs_fullpath(fpath, path);

    retstat = writel_syscall("lgetxattr", lgetxattr(fpath, name, value, size), 0);
    if (retstat >= 0)
	writel_msg("    value = \"%s\"\n", value);
    
    return retstat;
}

/** List extended attributes */
int dnfs_listxattr(const char *path, char *list, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    char *ptr;
    
    writel_msg("\ndnfs_listxattr(path=\"%s\", list=0x%08x, size=%d)\n",
	    path, list, size
	    );
    dnfs_fullpath(fpath, path);

    retstat = writel_syscall("llistxattr", llistxattr(fpath, list, size), 0);
    if (retstat >= 0) {
	writel_msg("    returned attributes (length %d):\n", retstat);
	if (list != NULL)
	    for (ptr = list; ptr < list + retstat; ptr += strlen(ptr)+1)
		writel_msg("    \"%s\"\n", ptr);
	else
	    writel_msg("    (null)\n");
    }
    
    return retstat;
}

/** Remove extended attributes */
int dnfs_removexattr(const char *path, const char *name)
{
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_removexattr(path=\"%s\", name=\"%s\")\n",
	    path, name);
    dnfs_fullpath(fpath, path);

    return writel_syscall("lremovexattr", lremovexattr(fpath, name), 0);
}
#endif

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 */
int dnfs_opendir(const char *path, struct fuse_file_info *fi)
{
    DIR *dp;
    int retstat = 0;
    char fpath[PATH_MAX];
    
    writel_msg("\ndnfs_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    dnfs_fullpath(fpath, path);

    // since opendir returns a pointer, takes some custom handling of
    // return status.
    dp = opendir(fpath);
    writel_msg("    opendir returned 0x%p\n", dp);
    if (dp == NULL)
	retstat = writel_error("dnfs_opendir opendir");
    
    fi->fh = (intptr_t) dp;
    
    writel_fi(fi);
    
    return retstat;
}

/** Read directory
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 */

int dnfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    DIR *dp;
    struct dirent *de;
    
    writel_msg("\ndnfs_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",
	    path, buf, filler, offset, fi);
    // once again, no need for fullpath -- but note that I need to cast fi->fh
    dp = (DIR *) (uintptr_t) fi->fh;

    // Every directory contains at least two entries: . and ..  If my
    // first call to the system readdir() returns NULL I've got an
    // error; near as I can tell, that's the only condition under
    // which I can get an error from readdir()
    de = readdir(dp);
    writel_msg("    readdir returned 0x%p\n", de);
    if (de == 0) {
	retstat = writel_error("dnfs_readdir readdir");
	return retstat;
    }

    // This will copy the entire directory into the buffer.  The loop exits
    // when either the system readdir() returns NULL, or filler()
    // returns something non-zero.  The first case just means I've
    // read the whole directory; the second means the buffer is full.
    do {
	writel_msg("calling filler with name %s\n", de->d_name);
	if (filler(buf, de->d_name, NULL, 0) != 0) {
	    writel_msg("    ERROR dnfs_readdir filler:  buffer full");
	    return -ENOMEM;
	}
    } while ((de = readdir(dp)) != NULL);
    
    writel_fi(fi);
    
    return retstat;
}

/** Release directory
 *
 */
int dnfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    writel_msg("\ndnfs_releasedir(path=\"%s\", fi=0x%08x)\n",
	    path, fi);
    writel_fi(fi);
    
    closedir((DIR *) (uintptr_t) fi->fh);
    
    return retstat;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 */
int dnfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    writel_msg("\ndnfs_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
	    path, datasync, fi);
    writel_fi(fi);
    
    return retstat;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 */
// the fuse_context is set up before this function is called, and
// fuse_get_context()->private_data returns the user_data passed to
// fuse_main().
void *dnfs_init(struct fuse_conn_info *conn)
{
    writel_msg("\ndnfs_init()\n");
    
    writel_conn(conn);
    writel_fuse_context(fuse_get_context());
    
    return DNFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 */
void dnfs_destroy(void *userdata)
{
    writel_msg("\ndnfs_destroy(userdata=0x%08x)\n", userdata);
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 *
 */
int dnfs_access(const char *path, int mask)
{
    int retstat = 0;
    char fpath[PATH_MAX];
   
    writel_msg("\ndnfs_access(path=\"%s\", mask=0%o)\n",
	    path, mask);
    dnfs_fullpath(fpath, path);
    
    retstat = access(fpath, mask);
    
    if (retstat < 0)
	retstat = writel_error("dnfs_access access");
    
    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 */
// Not implemented. .

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 *
 */
int dnfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    writel_msg("\ndnfs_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
	    path, offset, fi);
    writel_fi(fi);
    
    retstat = ftruncate(fi->fh, offset);
    if (retstat < 0)
	retstat = writel_error("dnfs_ftruncate ftruncate");
    
    return retstat;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 */
int dnfs_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    writel_msg("\ndnfs_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
	    path, statbuf, fi);
    writel_fi(fi);

    if (!strcmp(path, "/"))
	return dnfs_getattr(path, statbuf);
    
    retstat = fstat(fi->fh, statbuf);
    if (retstat < 0)
	retstat = writel_error("dnfs_fgetattr fstat");
    
    writel_stat(statbuf);
    
    return retstat;
}

struct fuse_operations dnfs_oper = {
  .getattr = dnfs_getattr,
  .readlink = dnfs_readlink,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = dnfs_mknod,
  .mkdir = dnfs_mkdir,
  .unlink = dnfs_unlink,
  .rmdir = dnfs_rmdir,
  .symlink = dnfs_symlink,
  .rename = dnfs_rename,
  .link = dnfs_link,
  .chmod = dnfs_chmod,
  .chown = dnfs_chown,
  .truncate = dnfs_truncate,
  .utime = dnfs_utime,
  .open = dnfs_open,
  .read = dnfs_read,
  .write = dnfs_write,

  .statfs = dnfs_statfs,
  .flush = dnfs_flush,
  .release = dnfs_release,
  .fsync = dnfs_fsync,
  
#ifdef HAVE_SYS_XATTR_H
  .setxattr = dnfs_setxattr,
  .getxattr = dnfs_getxattr,
  .listxattr = dnfs_listxattr,
  .removexattr = dnfs_removexattr,
#endif
  
  .opendir = dnfs_opendir,
  .readdir = dnfs_readdir,
  .releasedir = dnfs_releasedir,
  .fsyncdir = dnfs_fsyncdir,
  .init = dnfs_init,
  .destroy = dnfs_destroy,
  .access = dnfs_access,
  .ftruncate = dnfs_ftruncate,
  .fgetattr = dnfs_fgetattr
};

void dnfs_usage()
{
    fprintf(stderr, "usage:  dnfs [FUSE and mount options] rootDir mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct dnfs_state *dnfs_data;

    if ((getuid() == 0) || (geteuid() == 0)) {
    	fprintf(stderr, "Running DNFS as root opens unnacceptable security holes\n");
    	return 1;
    }

    // See which version of fuse we're running
    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	dnfs_usage();

    dnfs_data = malloc(sizeof(struct dnfs_state));
    if (dnfs_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the rootdir out of the argument list and save it in my
    // internal data
    dnfs_data->rootdir = realpath(argv[argc-2], NULL);
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    dnfs_data->logfile = writel_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &dnfs_oper, dnfs_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
