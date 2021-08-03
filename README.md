# SysProgG3

## Dependencies

- doxygen
- make
- makedepend
- libcurl
- fuse
- openssl

## Description

Repository contenente tutti gli assignment del gruppo 3 per il corso di System Programming 2020/2021

## Common Assignment IPC01

### How to run

- To build the project run `make`
- To test the library run `make test` or `make -s test`
- To generate the documentation run `make doc`, documentation can be found in `doc/html_doc`

## Common Assignment IPC02

### How to run

- To build the project run `make`
- To generate the documentation run `make doc`
- Run the right executables from build/CommonAssignmentIPC02/bin directory.
    - Reader-Writer problems have a single executable for each problem (fork) while Producer-Consumer problems have 2 executables, one for the producer and one for the consumer

## Common Assignment FS01

### How to run
This assignment has different source codes, we don’t provide a single executable to do all, so the following steps are required to test the calls reported above:
- make to build all
- from the root of the project run ./build/CommonAssignmentFS01/bin/dnfs build/CommonAssignmentFS01/fs/rootDir build/CommonAssignmentFS01/fs/mountPoint to create the necessary directories, to move executables into the root dir of the file system to mount, to mount the FUSE file system and to generate the dnfs.log file where we can read the calls
- open the log file into data/dnfs.log
- run always from the root of the project the executables into the mountPoint directory (./build/CommonAssignmentFS01/fs/mountPoint/random_chars or write_char_by_char or write_line_by_line or fifo_sendmsg or fifo_rcvmsg) or run commands from the shell into the mountPoint directory and see the calls into the log file. 
Before running the 2 executables for FIFO, make sure you are into the mount point so $cd build/CommonAssignmentFS01/fs/mountPoint and create the fifo file through $mknod fifo p, after you can run sender and receiver processes through $./fifo_rcvmsg and $./fifo_sendmsg message
- $fusermount -u build/CommonAssignmentFS01/fs/mountPoint to unmount the file system blocking the writing on the log file

### How to gerate the documentation
- $make doc 
to generate the html documentation with doxygen. In this assignment there aren't personal libraries, so the doc will only have the main functions into /src

## Common Assignment FS02

### How to run
- To build the project run make
- Change directory to build/CommonAssignmentFS02/ and create three directories: www/ mnt/ files/
- Generate httpfs.php with ./bin/httpfs generate php > www/httpfs.php and remove or comment the line  if ( VERBOSE ) set_error_handler( 'store_error' ); (line 31) in httpfs.php
- Start the server with php -S localhost:8000 -t www/
- Open another terminal in build/CommonAssignmentFS02/ and mount the filesystem with ./bin/httpfs mount http://localhost:8000/httpfs.php mnt/ ../files
- run shell commands into the mnt dir
- open the log file dnfs.log available in build/CommonAssignmentFS02/ and see the log report

### How to gerate the documentation
- $make doc
to generate the html documentation with doxygen

## Group Assignment GIPC01

### How to run

- To build the project run `make`
    - Two executables will be placed in `build/GroupAssignmentGIPC01/bin`
    - The `lamport` executable wants the number of processes as command line parameter
    - The `dining_philosopers` executable needs no command line arguments
- To generate the documentation run `make doc`

## Group Assignment GFS01

### How to run

- To build the project run `make`
- Change directory to `build/GroupAssignmentGFS01/` and create three directories: `www/` `mnt/` `files/`
- Generate httpfs.php with `./bin/httpfs generate php > www/httpfs.php` and remove or comment the line  `if ( VERBOSE ) set_error_handler( 'store_error' );` (line 31) in `httpfs.php`
- Start the server with `php -S localhost:8000 -t www/`
- Mount the filesystem with `./bin/httpfs mount http://localhost:8000/httpfs.php mnt/ ../files`
- Now every file created in `mnt/` will be encrypted when uploaded on the server and decrypted when read from the client, this can be seen with a cat on the files

- To generate the documentation run `make doc`
