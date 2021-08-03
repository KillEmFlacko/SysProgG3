#include "CommonAssignmentFS02/httpfs.h"

int httpfs_readdir( const char *path ,
                    void *buf ,
                    fuse_fill_dir_t filler ,
                    off_t offset ,
                    struct fuse_file_info *fi )
{
    HTTPFS_DO_SIMPLE_REQUEST( HTTPFS_OPCODE_readdir )
    {
        char *p;

        HTTPFS_CHECK_RESPONSE_STATUS;

        for ( p = response.payload ;
              p - response.payload < response.size ;
              p += strnlen( p , response.size - ( p - response.payload ) ) + 1 )
        {
            filler( buf , p , NULL , 0 );
        }

        HTTPFS_CLEANUP;
        writel_msg("\nhttpfs_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",
	        path, buf, filler, offset, fi);
        HTTPFS_RETURN( 0 );
    }
}
