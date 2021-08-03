#include "CommonAssignmentFS02/httpfs.h"

int httpfs_open( const char *path ,
                 struct fuse_file_info *fi )
{
    writel_msg("\nhttpfs_open(path=\"%s\", fi=0x%08x)\n", path, fi);
    HTTPFS_RETURN( 0 );
}
