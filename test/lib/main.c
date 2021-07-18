#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libgen.h>
#include "lib/lib.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define FAILURE_MESSAGE ANSI_COLOR_RED"Test FAILED"ANSI_COLOR_RESET

int main(int argc, char **argv){
	assert(add(1,2) == 3);
	printf("["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] "ANSI_COLOR_GREEN"Test OK\n"ANSI_COLOR_RESET,basename(argv[0]));
	exit(EXIT_SUCCESS);
}

