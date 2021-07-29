/* 
Pipes can connect only processes with common ancestry
Pipes are not permanent ...

FIFOs solve these problems: they are like pipes, but they 
are permanent and they have a given file name in UNIX

In order to create a fifo, you have to use the command 
mknod: 

$mknod channel p
(channel is the filename; p tells mknod to create a FIFO)

you can identify the new FIFO (channel) by ls (it starts with a p)
the FIFO can be read and write as a regular file. 

A process opening a fifo for reading, will block until another process 
open the FIFO for writing and viceversa
(e.g.: cat < channel) 
Put the command in background if necessary (&)

*/


/* programming a fifo is the same of programming a pipe
but you have to create the fifo with mknod
*/

// This Program send a message on the FIFo; the other one (S24_fifo_sendmsg) sends the message

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

extern int errno;
char *fifo = "fifo";
#define MSGSIZE 63

void fatal (char * s);


int main (int argc, char * argv[]){
  int fd = 0;
  char buffer [MSGSIZE+1];


  /* open the fifo for reading and writing*/
  if ((fd = open (fifo, O_RDWR)) <0 )
    fatal ("fifo open failed");
  /* receive messages*/
  for (;;){
    if (read (fd, buffer, MSGSIZE+1) < 0 )
      fatal ("message read failed");

    printf("message received: %s\n",buffer);
  }
    exit(0);
}



void fatal (char * s){
  perror (s);
  exit(1);

}
