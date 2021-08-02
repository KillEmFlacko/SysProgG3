#include "CommonAssignmentFS02/httpfs.h"

int httpfs_chmod( const char *path ,
                  mode_t mode )
{

    char fpath[PATH_MAX];
    writel_msg("\nhttpfs_chmod(fpath=\"%s\", mode=0%03o)\n", path, mode);
    dnfs_fullpath(fpath, path);
    writel_syscall("chmod", chmod(fpath, mode), 0);

    struct
    {
        uint32_t mode;
    }
    header = { htonl( mode ) };

    HTTPFS_DO_REQUEST_WITH_HEADER( HTTPFS_OPCODE_chmod )
    {
        HTTPFS_CHECK_RESPONSE_STATUS;
        HTTPFS_CLEANUP;
        HTTPFS_RETURN( 0 );
    } 

}
