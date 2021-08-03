#include "CommonAssignmentFS02/httpfs.h"

int httpfs_read( const char *path ,
                 char *buf ,
                 size_t size ,
                 off_t offset ,
                 struct fuse_file_info *fi )
{
    struct
    {
        uint32_t size;
        uint32_t offset;
    }
    header = { htonl( size ) ,
               htonl( offset ) };

    HTTPFS_DO_REQUEST_WITH_HEADER( HTTPFS_OPCODE_read )
    {
        HTTPFS_CHECK_RESPONSE_STATUS;
        if ( response.size > size )
        {
            HTTPFS_CLEANUP;
            HTTPFS_RETURN( EBADMSG );
        }

		memcpy( buf, response.payload, response.size );

        HTTPFS_CLEANUP;
        writel_msg("\nhttpfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	        path, buf, size, offset, fi);
        return response.size;
    }
}
