#include "GroupAssignmentGFS01/httpfs.h"

int httpfs_rmdir( const char *path )
{
    HTTPFS_DO_SIMPLE_REQUEST( HTTPFS_OPCODE_rmdir )
    {
        HTTPFS_CHECK_RESPONSE_STATUS;
        HTTPFS_CLEANUP;
        HTTPFS_RETURN( 0 );
    }
}
