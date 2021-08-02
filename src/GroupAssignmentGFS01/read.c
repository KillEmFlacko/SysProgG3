#include "GroupAssignmentGFS01/httpfs.h"
#include "GroupAssignmentGFS01/crypt.h"

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

		int block_size = EVP_CIPHER_block_size(_CRYPT_CIPHER);
		unsigned char plaintext[size + block_size];

		unsigned char key[KEY_LEN];
		Crypt_loadKey(GFS01_KEY_PATH,key);

		unsigned char iv[IV_LEN];
		Crypt_loadIV(GFS01_IV_PATH,iv);

		Crypt_decrypt(key,iv,(unsigned char*)response.payload,response.size,offset,plaintext);

		memcpy( buf, plaintext, response.size );

        HTTPFS_CLEANUP;
        return response.size;
    }
}
