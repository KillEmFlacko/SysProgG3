#include <stdlib.h>
#include "GroupAssignmentGFS01/generators.h"
#include "GroupAssignmentGFS01/httpfs.h"
#include "GroupAssignmentGFS01/fuse_api/fuse_api.h"
#include "GroupAssignmentGFS01/version.h"

static void usage()
{
    fprintf( stderr ,
             "Usage:\n\n"
             "    httpfs --help\n"
             "    httpfs --version\n"
             "    httpfs generators\n"
             "    httpfs generate <generator>\n"
             "    httpfs mount <url> <mount_point> [<remote_chroot>]\n" );
}

static void info()
{
    fprintf( stderr , "httpfs " HTTPFS_VERSION "\n" );
}

static void set_verbose_mode()
{
    char *env;

    env = getenv( "HTTPFS_VERBOSE" );
    if ( env && strcmp( env , "1" ) == 0 ) HTTPFS_VERBOSE = 1;
}

int main( int argc , char *argv[] )
{
    set_verbose_mode();

    if ( argc == 2 && strcmp( argv[ 1 ] , "--version" ) == 0 )
    {
        info();
    }
    else if ( argc == 2 && strcmp( argv[ 1 ] , "--help" ) == 0 )
    {
        usage();
    }
    else if ( argc == 2 && strcmp( argv[ 1 ] , "generators" ) == 0 )
    {
        const struct httpfs_generator *generator;

        for ( generator = HTTPFS_GENERATORS ; generator->name ; generator++ )
        {
            printf( "%s\n" , generator->name );
        }
    }
    else if ( argc == 3 && strcmp( argv[ 1 ] , "generate" ) == 0 )
    {
        if ( !httpfs_generate( argv[ 2 ] ) )
        {
            usage();
            return EXIT_FAILURE;
        }
    }
    else if ( ( argc == 4 || argc == 5 ) &&
              strcmp( argv[ 1 ] , "mount" ) == 0 )
    {
        struct httpfs httpfs;
        const char *url;
        const char *remote_chroot;
        char *mount_point;
        int rv;

        url = argv[ 2 ];
        remote_chroot = ( argc == 5 ? argv[ 4 ] : NULL );
        mount_point = argv[ 3 ];

        rv = httpfs_fuse_start( &httpfs , url , remote_chroot , mount_point );

        if ( rv )
        {
            fprintf( stderr , "Unable to mount: " );

            switch ( rv )
            {
            case HTTPFS_FUSE_ERROR:
                fprintf( stderr , "cannot initialize FUSE\n" );
                break;

            case HTTPFS_CURL_ERROR:
                fprintf( stderr , "cannot initialize cURL\n" );
                break;

            case HTTPFS_UNREACHABLE_SERVER_ERROR:
                fprintf( stderr , "cannot reach the remote server\n" );
                break;

            case HTTPFS_WRONG_CHROOT_ERROR:
                fprintf( stderr , "cannot find the remote path\n" );
                break;

            case HTTPFS_ERRNO_ERROR:
                fprintf( stderr , "errno (%i) %s\n" , errno , strerror( errno ) );
                break;
            }
        }
    }
    else
    {
        usage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
