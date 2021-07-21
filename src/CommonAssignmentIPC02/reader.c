#include <stdio.h>
#include <stdlib.h>
#include "lib/lib.h"

int main(int argc, char **argv)
{
	int a=0,b=0;

	if(argc >= 2)
		a = atoi(argv[1]);
	
	if(argc >= 3)
		b = atoi(argv[2]);

	printf("Hello world! %d\n",add(a,b));
}