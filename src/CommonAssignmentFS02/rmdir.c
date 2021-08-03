#include "CommonAssignmentFS02/httpfs.h"

int httpfs_rmdir( const char *path )
{
    HTTPFS_DO_SIMPLE_REQUEST( HTTPFS_OPCODE_rmdir )
    {
        HTTPFS_CHECK_RESPONSE_STATUS;
        HTTPFS_CLEANUP;
        writel_msg("httpfs_rmdir(path=\"%s\")\n", path);
        HTTPFS_RETURN( 0 );
    }
}
