#include "GroupAssignmentGFS01/httpfs.h"
#include "GroupAssignmentGFS01/crypt.h"
#include <openssl/cryptoerr.h>
#include <openssl/evp.h>

int httpfs_write( const char *path ,
                  const char *buf ,
                  size_t size ,
                  off_t offset ,
                  struct fuse_file_info *fi )
{
	int block_size = EVP_CIPHER_block_size(_CRYPT_CIPHER);
	char crypt_buf[size+block_size];

	unsigned char key[KEY_LEN];
	Crypt_loadKey(GFS01_KEY_PATH,key);

	unsigned char iv[IV_LEN];
	Crypt_loadIV(GFS01_IV_PATH,iv);

	Crypt_encrypt(key,iv,(unsigned char*)buf,size,offset,(unsigned char*)crypt_buf);

	struct raw_data raw_data = { ( char * )crypt_buf, size };
    struct
    {
        uint32_t size;
        uint32_t offset;
    }
    header = { htonl( size ) ,
               htonl( offset ) };

    HTTPFS_DO_REQUEST_WITH_HEADER_AND_DATA( HTTPFS_OPCODE_write )
    {
        uint32_t write_size;

        HTTPFS_CHECK_RESPONSE_STATUS;
        if ( response.size != sizeof( uint32_t ) )
        {
            HTTPFS_CLEANUP;
            HTTPFS_RETURN( EBADMSG );
        }

        write_size = ntohl( *( uint32_t * )response.payload );
        HTTPFS_CLEANUP;
        return write_size;
    }
}
