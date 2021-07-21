# Flags and compiler
<<<<<<< HEAD
DEBUG = -DDEBUG
export CFLAGS = -Wall $(DEBUG) -DTMP_FILE=\"$(TMP_ABSPATH)\"
=======
DEBUG =
export CFLAGS = -Wall $(DEBUG)
>>>>>>> 1c54dbb67cd1b66d03d2456b26681c80b91641a3
export LFLAGS = -Wall $(DEBUG)
export CC = gcc

# Project folders
export BASEDIR = $(shell pwd)
export SRC=$(BASEDIR)/src
export BIN=$(BASEDIR)/bin
export INCLUDE=$(BASEDIR)/include
export BUILD=$(BASEDIR)/build

SUBDIRS = src include test

# Tells make that those are arbitrary names used as targets
.PHONY : all clean test doc depend

# Complete build recipe
all depend:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# Documentation generation recipe
doc:
	doxygen

test:
	$(MAKE) -C ./test $@

# Clean recipe, must not be the first recipe,
# otherwise will be the default recipe executed 
# by make
clean: 
	rm -f *~ *.bak
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# makedepend dependencies (implicit rules)
# DO NOT DELETE
