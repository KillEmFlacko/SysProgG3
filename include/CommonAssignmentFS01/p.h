#ifndef __DNFS_P_H_
#define __DNFS_P_H_

// Latest FUSE version at the moment is 2.9
#define FUSE_USE_VERSION 29


// pwrite() needs this define. This implies the use of setvbuf() instead of
// setlinebuf()
#define _XOPEN_SOURCE 500

// dnfs_state struct
// define the state of the dnfs. It only needs
// logfile fp and the string with the path
// of root dir of the filesystem
// (where it is mounted)
#include <limits.h>
#include <stdio.h>
struct dnfs_state {
    FILE *logfile;
    char *rootdir;
};

//dnfs data is the private data of fuse context
#define DNFS_DATA ((struct dnfs_state *) fuse_get_context()->private_data)

#endif
