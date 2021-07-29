# SysProgG3

## Dependencies

- make
- makedepend
- doxygen

## Description

Repository contenente tutti gli assignment del gruppo 3 per il corso di System Programming 2020/2021

## Common Assignment FS02

The test partition can be found at [HPFS driver for Linux](https://artax.karlin.mff.cuni.cz/~mikulas/vyplody/hpfs/index-e.cgi), download and extract in the repository root directory.

- To load the __compiled__ module: `# insmod build/CommonAssignmentFS02/hpfs.ko`
- To mount the test partition: `# mount -t hpfs -o umask=0000 test-hpfs-partition mountpoint`
