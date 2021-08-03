#include "CommonAssignmentFS02/httpfs.h"


int httpfs_chown( const char *path ,
                  uid_t uid ,
                  gid_t gid )
{

    struct
    {
        uint32_t uid;
        uint32_t gid;
    }
    header = { htonl( uid ) ,
               htonl( gid ) };

    HTTPFS_DO_REQUEST_WITH_HEADER( HTTPFS_OPCODE_chown )
    {
        HTTPFS_CHECK_RESPONSE_STATUS;
        HTTPFS_CLEANUP;
        writel_msg("\nhttpfs_chown(path=\"%s\", uid=%d, gid=%d)\n", path, uid, gid);
        HTTPFS_RETURN( 0 );
        
    }
}
