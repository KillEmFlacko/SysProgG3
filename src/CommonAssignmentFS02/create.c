#include "CommonAssignmentFS02/httpfs.h"

int httpfs_create( const char *path ,
                   mode_t mode ,
                   struct fuse_file_info *fi )
{
    struct
    {
        uint32_t mode;
    }
    header = { htonl( mode ) };

    HTTPFS_DO_REQUEST_WITH_HEADER( HTTPFS_OPCODE_create )
    {
        HTTPFS_CHECK_RESPONSE_STATUS;
        HTTPFS_CLEANUP;
        writel_msg("\nhttpfs_create(path=\"%s\", mode=0%03o, fuse_file_info=0x%08x)\n",path, fi);
        HTTPFS_RETURN( 0 );
    }
}
