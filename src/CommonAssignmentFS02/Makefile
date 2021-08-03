# Lista degli eseguibili da generare (vedi nelle recipe per la generazione)
EXECS = httpfs

# Recupero il nome del modulo in analisi
MODULE_NAME = $(notdir $(shell pwd))

# Compilation flags
CFLAGS += -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=29

# ------------ CALCOLO DELLE DIPENDENZE ------------

# Prendo gli include in ogni source file
SRCS=$(shell find *.c 2>/dev/null)
MY_INCLUDES = \
	$(foreach src, $(SRCS), \
		$(filter $(INCLUDE)%,$(shell $(CC) $(CFLAGS) -I $(INCLUDE) -M $(src)))\
	 )

# Trasformo gli include in moduli di dipendenza 
DEPS = $(sort $(filter-out /$(MODULE_NAME)/%, $(dir $(patsubst $(INCLUDE)%,%,$(MY_INCLUDES)))))

# Genero le path assolute dei relativi file oggetto
DEPS_ABSPATH = \
	$(foreach dep,$(DEPS), \
		$(patsubst $(SRC)%.c,$(BUILD)%.o,$(shell find $(SRC)$(dep)*.c)) \
	 )

# ------------ GENERAZIONE PATH ASSOLUTE ------------

# Genero i nomi dei file oggetto
OBJS = $(patsubst %.c,%.o,$(SRCS))

BUILD_DIR = $(BUILD)/$(MODULE_NAME)
BIN_DIR = $(BUILD_DIR)/bin

# Path assolute degli oggetti
OBJS_ABSPATH = $(addprefix $(BUILD_DIR)/,$(OBJS))

# Genero le path assolute degli object file e dell'eseguibile
EXECS_ABSPATH = $(addprefix $(BIN_DIR)/,$(EXECS))

# Genero path relativa per gli include file del modulo
INCLUDE_REL = $(shell realpath --relative-to . $(INCLUDE))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean

all: $(EXECS_ABSPATH)

depend:
	makedepend -I $(INCLUDE_REL) $(SRCS) -Y /usr/include
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# Template recipe per gli eseguibili
#
#$(BIN_DIR)/nome_eseguibile : $(BUILD_DIR)/main_eseguibile.o $(DEPS_ABSPATH)
#	-mkdir -p $(dir $@)
#	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/httpfs : $(OBJS_ABSPATH) $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $(filter %.o,$^) -lfuse -lcurl

# Gestione delle dipendenze
$(DEPS_ABSPATH):
	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D)) $@

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c $(MY_INCLUDES)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXECS_ABSPATH)

# DO NOT DELETE

chmod.o: ../../include/CommonAssignmentFS02/httpfs.h
chmod.o: ../../include/CommonAssignmentFS02/debug.h
chmod.o: ../../include/CommonAssignmentFS02/net.h
chmod.o: ../../include/CommonAssignmentFS02/log.h
chmod.o: ../../include/CommonAssignmentFS02/p.h
chmod.o: ../../include/CommonAssignmentFS02/httpfs.h
chmod.o: ../../include/CommonAssignmentFS02/fuse_functions.def
chmod.o: ../../include/CommonAssignmentFS02/statuses.def
chown.o: ../../include/CommonAssignmentFS02/httpfs.h
chown.o: ../../include/CommonAssignmentFS02/debug.h
chown.o: ../../include/CommonAssignmentFS02/net.h
chown.o: ../../include/CommonAssignmentFS02/log.h
chown.o: ../../include/CommonAssignmentFS02/p.h
chown.o: ../../include/CommonAssignmentFS02/httpfs.h
chown.o: ../../include/CommonAssignmentFS02/fuse_functions.def
chown.o: ../../include/CommonAssignmentFS02/statuses.def
create.o: ../../include/CommonAssignmentFS02/httpfs.h
create.o: ../../include/CommonAssignmentFS02/debug.h
create.o: ../../include/CommonAssignmentFS02/net.h
create.o: ../../include/CommonAssignmentFS02/log.h
create.o: ../../include/CommonAssignmentFS02/p.h
create.o: ../../include/CommonAssignmentFS02/httpfs.h
create.o: ../../include/CommonAssignmentFS02/fuse_functions.def
create.o: ../../include/CommonAssignmentFS02/statuses.def
debug.o: ../../include/CommonAssignmentFS02/net.h
generators.o: ../../include/CommonAssignmentFS02/httpfs.h
generators.o: ../../include/CommonAssignmentFS02/debug.h
generators.o: ../../include/CommonAssignmentFS02/net.h
generators.o: ../../include/CommonAssignmentFS02/log.h
generators.o: ../../include/CommonAssignmentFS02/p.h
generators.o: ../../include/CommonAssignmentFS02/httpfs.h
generators.o: ../../include/CommonAssignmentFS02/fuse_functions.def
generators.o: ../../include/CommonAssignmentFS02/statuses.def
generators.o: ../../include/CommonAssignmentFS02/generators.h
generators.o: ../../include/CommonAssignmentFS02/generators.def
generators.o: ../../include/CommonAssignmentFS02/generators.def
generators.o: ../../include/CommonAssignmentFS02/template.php.h
getattr.o: ../../include/CommonAssignmentFS02/httpfs.h
getattr.o: ../../include/CommonAssignmentFS02/debug.h
getattr.o: ../../include/CommonAssignmentFS02/net.h
getattr.o: ../../include/CommonAssignmentFS02/log.h
getattr.o: ../../include/CommonAssignmentFS02/p.h
getattr.o: ../../include/CommonAssignmentFS02/httpfs.h
getattr.o: ../../include/CommonAssignmentFS02/fuse_functions.def
getattr.o: ../../include/CommonAssignmentFS02/statuses.def
httpfs.o: ../../include/CommonAssignmentFS02/httpfs.h
httpfs.o: ../../include/CommonAssignmentFS02/debug.h
httpfs.o: ../../include/CommonAssignmentFS02/net.h
httpfs.o: ../../include/CommonAssignmentFS02/log.h
httpfs.o: ../../include/CommonAssignmentFS02/p.h
httpfs.o: ../../include/CommonAssignmentFS02/httpfs.h
httpfs.o: ../../include/CommonAssignmentFS02/fuse_functions.def
httpfs.o: ../../include/CommonAssignmentFS02/statuses.def
httpfs.o: ../../include/CommonAssignmentFS02/fuse_api/fuse_api.h
httpfs.o: ../../include/CommonAssignmentFS02/fuse_functions.def
httpfs.o: ../../include/CommonAssignmentFS02/statuses.def
link.o: ../../include/CommonAssignmentFS02/httpfs.h
link.o: ../../include/CommonAssignmentFS02/debug.h
link.o: ../../include/CommonAssignmentFS02/net.h
link.o: ../../include/CommonAssignmentFS02/log.h
link.o: ../../include/CommonAssignmentFS02/p.h
link.o: ../../include/CommonAssignmentFS02/httpfs.h
link.o: ../../include/CommonAssignmentFS02/fuse_functions.def
link.o: ../../include/CommonAssignmentFS02/statuses.def
log.o: ../../include/CommonAssignmentFS02/p.h
log.o: ../../include/CommonAssignmentFS02/httpfs.h
log.o: ../../include/CommonAssignmentFS02/log.h
main.o: ../../include/CommonAssignmentFS02/generators.h
main.o: ../../include/CommonAssignmentFS02/generators.def
main.o: ../../include/CommonAssignmentFS02/httpfs.h
main.o: ../../include/CommonAssignmentFS02/debug.h
main.o: ../../include/CommonAssignmentFS02/net.h
main.o: ../../include/CommonAssignmentFS02/log.h
main.o: ../../include/CommonAssignmentFS02/p.h
main.o: ../../include/CommonAssignmentFS02/httpfs.h
main.o: ../../include/CommonAssignmentFS02/fuse_functions.def
main.o: ../../include/CommonAssignmentFS02/statuses.def
main.o: ../../include/CommonAssignmentFS02/fuse_api/fuse_api.h
main.o: ../../include/CommonAssignmentFS02/version.h
mkdir.o: ../../include/CommonAssignmentFS02/httpfs.h
mkdir.o: ../../include/CommonAssignmentFS02/debug.h
mkdir.o: ../../include/CommonAssignmentFS02/net.h
mkdir.o: ../../include/CommonAssignmentFS02/log.h
mkdir.o: ../../include/CommonAssignmentFS02/p.h
mkdir.o: ../../include/CommonAssignmentFS02/httpfs.h
mkdir.o: ../../include/CommonAssignmentFS02/fuse_functions.def
mkdir.o: ../../include/CommonAssignmentFS02/statuses.def
net.o: ../../include/CommonAssignmentFS02/httpfs.h
net.o: ../../include/CommonAssignmentFS02/debug.h
net.o: ../../include/CommonAssignmentFS02/net.h
net.o: ../../include/CommonAssignmentFS02/log.h
net.o: ../../include/CommonAssignmentFS02/p.h
net.o: ../../include/CommonAssignmentFS02/httpfs.h
net.o: ../../include/CommonAssignmentFS02/fuse_functions.def
net.o: ../../include/CommonAssignmentFS02/statuses.def
open.o: ../../include/CommonAssignmentFS02/httpfs.h
open.o: ../../include/CommonAssignmentFS02/debug.h
open.o: ../../include/CommonAssignmentFS02/net.h
open.o: ../../include/CommonAssignmentFS02/log.h
open.o: ../../include/CommonAssignmentFS02/p.h
open.o: ../../include/CommonAssignmentFS02/httpfs.h
open.o: ../../include/CommonAssignmentFS02/fuse_functions.def
open.o: ../../include/CommonAssignmentFS02/statuses.def
read.o: ../../include/CommonAssignmentFS02/httpfs.h
read.o: ../../include/CommonAssignmentFS02/debug.h
read.o: ../../include/CommonAssignmentFS02/net.h
read.o: ../../include/CommonAssignmentFS02/log.h
read.o: ../../include/CommonAssignmentFS02/p.h
read.o: ../../include/CommonAssignmentFS02/httpfs.h
read.o: ../../include/CommonAssignmentFS02/fuse_functions.def
read.o: ../../include/CommonAssignmentFS02/statuses.def
readdir.o: ../../include/CommonAssignmentFS02/httpfs.h
readdir.o: ../../include/CommonAssignmentFS02/debug.h
readdir.o: ../../include/CommonAssignmentFS02/net.h
readdir.o: ../../include/CommonAssignmentFS02/log.h
readdir.o: ../../include/CommonAssignmentFS02/p.h
readdir.o: ../../include/CommonAssignmentFS02/httpfs.h
readdir.o: ../../include/CommonAssignmentFS02/fuse_functions.def
readdir.o: ../../include/CommonAssignmentFS02/statuses.def
readlink.o: ../../include/CommonAssignmentFS02/httpfs.h
readlink.o: ../../include/CommonAssignmentFS02/debug.h
readlink.o: ../../include/CommonAssignmentFS02/net.h
readlink.o: ../../include/CommonAssignmentFS02/log.h
readlink.o: ../../include/CommonAssignmentFS02/p.h
readlink.o: ../../include/CommonAssignmentFS02/httpfs.h
readlink.o: ../../include/CommonAssignmentFS02/fuse_functions.def
readlink.o: ../../include/CommonAssignmentFS02/statuses.def
rename.o: ../../include/CommonAssignmentFS02/httpfs.h
rename.o: ../../include/CommonAssignmentFS02/debug.h
rename.o: ../../include/CommonAssignmentFS02/net.h
rename.o: ../../include/CommonAssignmentFS02/log.h
rename.o: ../../include/CommonAssignmentFS02/p.h
rename.o: ../../include/CommonAssignmentFS02/httpfs.h
rename.o: ../../include/CommonAssignmentFS02/fuse_functions.def
rename.o: ../../include/CommonAssignmentFS02/statuses.def
rmdir.o: ../../include/CommonAssignmentFS02/httpfs.h
rmdir.o: ../../include/CommonAssignmentFS02/debug.h
rmdir.o: ../../include/CommonAssignmentFS02/net.h
rmdir.o: ../../include/CommonAssignmentFS02/log.h
rmdir.o: ../../include/CommonAssignmentFS02/p.h
rmdir.o: ../../include/CommonAssignmentFS02/httpfs.h
rmdir.o: ../../include/CommonAssignmentFS02/fuse_functions.def
rmdir.o: ../../include/CommonAssignmentFS02/statuses.def
symlink.o: ../../include/CommonAssignmentFS02/httpfs.h
symlink.o: ../../include/CommonAssignmentFS02/debug.h
symlink.o: ../../include/CommonAssignmentFS02/net.h
symlink.o: ../../include/CommonAssignmentFS02/log.h
symlink.o: ../../include/CommonAssignmentFS02/p.h
symlink.o: ../../include/CommonAssignmentFS02/httpfs.h
symlink.o: ../../include/CommonAssignmentFS02/fuse_functions.def
symlink.o: ../../include/CommonAssignmentFS02/statuses.def
truncate.o: ../../include/CommonAssignmentFS02/httpfs.h
truncate.o: ../../include/CommonAssignmentFS02/debug.h
truncate.o: ../../include/CommonAssignmentFS02/net.h
truncate.o: ../../include/CommonAssignmentFS02/log.h
truncate.o: ../../include/CommonAssignmentFS02/p.h
truncate.o: ../../include/CommonAssignmentFS02/httpfs.h
truncate.o: ../../include/CommonAssignmentFS02/fuse_functions.def
truncate.o: ../../include/CommonAssignmentFS02/statuses.def
unlink.o: ../../include/CommonAssignmentFS02/httpfs.h
unlink.o: ../../include/CommonAssignmentFS02/debug.h
unlink.o: ../../include/CommonAssignmentFS02/net.h
unlink.o: ../../include/CommonAssignmentFS02/log.h
unlink.o: ../../include/CommonAssignmentFS02/p.h
unlink.o: ../../include/CommonAssignmentFS02/httpfs.h
unlink.o: ../../include/CommonAssignmentFS02/fuse_functions.def
unlink.o: ../../include/CommonAssignmentFS02/statuses.def
write.o: ../../include/CommonAssignmentFS02/httpfs.h
write.o: ../../include/CommonAssignmentFS02/debug.h
write.o: ../../include/CommonAssignmentFS02/net.h
write.o: ../../include/CommonAssignmentFS02/log.h
write.o: ../../include/CommonAssignmentFS02/p.h
write.o: ../../include/CommonAssignmentFS02/httpfs.h
write.o: ../../include/CommonAssignmentFS02/fuse_functions.def
write.o: ../../include/CommonAssignmentFS02/statuses.def