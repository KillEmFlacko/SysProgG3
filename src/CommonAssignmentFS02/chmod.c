#include "CommonAssignmentFS02/httpfs.h"

int httpfs_chmod( const char *path ,
                  mode_t mode )
{

    struct
    {
        uint32_t mode;
    }
    header = { htonl( mode ) };

    HTTPFS_DO_REQUEST_WITH_HEADER( HTTPFS_OPCODE_chmod )
    {
        HTTPFS_CHECK_RESPONSE_STATUS;
        HTTPFS_CLEANUP;
        writel_msg("\nhttpfs_chmod(fpath=\"%s\", mode=0%03o)\n", path, mode);
        HTTPFS_RETURN( 0 );
    } 

}
